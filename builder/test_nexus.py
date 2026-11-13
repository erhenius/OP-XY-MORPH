"""Validate an attached Nexus source root and run its own host tests; no device."""
import sys,tempfile,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'sdk'))
from tooling.machines import machine_at
from tooling.native import check_sources
from tooling.machine_test import host_tests
m=machine_at(Path(sys.argv[1]).resolve());check_sources([m])
with tempfile.TemporaryDirectory() as d:print('PASS',m.id,host_tests(m,shutil.which('g++') or shutil.which('clang++'),Path(d)),'host tests')
