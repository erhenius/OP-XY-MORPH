from pathlib import Path
import tempfile,subprocess,shutil,sys
root=Path(__file__).resolve().parents[1].parent
for gate in [33,34]:
 r=Path(sys.argv[1 if gate==33 else 2]).resolve() if len(sys.argv)==3 else root/f'Morph_Gate{gate}_Development'
 with tempfile.TemporaryDirectory(dir=root,prefix='measure-') as tmp:
  p=Path(tmp)
  shutil.copy(r/'test_boundary.py',p/'test_boundary.py')
  cases=((Path(sys.argv[1]).resolve() if len(sys.argv)==3 else root/'Morph_Gate33_Development')/'boundary_cases.cpp').read_text()
  mark='    std::printf("Gate33 additional state:'
  i=cases.index(mark)
  workload='''
    selected=11;playing[0]=0;cv_lfo_on=0;telemetry25=0;host_recording=false;
    morphcapture::control_reset(state);fill(0x30000000);
    u32 start=block_queries;sdk_gui_tick();
    std::printf("MEASURE startup %u\\n",block_queries-start);
    fill(0x10000000);capture(0);fill(0x60000000);capture(1);fill(0x30000000);capture(2);
    for(u32 scenario=0;scenario<4;++scenario){
      morphcapture::control_reset(state);cv_lfo_on=0;telemetry25=0;lfo[11][3]=0;
      fill(0x30000000);sdk_gui_tick();fill(0x10000000);capture(0);fill(0x60000000);capture(1);fill(0x30000000);capture(2);
      value(11,3)=morphcapture::automation::encode(0);
      state.active_tracks=scenario==3?0u:scenario==2?1u:255u;
      if(scenario>0){cv_lfo_on=1;telemetry25=0xab030001u;capture(3);drain();}
      u32 total=0,peak=0;
      for(u32 tick=0;tick<240;++tick){
        lfo[11][3]=scenario?((tick&1u)?0x10000000:0x30000000):0;
        start=block_queries;sdk_gui_tick();u32 delta=block_queries-start;
        total+=delta;if(delta>peak)peak=delta;
      }
      std::printf("MEASURE scenario %u queries %u peak %u\\n",scenario,total,peak);
    }
'''
  (p/'boundary_cases.cpp').write_text(cases[:i]+workload+cases[i:])
  result=subprocess.run(['python3',str(p/'test_boundary.py'),str(r/'sdk'),str(r/'machine')],capture_output=True,text=True)
  print('GATE',gate,'exit',result.returncode)
  print('\n'.join(x for x in result.stdout.splitlines() if x.startswith('MEASURE')))
  if result.returncode:print(result.stderr[-1000:])
