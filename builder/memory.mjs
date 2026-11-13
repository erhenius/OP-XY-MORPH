// Allocation analysis only: never patches firmware or transfers files.
import {readFileSync,writeFileSync} from 'node:fs';import {join} from 'node:path';
import {openKit,memoryKit,linkKit} from './sdk/builder/core.mjs';
import {openOriginal} from './kit_compat.mjs';
const job=process.argv[2],read=n=>JSON.parse(readFileSync(join(job,n))),c=read('config.json'),i=read('inspection.json');
const {core:oldCore,kit:original}=await openOriginal(JSON.parse(readFileSync(c.kit)));
const kit=c.calculation_kit?await openKit(JSON.parse(readFileSync(c.calculation_kit))):original;
const core=c.calculation_kit?{memoryKit,linkKit}:oldCore;
const expanded=core.memoryKit(kit,true),baseline=i.selected.length?oldCore.linkKit(oldCore.memoryKit(original,true),i.selected,i.numbers,[]).sizes.allocated:0;
const unknown=c.unknown??[],ids=[...i.selected,...(c.additions??[])];
let report={reserved_bytes:4194304,table_bytes:expanded.profile.table_bytes,baseline_bytes:baseline,unknown,status:unknown.length?'Incomplete — some selected machines require compilation':c.calculation_kit?'Calculated after compilation':'Calculated from matching compiled objects'};
try{
 const linked=ids.length?core.linkKit(expanded,ids,i.numbers,[]):{sizes:{allocated:0}};
 const allocated=linked.sizes.allocated;
 report={...report,allocated_bytes:allocated,change_bytes:allocated-baseline,available_bytes:4194304-report.table_bytes-allocated,percent:(allocated+report.table_bytes)*100/4194304};
}catch(e){if(!e.capacity)throw e;report.status='Does not fit';report.detail=e.message;report.available_bytes=null;report.shortage_bytes=null;}
writeFileSync(join(job,'memory.json'),JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report));
