#!/usr/bin/env python3
import json,queue,tempfile,threading,tkinter as tk,subprocess,sys
from pathlib import Path
from tkinter import ttk,filedialog,messagebox
from manager import Builder
from backend import ROOT,private_root,export_report
from imports import extract,discover
import catalog
from machine_list import merge_rows,available_rows,name_key
import sdk_addons
class App:
 def __init__(self,root):
  self.root=root;root.title('Morph v1.3.0-dev / XY Machine Manager — SDK additions');root.geometry('1180x800');self.events=queue.Queue();self.busy=False;self.scanned=False;self.memory_token=None;self.rows=[];self.installed_ids=set();self.selected=[];self.removals=[];self.installed_rows=[];self.update_morph=tk.BooleanVar(value=False);self.builder=Builder(lambda x:self.events.put(('log',x)));root.protocol('WM_DELETE_WINDOW',self.close)
  p=ttk.Frame(root,padding=14);p.pack(fill='both',expand=True)
  ttk.Label(p,text='Manage machines on your OP-XY',font=('',20,'bold')).pack(anchor='w')
  ttk.Label(p,text='1. Scan setup   2. Add or remove machines   3. Build and review   4. Stage → WRITE OS').pack(anchor='w',pady=8)
  self.kit=tk.StringVar(value='');self.provenance=tk.StringVar()
  for label,var,command in [('Matching kit',self.kit,self.choose_kit),('Previous build job (optional)',self.provenance,self.choose_job)]:
   row=ttk.Frame(p);row.pack(fill='x');ttk.Label(row,text=label,width=28).pack(side='left');ttk.Entry(row,textvariable=var).pack(side='left',fill='x',expand=True);ttk.Button(row,text='Choose…',command=command).pack(side='left')
  controls=ttk.Frame(p);controls.pack(fill='x',pady=8)
  for text,fn in [('Scan XY',self.scan),('Load saved setup',self.offline),('Calculate footprint',self.calculate),('Refresh catalog',self.refresh),('Import ZIP',self.zip),('Import folder',self.folder),('Add selected',self.add),('Clear changes',self.clear),('Build',self.build),('Cancel',self.cancel)]:ttk.Button(controls,text=text,command=fn).pack(side='left')
  row=ttk.Frame(p);row.pack(fill='x')
  ttk.Button(row,text='Import SDK addition…',command=self.sdk_addition).pack(side='left',padx=(0,12))
  ttk.Button(row,text='Install bundled Morph (Morph v1.3.0-dev (Gate40))',command=self.install_morph).pack(side='left')
  ttk.Checkbutton(row,text='Replace installed Morph with bundled Morph v1.3.0-dev (Gate40)',variable=self.update_morph,command=self.selection_changed).pack(side='left',padx=12)
  self.memory_text=tk.StringVar(value='Memory: scan or load a saved setup')
  ttk.Label(p,textvariable=self.memory_text,wraplength=1000).pack(anchor='w',pady=6)
  self.memory_bar=tk.Canvas(p,height=22,highlightthickness=0);self.memory_bar.pack(fill='x')
  ttk.Label(p,text='4 MiB module reservation • blue: baseline • orange: increase • grey: tables • green: free. CPU and sample RAM are separate.').pack(anchor='w')
  self.tabs=ttk.Notebook(p);self.tabs.pack(fill='both',expand=True)
  self.available=self.table('Available machines');self.installed=self.table('Installed setup')
  actions=ttk.Frame(p);actions.pack(fill='x',pady=4)
  ttk.Button(actions,text='Remove selected installed machines',command=self.remove).pack(side='left')
  ttk.Button(actions,text='Keep selected installed machines',command=self.keep).pack(side='left',padx=8)
  ttk.Label(p,text='Removal takes effect only after staging and WRITE OS. Projects and presets are retained.').pack(anchor='w')
  self.summary=tk.StringVar(value='No changes selected');ttk.Label(p,textvariable=self.summary,wraplength=1000).pack(anchor='w',pady=8)
  self.status=tk.StringVar(value='Connect the XY in MTP mode. Scanning and building are read-only.');ttk.Label(p,textvariable=self.status,wraplength=1000).pack(anchor='w')
  row=ttk.Frame(p);row.pack(fill='x',pady=6)
  self.stage_button=ttk.Button(row,text='Stage verified candidate…',command=self.stage,state='disabled');self.stage_button.pack(side='left')
  for text,fn in [('Copy log',self.copy),('Export report',self.report),('Show/hide log',self.toggle)]:ttk.Button(row,text=text,command=fn).pack(side='left',padx=4)
  self.log=tk.Text(p,height=10,wrap='word');self.log.pack(fill='x');self.log_visible=True
  self.show_catalog(catalog.load(private_root()/'xy-machine-manager-cache'));root.after(100,self.drain)
 def table(self,title):
  table=ttk.Treeview(self.tabs,columns=('name','kind','status'),show='headings',selectmode='extended')
  for col in ('name','kind','status'):table.heading(col,text=col.title());table.column(col,width=200 if col!='status' else 540)
  self.tabs.add(table,text=title);return table
 def choose_kit(self):
  if self.busy:return
  p=filedialog.askopenfilename(filetypes=[('Kit JSON','*.json')])
  if p:self.kit.set(p);self.invalidate()
 def choose_job(self):
  if self.busy:return
  p=filedialog.askdirectory(title='Previous build job containing result.json and native-kit')
  if p:self.provenance.set(p);self.invalidate()
 def invalidate(self):
  self.scanned=False;self.stage_button.config(state='disabled');self.installed_ids=set();self.removals=[];self.installed_rows=[]
  self.installed.delete(*self.installed.get_children());self.render_available()
 def work(self,kind,fn):
  if self.busy:return
  self.busy=True;self.stage_button.config(state='disabled')
  def run():
   try:self.events.put((kind,fn()))
   except Exception as e:self.events.put(('error',str(e)))
  threading.Thread(target=run,daemon=True).start()
 def selection_changed(self):
  self.stage_button.config(state='disabled');self.memory_token=None;self.memory_text.set('Selection changed; calculation pending');self.update_summary()
  self.root.after(400,self.lightweight)
 def lightweight(self):
  if self.busy or not self.scanned or (self.builder.job/'sources').exists():return
  selected=list(self.selected);update=self.update_morph.get();removals=list(self.removals);token=self.token()
  self.work('memory',lambda:(token,self.builder.calculate(selected,update_morph=update,removals=removals)))
 def token(self):return json.dumps([self.selected,self.update_morph.get(),sorted(self.removals),str(self.builder.job)],sort_keys=True)
 def offline(self):
  if self.busy:return
  folder=filedialog.askdirectory(title='Saved device export with os.ldr and layout.json')
  if folder:self.work('scan',lambda:self.builder.scan(self.kit.get(),offline=folder,provenance=self.provenance.get() or None))
 def calculate(self):
  if not self.scanned:messagebox.showinfo('Load baseline','Scan XY or load a saved device export first.');return
  token=self.token();selected=list(self.selected);update=self.update_morph.get();removals=list(self.removals)
  compile_missing=messagebox.askyesno('Calculate footprint','Compile missing machines if needed? This can compile the collection, stores private intermediate files, and never transfers to the device.')
  def operation():
   report=self.builder.calculate(selected,update_morph=update,compile_missing=compile_missing,removals=removals)
   return json.dumps([selected,update,sorted(removals),str(self.builder.job)],sort_keys=True),report
  self.work('memory',operation)
 def show_memory(self,value):
  allocated=value.get('allocated_bytes',value.get('baseline_bytes',0));tables=value['table_bytes'];reserved=value['reserved_bytes'];base=value.get('baseline_bytes',allocated);change=allocated-base
  status=value['status'];unknown=value.get('unknown',[])
  text=f"{status}: {allocated:,} bytes ({allocated/1024:.1f} KiB / {allocated/1048576:.3f} MiB); {(allocated+tables)*100/max(reserved,1):.2f}% including {tables:,} table bytes. Change {change:+,} bytes."
  if unknown:text+=' Uncompiled: '+', '.join(unknown)+'. Known usage only.'
  if value.get('available_bytes') is not None:text+=f" Free {value['available_bytes']:,} bytes."
  if status=='Does not fit':text+=' Required total unavailable: linker stops at capacity.'
  self.memory_text.set(text);self.memory_bar.delete('all');width=max(self.memory_bar.winfo_width(),900);x=0
  for amount,color in [(min(base,allocated),'#4477bb'),(max(change,0),'#ee9944'),(tables,'#888888'),(max(reserved-allocated-tables,0),'#bbbbbb' if unknown or status=='Does not fit' else '#66aa77')]:
   end=min(width,x+width*amount/max(reserved,1));self.memory_bar.create_rectangle(x,0,end,22,fill=color,outline='');x=end
 def scan(self):self.work('scan',lambda:self.builder.scan(self.kit.get(),provenance=self.provenance.get() or None))
 def refresh(self):self.work('catalog',lambda:catalog.load(private_root()/'xy-machine-manager-cache',True))
 def show_catalog(self,data):
  self.rows=merge_rows(self.rows,data['catalog']['machines'],refresh_catalog=True);self.render_available()
  self.status.set('Catalog: '+str(data.get('fetched_at') or 'bundled snapshot, date unknown'))
 def render_available(self):
  chosen={self.available.item(i,'tags')[0] for i in self.available.selection() if self.available.item(i,'tags')}
  self.available.delete(*self.available.get_children())
  for index,row in available_rows(self.rows,self.installed_ids):
   status=row.get('status') or 'Catalog-listed; compatibility pending • '+row.get('commit','')[:12]
   self.available.insert('','end',iid=str(index),values=(row.get('name') or row['id'],row.get('kind','?'),status),tags=(row['id'],))
   if row['id'] in chosen:self.available.selection_add(str(index))
 def imported(self,path):
  valid=[]
  for row in discover(path):
   if 'error' in row:self.log.insert('end',row['path']+': '+row['error']+'\n');continue
   valid.append(row)
  self.rows=merge_rows(self.rows,valid);self.render_available()
 def sdk_addition(self):
  if self.busy:return
  p=filedialog.askopenfilename(title='Compatible SDK addition bundle',filetypes=[('SDK addition ZIP','*.zip')])
  if not p:return
  try:
   base=private_root()/'xy-sdk-additions';base.mkdir(exist_ok=True)
   parent=Path(tempfile.mkdtemp(dir=base));unpacked=extract(p,parent/'bundle')
   manifests=list(unpacked.rglob('sdk-addon.json'))
   if len(manifests)!=1:raise ValueError('Choose an SDK addition bundle containing one sdk-addon.json. Ordinary machine ZIPs use Import ZIP.')
   bundle=manifests[0].parent;info=sdk_addons.inspect_bundle(bundle,ROOT)
   if not messagebox.askyesno('Prepare updated installer',info['name']+'\n\nThis is an installer source update. Use a bundle from a trusted author. Prepare and check a separate copy? Your current installer stays available. A fresh scan is required in the new copy.'):return
   self.work('sdk_addition',lambda:sdk_addons.prepare(bundle,ROOT,parent/'installer'))
  except Exception as e:messagebox.showerror('SDK addition refused',str(e))
 def zip(self):
  if self.busy:return
  p=filedialog.askopenfilename(filetypes=[('ZIP','*.zip')])
  if p:
   try:
    base=private_root()/'xy-machine-imports';base.mkdir(exist_ok=True);parent=Path(tempfile.mkdtemp(dir=base));folder=extract(p,parent/'repository')
    if any(folder.rglob('sdk-addon.json')):raise ValueError('This is an SDK addition bundle. Use Import SDK addition… instead.')
    self.imported(folder)
   except Exception as e:messagebox.showerror('Import failed',str(e))
 def folder(self):
  if self.busy:return
  p=filedialog.askdirectory()
  if p:self.imported(p)
 def add(self):
  if self.busy:return
  for index in self.available.selection():
   row=self.rows[int(index)]
   if any(r['id']==row['id'] for r in self.selected):continue
   if self.scanned and row['id'] in self.builder.inspection['selected']:messagebox.showinfo('Already installed',row['id']);continue
   self.selected.append(row)
  self.summary.set('Add: '+', '.join(r['id'] for r in self.selected));self.selection_changed()
 def clear(self):
  if self.busy:return
  self.selected=[];self.removals=[];self.update_morph.set(False);self.render_installed();self.selection_changed()
 def update_summary(self):
  parts=[]
  if self.selected:parts.append('Add: '+', '.join(r['id'] for r in self.selected))
  if self.removals:parts.append('Remove: '+', '.join(sorted(self.removals)))
  if self.update_morph.get():parts.append('Replace: morphcapture')
  self.summary.set(' • '.join(parts) or 'No changes selected')
 def render_installed(self):
  chosen=set(self.installed.selection());self.installed.delete(*self.installed.get_children())
  for m in sorted(self.installed_rows,key=name_key):
   status='REMOVE on next build' if m['id'] in self.removals else 'Verified '+m['id']+' • '+str(m.get('identity'))
   self.installed.insert('','end',iid=m['id'],values=(m.get('name') or m['id'],m.get('kind','machine'),status),tags=(m['id'],))
   if m['id'] in chosen:self.installed.selection_add(m['id'])
 def remove(self):
  if self.busy or not self.scanned:return
  for id in self.installed.selection():
   if id in self.installed_ids and id not in self.removals:self.removals.append(id)
  if 'morphcapture' in self.removals:self.update_morph.set(False)
  self.render_installed();self.selection_changed()
 def keep(self):
  if self.busy:return
  chosen=set(self.installed.selection());self.removals=[id for id in self.removals if id not in chosen]
  self.render_installed();self.selection_changed()
 def install_morph(self):
  if self.busy:return
  if 'morphcapture' in self.installed_ids:messagebox.showinfo('Morph is installed','Use Replace installed Morph to update it.');return
  if any(r['id']=='morphcapture' for r in self.selected):return
  self.selected.append({'id':'morphcapture','path':str(ROOT/'machine')});self.summary.set('Add: '+', '.join(r['id'] for r in self.selected));self.selection_changed()
 def build(self):
  if not self.scanned:messagebox.showinfo('Scan first','Scan the connected setup before building.');return
  if self.memory_token==self.token() and (self.builder.job/'compiled-output-hashes.json').exists():
   self.work('build',self.builder.finalize_calculated);return
  if self.builder.job and (self.builder.job/'sources').exists():messagebox.showinfo('Scan again','Start a fresh scan before rebuilding.');return
  update=self.update_morph.get()
  if update and not messagebox.askyesno('Replace Morph?', 'Rebuild with bundled Morph v1.3.0-dev (Gate40) Morph while preserving its existing IDs? This is an explicit replacement, not automatic version detection. A fresh memory check and SDK backup apply.'):return
  self.work('build',lambda:self.builder.build_selected(list(self.selected),update_morph=update,removals=list(self.removals)))
 def stage(self):
  if self.busy:return
  if messagebox.askyesno('Stage candidate?',self.summary.get()+'\n\nCopy these verified files using SDK backups and readback? Then run WRITE OS and reboot on the XY.') :self.work('stage',self.builder.stage)
 def cancel(self):
  try:self.builder.cancel();self.status.set('Cancellation requested; waiting for the current operation to stop.')
  except Exception as e:messagebox.showinfo('Transfer in progress',str(e))
 def close(self):
  if self.busy:messagebox.showinfo('Operation in progress','Cancel the build or wait for transfer completion before closing.');return
  self.root.destroy()
 def copy(self):self.root.clipboard_clear();self.root.clipboard_append(self.log.get('1.0','end-1c'))
 def report(self):
  if self.busy:return
  p=filedialog.asksaveasfilename(defaultextension='.zip',initialfile='XY_Manager_Report.zip')
  if p:
   try:export_report(self.builder.job,p)
   except Exception as e:messagebox.showerror('Export failed',str(e))
 def toggle(self):
  if self.log_visible:self.log.pack_forget()
  else:self.log.pack(fill='x')
  self.log_visible=not self.log_visible
 def drain(self):
  for _ in range(100):
   try:kind,value=self.events.get_nowait()
   except queue.Empty:break
   if kind=='log':self.log.insert('end',str(value)+'\n');self.log.see('end');self.status.set(str(value)[-220:]);continue
   self.busy=False
   if kind=='scan':
    self.scanned=True;self.root.after(400,self.lightweight);self.installed.delete(*self.installed.get_children())
    self.installed_ids=set(value['selected']);self.selected=[r for r in self.selected if r['id'] not in self.installed_ids]
    self.removals=[];self.installed_rows=value['machines'];self.render_installed();self.update_summary()
    self.render_available()
    self.status.set('Exactly reconstructed '+str(len(value['selected']))+' installed machines. Accessible OS may differ from running firmware.')
   elif kind=='memory':
    token,report=value
    if token==self.token():self.memory_token=token;self.show_memory(report)
    else:self.memory_text.set('Selection changed; calculate again.')
   elif kind=='catalog':self.show_catalog(value)
   elif kind=='build':
    self.summary.set('Add: '+', '.join(value['additions'])+' • Remove: '+', '.join(value.get('removals',[]))+' • Replace: '+', '.join(value.get('replacements',[]))+f" • {value['allocated_bytes']:,} bytes used; {value['available_bytes']:,} free. Files: "+', '.join(['os.ldr',*value['content_sha256']]))
    self.show_memory({**value,'baseline_bytes':json.loads((self.builder.job/'memory.json').read_text()).get('baseline_bytes',value['allocated_bytes']),'status':'Verified final firmware allocation'})
    self.stage_button.config(state='normal');self.status.set('Verified candidate ready. Previous/new provenance retained at '+str(self.builder.job))
   elif kind=='stage':self.status.set('Staged and verified. Run WRITE OS on the XY, then reboot.')
   elif kind=='sdk_addition':
    self.invalidate();self.memory_token=None;self.memory_text.set('SDK addition prepared; open the new installer and scan again.')
    self.status.set(value['status']+' • '+value['path'])
    if messagebox.askyesno('SDK addition prepared',value['name']+'\n\nMorph host checks passed. Open the updated installer? Native build and device checks are still required.'):
     try:
      subprocess.Popen([sys.executable,str(Path(value['path'])/'app.py')],cwd=value['path']);self.root.destroy();return
     except Exception as e:messagebox.showerror('Could not open installer',str(e)+'\nOpen '+value['path']+'/Open_XY_Machine_Manager.command')
   else:
    self.scanned=False
    error=str(value)
    if 'Expected one OP-XY' in error:error='No accessible OP-XY: choose mtp file transfer (mac), reconnect USB, and close OpenMTP, Field Kit and other transfer apps. Retry Scan XY. '+error
    self.log.insert('end','ERROR: '+error+'\n');self.status.set('Stopped: '+error)
  self.root.after(100,self.drain)
if __name__=='__main__':
 root=tk.Tk();App(root);root.mainloop()
