"""Exact-base SDK addition bundles. Prepare a separate installer; never patch a device.

SDK bundles are trusted installer source updates, not ordinary machine imports.
No contributed installer script is run. Existing profile edits and Morph API
names must survive, and the original installer is never modified.
"""
import hashlib
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path, PurePosixPath

PROFILE = 'sdk/profiles/opxy-1.2.1.json'
IGNORED = {'.git', '.venv', '__pycache__', 'node_modules', 'local'}

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def require(ok, message):
    if not ok:
        raise ValueError(message)

def read_json(path):
    return json.loads(Path(path).read_text())

def safe_relative(value):
    require(isinstance(value, str) and '\\' not in value, 'Invalid SDK path')
    p = PurePosixPath(value)
    require(not p.is_absolute() and len(p.parts) > 1 and p.parts[0] == 'sdk' and
            all(x not in ('..', '.', '') and ':' not in x for x in p.parts) and
            not any(x in IGNORED for x in p.parts), 'SDK additions may only update SDK source files')
    require(p.suffix.lower() not in ('.ldr', '.tfw', '.xy', '.oxy', '.o', '.a', '.elf', '.wasm', '.exe', '.dll'),
            'Compiled/private files are not SDK source additions')
    return p.as_posix()

def validate_profile(old, new):
    # Additions may add reviewed hooks/services, never change the existing
    # stock authentication, reservation, table ranges or patch policies.
    for key, value in old.items():
        if key not in ('patches', 'services'):
            require(new.get(key) == value, 'SDK addition changes protected profile field: ' + key)
    for key, value in old.get('services', {}).items():
        require(new.get('services', {}).get(key) == value, 'SDK addition changes an existing service')
    patches = new.get('patches', [])
    require(isinstance(patches, list), 'Invalid SDK patch list')
    for patch in old['patches']:
        require(patch in patches, 'SDK addition removes or changes an existing firmware edit')
    intervals = []
    for patch in patches:
        require(isinstance(patch.get('length'), int) and patch['length'] > 0, 'Invalid hook length')
        at = int(patch['address'], 16)
        require(len(bytes.fromhex(patch['before'])) == patch['length'], 'Invalid hook preimage')
        require(not any(at < end and start < at + patch['length'] for start, end in intervals), 'Overlapping SDK firmware edits')
        intervals.append((at, at + patch['length']))

def inspect_bundle(bundle, installer):
    bundle, installer = Path(bundle).resolve(), Path(installer).resolve()
    require(not bundle.is_symlink(), 'SDK bundle symlinks are not supported')
    manifest = read_json(bundle / 'sdk-addon.json')
    require(manifest.get('schema') == 1 and manifest.get('type') == 'xy-sdk-addition', 'Unsupported SDK addition format')
    require(isinstance(manifest.get('name'), str) and 0 < len(manifest['name']) <= 100, 'SDK addition needs a name')
    require(manifest.get('base_payload_sha256') == sha(installer / 'PAYLOAD_SHA256.json'),
            'SDK addition targets a different installer edition')
    require(manifest.get('base_profile_sha256') == sha(installer / PROFILE), 'SDK addition targets a different SDK profile')
    files = manifest.get('files')
    require(isinstance(files, dict) and 0 < len(files) <= 5000, 'Invalid SDK addition file list')
    seen = set()
    for rel, expected in files.items():
        rel = safe_relative(rel)
        require(rel.casefold() not in seen, 'Duplicate SDK path')
        seen.add(rel.casefold())
        source, target = bundle / 'files' / rel, installer / rel
        require(source.is_file() and not source.is_symlink(), 'Missing SDK source: ' + rel)
        require(not any(p.is_symlink() for p in source.parents if p.is_relative_to(bundle)), 'SDK source symlink')
        require(isinstance(expected, dict) and expected.get('after') == sha(source), 'SDK source hash mismatch: ' + rel)
        require(not target.is_symlink() and (expected.get('before') == sha(target) if target.exists() else expected.get('before') is None),
                'SDK base file differs: ' + rel)
        if target.is_file() and rel.startswith('sdk/sdk/include/opxy/morph_'):
            require(source.read_bytes() == target.read_bytes(), 'SDK addition changes a protected Morph header: ' + rel)
        # Retain all existing Morph bridge/HUD entry points in shared units.
        if target.is_file() and target.suffix in ('.cpp', '.hpp', '.h', '.in', '.s'):
            old_names = set(re.findall(r'\bsdk_morph_[a-zA-Z0-9_]+\b', target.read_text(errors='replace')))
            new_names = set(re.findall(r'\bsdk_morph_[a-zA-Z0-9_]+\b', source.read_text(errors='replace')))
            require(old_names <= new_names, 'SDK addition removes Morph interfaces: ' + rel)
    actual = {p.relative_to(bundle / 'files').as_posix() for p in (bundle / 'files').rglob('*') if p.is_file()}
    require(actual == set(files), 'SDK bundle contains undeclared source files')
    profile_path = bundle / 'files' / PROFILE if PROFILE in files else installer / PROFILE
    validate_profile(read_json(installer / PROFILE), read_json(profile_path))
    core_rel = 'sdk/builder/core.mjs'
    core = bundle / 'files' / core_rel if core_rel in files else installer / core_rel
    match = re.search(r"^export const PROFILE_SHA256 = '([0-9a-f]{64})';$", core.read_text(), re.M)
    require(match and match[1] == sha(profile_path), 'SDK profile and builder authentication disagree')
    # Compiler authentication is not an SDK feature update.
    if 'sdk/profiles/toolchains.json' in files:
        require((bundle / 'files/sdk/profiles/toolchains.json').read_bytes() ==
                (installer / 'sdk/profiles/toolchains.json').read_bytes(), 'SDK addition changes compiler authentication')
    return manifest

def payload(root):
    return {p.relative_to(root).as_posix(): sha(p) for p in sorted(root.rglob('*'))
            if p.is_file() and p.name != 'PAYLOAD_SHA256.json' and
            not any(x in IGNORED for x in p.relative_to(root).parts)}

def prepare(bundle, installer, destination, run_checks=True):
    installer, destination = Path(installer).resolve(), Path(destination).resolve()
    require(not destination.exists() and not destination.is_relative_to(installer), 'Choose a fresh installer destination')
    # Check the current edition before accepting exact-base metadata.
    originals = read_json(installer / 'PAYLOAD_SHA256.json')
    for rel, expected in originals.items():
        p = (installer / rel).resolve()
        require(p.is_relative_to(installer) and p.is_file() and sha(p) == expected, 'Installer integrity mismatch: ' + rel)
    manifest = inspect_bundle(bundle, installer)
    try:
        destination.mkdir(parents=True)
        for rel in originals:
            target = destination / rel
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(installer / rel, target)
            require(sha(target) == originals[rel], 'Installer changed during preparation: ' + rel)
        shutil.copy2(installer / 'PAYLOAD_SHA256.json', destination / 'PAYLOAD_SHA256.json')
        old_hash = sha(installer / PROFILE)
        history = destination / 'sdk_history' / old_hash
        if not history.exists():
            for rel in originals:
                if rel.startswith('sdk/'):
                    target = history / rel[4:]
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(destination / rel, target)
        index = destination / 'sdk_history/index.json'
        known = read_json(index) if index.exists() else []
        index.write_text(json.dumps(sorted(set(known + [old_hash])), indent=2) + '\n')
        for rel in manifest['files']:
            target = destination / rel
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(Path(bundle) / 'files' / rel, target)
            require(sha(target) == manifest['files'][rel]['after'], 'SDK bundle changed during preparation: ' + rel)
        previous_toolchain = installer / 'sdk/local/toolchain'
        if not (previous_toolchain / 'bin').is_dir() and (installer / 'SDK_ADDITION.json').is_file():
            previous_toolchain = Path(read_json(installer / 'SDK_ADDITION.json').get('previous_toolchain', str(previous_toolchain)))
        receipt = {'schema': 1, 'name': manifest['name'], 'base_payload_sha256': manifest['base_payload_sha256'],
                   'base_profile_sha256': old_hash, 'profile_sha256': sha(destination / PROFILE),
                   'files': manifest['files'], 'previous_installer': str(installer),
                   'previous_toolchain': str(previous_toolchain),
                   'status': 'Host boundary checks pending; native and hardware acceptance required'}
        (destination / 'SDK_ADDITION.json').write_text(json.dumps(receipt, indent=2) + '\n')
        if run_checks:
            # The installer-owned harness is unchanged by the bundle. It
            # compiles and exercises Morph with the proposed SDK; no flash.
            result = subprocess.run([sys.executable, str(destination / 'test_boundary.py')],
                                    cwd=destination, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180)
            (destination / 'SDK_ADDITION_HOST_CHECK.log').write_text(result.stdout)
            require(result.returncode == 0, 'SDK addition failed Morph host checks; original installer is unchanged')
            receipt['status'] = 'Morph host boundary checks passed; native and hardware acceptance required'
            (destination / 'SDK_ADDITION.json').write_text(json.dumps(receipt, indent=2) + '\n')
        (destination / 'PAYLOAD_SHA256.json').write_text(json.dumps(payload(destination), indent=2) + '\n')
        return {'name': manifest['name'], 'path': str(destination), 'status': receipt['status']}
    except Exception:
        if destination.exists():
            shutil.rmtree(destination)
        raise
