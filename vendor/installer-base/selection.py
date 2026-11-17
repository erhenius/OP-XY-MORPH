"""Validate an explicit collection edit before touching sources or build outputs."""
def plan(installed, additions, removals=(), update_morph=False):
 installed=list(installed);additions=list(additions);removals=list(removals)
 if len(removals)!=len(set(removals)) or any(x not in installed for x in removals):
  raise ValueError('Remove only installed machines, once each')
 if len(additions)!=len(set(additions)) or any(x in installed for x in additions):
  raise ValueError('Duplicate or already installed machine')
 if update_morph and ('morphcapture' not in installed or 'morphcapture' in removals):
  raise ValueError('Morph update requires an installed Morph that is not marked for removal')
 return [x for x in installed if x not in removals]+additions
