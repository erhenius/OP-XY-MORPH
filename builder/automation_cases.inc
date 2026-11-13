    // HOST recorder/erase model. SDK tick, controller, queue and codec are real.
    for(u32 i=0;i<8;++i)engines[i]=0;
    selected=11u;playing[0]=playing[1]=playing[3]=0;
    fill(0x10000000);morphcapture::control_reset(state);sdk_gui_tick();
    fill(0x60000000);capture(1);
    fill(0x30000000);capture(2);
    host_recording=true;playing[0]=1;playing[3]=1;host_step=3;
    aux_events.turn(3,4);sdk_gui_tick();drain();
    check(host_recorded==1u,"Movement recorded more than one lane event");
    check(host_lane_mask[3][11][3],"AUX movement lane missing");
    for(u32 step=0;step<16u;++step)for(u32 t=0;t<16u;++t)for(u32 i=0;i<control::track_parameters;++i)
      check(!host_lane_mask[step][t][i] || (t==11u && i==3u),"Destination automation leaked");
    check(morphcapture::recorded_reference,"Recording origin not retained");
    // Overdub the same stock lane; no capture or LIVE lane gets recorded.
    aux_events.turn(3,-4);sdk_gui_tick();drain();
    check(host_recorded==2u && host_lane[3][11][3]==morphcapture::automation::encode(0),"Overdub failed");
    host_recording=false;playing[3]=0;
    value(11,3)=morphcapture::automation::encode(64);sdk_gui_tick();drain();
    check(state.travel==64 && bases[0][0]==0x60000000u,"Playback B endpoint wrong");
    value(11,3)=morphcapture::automation::encode(-64);ticks96=0;sdk_gui_tick();drain();
    check(state.travel==-64 && bases[0][0]==0x10000000u,"Loop A endpoint wrong");
    // A real active destination lock wins live, while its stored base follows.
    sdk_param_step_set(kProject,0,0,0x22220000);value(11,3)=morphcapture::automation::encode(64);
    sdk_gui_tick();drain();
    check(!state.fault && bases[0][0]==0x60000000u && value(0,0)==0x22220000,"Lock blocked controller or lost priority");
    unlock(0,0);value(0,0)=static_cast<i32>(bases[0][0]);
    check(value(0,0)==0x60000000,"Stored destination failed to resume");
    // Persist five banks, schema identity and pattern ownership.
    opxy::u8 saved[5684]={};
    check(morphcapture::control_save(state,saved,sizeof saved)==sizeof saved,"Save payload length wrong");
    morphcapture::control_load(state,saved,sizeof saved);sdk_gui_tick();
    check(state.scene[0][0][0]==0x10000000 && state.scene[1][0][0]==0x60000000,"Restored snapshots overwritten by startup");
    check(!state.mode && morphcapture::recorded_reference,"Load must require manual LIVE and retain recording basis");
    capture(3);drain();check(state.mode && state.travel==64,"Loaded movement did not reproduce");
    capture(3);drain();
    for(u32 i=0;i<5u;++i){value(11,3)=morphcapture::automation::encode(i*10-20);sdk_gui_tick();}
    check(!state.mode && state.phase==morphcapture::idle && bases[0][0]==0x30000000u,"Playback undid manual bypass");
    saved[900]^=1u;morphcapture::control_load(state,saved,sizeof saved);
    check(!state.base_ready && !morphcapture::restored_pending,"Corrupt persistence published");saved[900]^=1u;
    morphcapture::control_load(state,saved,100u);check(!state.base_ready,"Truncated persistence accepted");
    morphcapture::control_load(state,saved,sizeof saved);engines[0]=99;sdk_gui_tick();
    check(state.scene[0][0][0]==static_cast<i32>(bases[0][0]) && !morphcapture::recorded_reference,"Incompatible engine restored old snapshot");
    engines[0]=0;
    // Model BAR+M2's documented removal of this track's parameter locks only.
    host_lane_mask[3][0][5]=true;
    for(u32 step=0;step<16;++step)for(u32 i=0;i<control::track_parameters;++i)host_lane_mask[step][11][i]=false;
    check(!host_lane_mask[3][11][3] && host_lane_mask[3][0][5],"Host erase model removed unrelated track");
    playing[0]=0;
