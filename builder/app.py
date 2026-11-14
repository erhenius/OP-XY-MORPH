#!/usr/bin/env python3
import json,queue,tempfile,threading,tkinter as tk
from pathlib import Path
from tkinter import ttk,filedialog,messagebox
from manager import Builder
from backend import ROOT,private_root,export_report
from imports import extract,discover
import catalog
class App:
 def __init__(self,root):
  self.root=root;root.title('Morph v1.2.0 / XY Machine Manager');root.geometry('1050x760');self.events=queue.Queue();self.busy=False;self.scanned=False;self.memory_token=None;self.rows=[];self.selected=[];self.update_morph=tk.BooleanVar(value=False);self.builder=Builder(lambda x:self.events.put(('log',x)));root.protocol('WM_DELETE_WINDOW',self.close)
  p=ttk.Frame(root,padding=14);p.pack(fill='both',expand=True)
  ttk.Label(p,text='Add machines to your OP-XY',font=('',20,'bold')).pack(anchor='w')
  ttk.Label(p,text='1. Scan setup   2. Select additions   3. Build and review   4. Stage → WRITE OS').pack(anchor='w',pady=8)
  self.kit=tk.StringVar(value='');self.provenance=tk.StringVar()
  for label,var,command in [('Matching kit',self.kit,self.choose_kit),('Previous build job (optional)',self.provenance,self.choose_job)]:
   row=ttk.Frame(p);row.pack(fill='x');ttk.Label(row,text=label,width=28).pack(side='left');ttk.Entry(row,textvariable=var).pack(side='left',fill='x',expand=True);ttk.Button(row,text='Choose…',command=command).pack(side='left')
  controls=ttk.Frame(p);controls.pack(fill='x',pady=8)
  for text,fn in [('Scan XY',self.scan),('Load saved setup',self.offline),('Calculate footprint',self.calculate),('Refresh catalog',self.refresh),('Import ZIP',self.zip),('Import folder',self.folder),('Add selected',self.add),('Clear additions',self.clear),('Build',self.build),('Cancel',self.cancel)]:ttk.Button(controls,text=text,command=fn).pack(side='left')
  row=ttk.Frame(p);row.pack(fill='x')
  ttk.Button(row,text='Install bundled Morph (Morph v1.2.0 (Gate39))',command=self.install_morph).pack(side='left')
  ttk.Checkbutton(row,text='Replace installed Morph with bundled Morph v1.2.0 (Gate39)',variable=self.update_morph,command=self.selection_changed).pack(side='left',padx=12)
  self.memory_text=tk.StringVar(value='Memory: scan or load a saved setup')
  ttk.Label(p,textvariable=self.memory_text,wraplength=1000).pack(anchor='w',pady=6)
  self.memory_bar=tk.Canvas(p,height=22,highlightthickness=0);self.memory_bar.pack(fill='x')
  ttk.Label(p,text='4 MiB module reservation • blue: baseline • orange: increase • grey: tables • green: free. CPU and sample RAM are separate.').pack(anchor='w')
  self.tabs=ttk.Notebook(p);self.tabs.pack(fill='both',expand=True)
  self.available=self.table('Available machines');self.installed=self.table('Installed setup')
  self.summary=tk.StringVar(value='No additions selected');ttk.Label(p,textvariable=self.summary,wraplength=1000).pack(anchor='w',pady=8)
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
 def invalidate(self):self.scanned=False;self.stage_button.config(state='disabled')
 def work(self,kind,fn):
  if self.busy:return
  self.busy=True;self.stage_button.config(state='disabled')
  def run():
   try:self.events.put((kind,fn()))
   except Exception as e:self.events.put(('error',str(e)))
  threading.Thread(target=run,daemon=True).start()
 def selection_changed(self):
  self.stage_button.config(state='disabled');self.memory_token=None;self.memory_text.set('Selection changed; calculation pending')
  self.root.after(400,self.lightweight)
 def lightweight(self):
  if self.busy or not self.scanned or (self.builder.job/'sources').exists():return
  selected=list(self.selected);update=self.update_morph.get();token=self.token()
  self.work('memory',lambda:(token,self.builder.calculate(selected,update_morph=update)))
 def token(self):return json.dumps([self.selected,self.update_morph.get(),str(self.builder.job)],sort_keys=True)
 def offline(self):
  if self.busy:return
  folder=filedialog.askdirectory(title='Saved device export with os.ldr and layout.json')
  if folder:self.work('scan',lambda:self.builder.scan(self.kit.get(),offline=folder,provenance=self.provenance.get() or None))
 def calculate(self):
  if not self.scanned:messagebox.showinfo('Load baseline','Scan XY or load a saved device export first.');return
  token=self.token();selected=list(self.selected);update=self.update_morph.get()
  compile_missing=messagebox.askyesno('Calculate footprint','Compile missing machines if needed? This can compile the collection, stores private intermediate files, and never transfers to the device.')
  def operation():
   report=self.builder.calculate(selected,update_morph=update,compile_missing=compile_missing)
   return json.dumps([selected,update,str(self.builder.job)],sort_keys=True),report
  self.work('memory',operation)
 def show_memory(self,value):
  allocated=value.get('allocated_bytes',value.get('baseline_bytes',0));tables=value['table_bytes'];reserved=value['reserved_bytes'];base=value.get('baseline_bytes',allocated);change=allocated-base
  status=value['status'];unknown=value.get('unknown',[])
  text=f"{status}: {allocated:,} bytes ({allocated/1024:.1f} KiB / {allocated/1048576:.3f} MiB); {(allocated+tables)*100/reserved:.2f}% including {tables:,} table bytes. Change {change:+,} bytes."
  if unknown:text+=' Uncompiled: '+', '.join(unknown)+'. Known usage only.'
  if value.get('available_bytes') is not None:text+=f" Free {value['available_bytes']:,} bytes."
  if status=='Does not fit':text+=' Required total unavailable: linker stops at capacity.'
  self.memory_text.set(text);self.memory_bar.delete('all');width=max(self.memory_bar.winfo_width(),900);x=0
  for amount,color in [(min(base,allocated),'#4477bb'),(max(change,0),'#ee9944'),(tables,'#888888'),(max(reserved-allocated-tables,0),'#bbbbbb' if unknown or status=='Does not fit' else '#66aa77')]:
   end=min(width,x+width*amount/reserved);self.memory_bar.create_rectangle(x,0,end,22,fill=color,outline='');x=end
 def scan(self):self.work('scan',lambda:self.builder.scan(self.kit.get(),provenance=self.provenance.get() or None))
 def refresh(self):self.work('catalog',lambda:catalog.load(private_root()/'xy-machine-manager-cache',True))
 def show_catalog(self,data):
  for row in data['catalog']['machines']:
   index=len(self.rows);self.rows.append(row);self.available.insert('','end',iid=str(index),values=(row['id'],row.get('kind','?'),'Catalog-listed; compatibility pending • '+row.get('commit','')[:12]))
  self.status.set('Catalog: '+str(data.get('fetched_at') or 'bundled snapshot, date unknown'))
 def imported(self,path):
  for row in discover(path):
   if 'error' in row:self.log.insert('end',row['path']+': '+row['error']+'\n');continue
   index=len(self.rows);self.rows.append(row);self.available.insert('','end',iid=str(index),values=(row['name'],row['kind'],row['status']))
 def zip(self):
  if self.busy:return
  p=filedialog.askopenfilename(filetypes=[('ZIP','*.zip')])
  if p:
   try:
    base=private_root()/'xy-machine-imports';base.mkdir(exist_ok=True);parent=Path(tempfile.mkdtemp(dir=base));self.imported(extract(p,parent/'repository'))
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
  self.selected=[];self.summary.set('No additions selected');self.selection_changed()
 def install_morph(self):
  if self.busy:return
  if any(r['id']=='morphcapture' for r in self.selected):return
  self.selected.append({'id':'morphcapture','path':str(ROOT/'machine')});self.summary.set('Add: '+', '.join(r['id'] for r in self.selected));self.selection_changed()
 def build(self):
  if not self.scanned:messagebox.showinfo('Scan first','Scan the connected setup before building.');return
  if self.memory_token==self.token() and (self.builder.job/'compiled-output-hashes.json').exists():
   self.work('build',self.builder.finalize_calculated);return
  if self.builder.job and (self.builder.job/'sources').exists():messagebox.showinfo('Scan again','Start a fresh scan before rebuilding.');return
  update=self.update_morph.get()
  if update and not messagebox.askyesno('Replace Morph?', 'Rebuild with bundled Morph v1.2.0 (Gate39) Morph while preserving its existing IDs? This is an explicit replacement, not automatic version detection. A fresh memory check and SDK backup apply.'):return
  self.work('build',lambda:self.builder.build_selected(list(self.selected),update_morph=update))
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
    for m in value['machines']:self.installed.insert('','end',values=(m.get('name') or m['id'],m.get('kind','machine'),'Verified '+m['id']+' • '+str(m.get('identity'))+' • source rebuild pending'))
    self.status.set('Exactly reconstructed '+str(len(value['selected']))+' installed machines. Accessible OS may differ from running firmware.')
   elif kind=='memory':
    token,report=value
    if token==self.token():self.memory_token=token;self.show_memory(report)
    else:self.memory_text.set('Selection changed; calculate again.')
   elif kind=='catalog':self.show_catalog(value)
   elif kind=='build':
    self.summary.set('Add: '+', '.join(value['additions'])+' • Replace: '+', '.join(value.get('replacements',[]))+f" • {value['allocated_bytes']:,} bytes used; {value['available_bytes']:,} free. Files: "+', '.join(['os.ldr',*value['content_sha256']]))
    self.show_memory({**value,'baseline_bytes':json.loads((self.builder.job/'memory.json').read_text()).get('baseline_bytes',value['allocated_bytes']),'status':'Verified final firmware allocation'})
    self.stage_button.config(state='normal');self.status.set('Verified candidate ready. Previous/new provenance retained at '+str(self.builder.job))
   elif kind=='stage':self.status.set('Staged and verified. Run WRITE OS on the XY, then reboot.')
   else:
    self.scanned=False
    error=str(value)
    if 'Expected one OP-XY' in error:error='No accessible OP-XY: choose mtp file transfer (mac), reconnect USB, and close OpenMTP, Field Kit and other transfer apps. Retry Scan XY. '+error
    self.log.insert('end','ERROR: '+error+'\n');self.status.set('Stopped: '+error)
  self.root.after(100,self.drain)
if __name__=='__main__':
 root=tk.Tk();App(root);root.mainloop()
