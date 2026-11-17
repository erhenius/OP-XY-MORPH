"""Stable available-machine rows, shared by catalog and local imports."""
def merge_rows(existing, incoming, refresh_catalog=False):
    local = [r for r in existing if 'path' in r]
    rows = {} if refresh_catalog else {r['id']: r for r in existing}
    rows.update((r['id'], r) for r in incoming)
    if refresh_catalog:
        rows.update((r['id'], r) for r in local)
    return list(rows.values())

def name_key(row):
    return ((row.get('name') or row['id']).casefold(), row['id'].casefold())

def available_rows(rows, installed):
    installed = set(installed)
    return sorted(((i, r) for i, r in enumerate(rows) if r['id'] not in installed),
                  key=lambda pair: name_key(pair[1]))
