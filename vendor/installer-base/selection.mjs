import assert from 'node:assert/strict';
export function selectionPlan(installed, additions=[], removals=[], replacements=[]) {
 assert(Array.isArray(removals) && new Set(removals).size===removals.length && removals.every(id=>installed.includes(id)),'Remove only installed machines, once each');
 assert(Array.isArray(additions) && new Set(additions).size===additions.length && additions.every(id=>!installed.includes(id)),'Duplicate or already installed machine');
 assert(replacements.every(id=>id==='morphcapture' && installed.includes(id) && !removals.includes(id)) && new Set(replacements).size===replacements.length,'Conflicting or invalid Morph replacement');
 return [...installed.filter(id=>!removals.includes(id)),...additions];
}
