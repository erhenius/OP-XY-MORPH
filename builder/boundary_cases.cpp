#include <cstring>
static unsigned checks=0;
void check(bool ok,const char* why){require(ok,why);++checks;}
void frame(u32 page=88) {
    opxy::platform::ScreenCursor cursor={0,4};
    sdk_screen_next(&cursor,page);
}
void drain() {
    for(u32 i=0;i<80 && state.phase!=morphcapture::idle && state.phase!=morphcapture::failed;++i)sdk_gui_tick();
    check(state.phase==morphcapture::idle && !state.fault,"Scene transaction did not finish cleanly");
}
void fill(i32 v) {
    for(u32 t=0;t<8;++t)for(u32 p=0;p<control::track_parameters;++p)bases[t][p]=static_cast<u32>(v);
}
void capture(u32 encoder) {aux_events.click(encoder);sdk_gui_tick();}
void verify_all(i32 v) {
    for(u32 t=0;t<8;++t)for(u32 p=0;p<26u;++p)
        check(static_cast<i32>(bases[t][morphcapture::slots[p]])==v,"Stored parameter mismatch");
}
int main() {
    sdk_aux.engine=aux_engine;sdk_aux.tick=aux_tick;aux_engines[11]=42;
    project_loading=1;sdk_gui_tick();
    check(!state.base_ready && !state.captured_mask[0] && !state.captured_mask[1],"Startup captured while loading");project_loading=0;
    fill(0x12000000);bases[0][0]=0xffffffffu;sdk_gui_tick();
    check(!state.base_ready && !state.captured_mask[0] && !state.captured_mask[1],"Failed startup partially captured scenes");
    fill(0x12000000);const u32 startup_sets=sets;sdk_gui_tick();
    check(state.base_ready && state.captured_mask[0]==255u && state.captured_mask[1]==255u,"Startup required manual AUX capture");
    for(u32 t=0;t<8u;++t)for(u32 p=0;p<26u;++p)
      check(state.scene[0][t][p]==0x12000000 && state.scene[1][t][p]==0x12000000 && state.base[t][p]==0x12000000,"Startup A/B/S differ");
    check(sets==startup_sets && !state.mode && !state.origin_ready,"Startup wrote parameters or entered LIVE");
    fill(0x22000000);selected=11u;sdk_gui_tick();selected=0;sdk_gui_tick();
    check(state.scene[0][0][0]==0x12000000 && state.base[0][0]==0x12000000,"AUX visit recaptured startup");
    fill(0x10000000);capture(0); // A; automatic S starts here
    fill(0x60000000);capture(1); // B
    fill(0x30000000);capture(2); // independent S
    check(state.captured_mask[0]==255 && state.captured_mask[1]==255 && state.base_ready,"A/B/S setup failed");
    check(!morph_callback && !morph_overlay,"SDK did not end callback scope");
    check(morph_check(morph_session),"Persistent session unexpectedly invalid");
    check(!sdk_morph_session_context_ok(),"Write-side session check escaped callback scope");
    i32 sample=-7;
    check(!sdk_morph_stored_current_read(0,0,&sample),"Callback-only reader leaked");
    check(!sdk_morph_stored_once_submit(0,0,123),"Out-of-callback write accepted");
    check(sdk_morph_hud_substitute!=nullptr,"HUD callback not published");
    held_keys[14]=1;selected=0;
    check(sdk_morph_hud_substitute(88)==219,"HUD denied AFTER callback despite valid captured session");
    frame();check(order.to==219 && sdk_morph_hud_frame_authorized(0),"Actual compositor did not authorize HUD");
    morphcapture::hud::Snapshot snap={};
    const u32 queries_before=block_queries;
    check(sdk_morph_hud_snapshot_read(0,&snap),"HUD snapshot denied after callback");
    const u32 snapshot_queries=block_queries-queries_before;
    std::printf("Snapshot track-block queries: %u\n",snapshot_queries);
    check(snapshot_queries<=40u,"HUD snapshot repeats whole-session scans per parameter");

    check(snap.a[0]==0x10000000 && snap.b[0]==0x60000000 && snap.s[0]==0x30000000 && snap.current[0]==0x30000000,"HUD snapshot values wrong");
    morphcapture::DisplayState ds={};opxy::draw::Input di={};morphcapture::Frame df={};di.track=0;
    morphcapture::display(ds,di,df);
    check(opxy::draw::valid(df),"Instrument rendered invalid draw frame");
    di.track=11;morphcapture::display(ds,di,df);
    check(opxy::draw::valid(df),"AUX rendered invalid draw frame");
    check(df.texts[27].chars[0]=='A' && df.texts[28].chars[0]=='B' && df.texts[29].chars[0]=='S' && df.texts[30].chars[0]=='M',"AUX labels missing");
    check(std::strcmp(df.texts[22].chars,"A captured: 8/8")==0 && std::strcmp(df.texts[23].chars,"B captured: 8/8")==0,"AUX capture counts wrong");
    check(std::strcmp(df.texts[24].chars,"BYPASS")==0 && std::strcmp(df.texts[25].chars,"0%")==0,"AUX bypass wrong");
    const i32 saved_travel=state.travel;const u32 saved_mode=state.mode;
    state.mode=1u;state.travel=-32;morphcapture::display(ds,di,df);
    check(std::strcmp(df.texts[24].chars,"LIVE")==0 && std::strcmp(df.texts[25].chars,"50% to A")==0,"AUX A percentage wrong");
    state.travel=64;morphcapture::display(ds,di,df);
    check(std::strcmp(df.texts[25].chars,"100% to B")==0,"AUX B endpoint wrong");
    state.mode=saved_mode;state.travel=saved_travel;
    // Each instrument and M1/M2/M3 must survive the boundary.
    for(u32 t=0;t<8;++t){held_keys[14+selected]=0;selected=t;held_keys[14+t]=1;
      for(module_word=0;module_word<3;++module_word){frame();check(order.to==219,"HUD route missing on a track/page");check(sdk_morph_hud_snapshot_read(t,&snap),"Track snapshot unavailable");}}
    held_keys[21]=0;selected=0;held_keys[14]=1;module_word=1;frame();
    // Held K4 sweep through real frame-owned mailbox, then actual SDK writes.
    for(u32 n=0;n<16;++n){check(morphcapture::sdk_morph_hud_raw_turn(0,3,4,1),"HUD sweep offer rejected");sdk_gui_tick();drain();frame();}
    check(state.travel==64,"Did not reach B endpoint");verify_all(0x60000000);
    check(morphcapture::sdk_morph_hud_physical_press(3),"BYPASS click denied");sdk_gui_tick();drain();verify_all(0x30000000);
    check(!state.mode,"Did not return to BYPASS");
    frame();check(morphcapture::sdk_morph_hud_raw_turn(0,3,4,1),"Review sweep denied");sdk_gui_tick();drain();
    capture(3);
    for(u32 n=0;n<80 && state.phase==morphcapture::sending;++n)sdk_gui_tick();
    check(state.target_bypass && state.phase==morphcapture::verify_first,"Missing S verification");
    aux_events.turn(3,1);sdk_gui_tick();
    check(state.target_bypass && state.travel==4,"AUX interrupted return to S");
    drain();verify_all(0x30000000);
    // Scene edits are deferred, never writes from the compositor/input path.
    frame();const i32 old_a=state.scene[0][0][8];u32 old_sets=sets;
    check(morphcapture::sdk_morph_hud_raw_turn(0,0,1,0),"Scene edit offer denied");
    check(state.scene[0][0][8]==old_a && sets==old_sets,"Input mutated before controller consumption");
    sdk_gui_tick();check(state.scene[0][0][8]>old_a && sets==old_sets,"Scene edit did not stay scene-only");
    // Real compositor samples two raw pulse words before offering one detent.
    const i32 before_raw=state.scene[0][0][8];
    frame();raw_words[0]=1;frame();sdk_gui_tick();
    check(state.scene[0][0][8]==before_raw,"Half detent changed scene");
    raw_words[0]=2;frame();sdk_gui_tick();
    check(state.scene[0][0][8]==before_raw+0x02000000,"Compositor raw detent did not reach controller");
    module_word=0;frame();
    check(morphcapture::sdk_morph_hud_raw_turn(0,0,1,0),"Basic M1 mapping edit denied");sdk_gui_tick();
    module_word=1;frame();
    // SHIFT endpoint gestures run outside callback, consume on the next tick.
    frame();shifted=true;frame();held_keys[12]=1;frame();held_keys[12]=0;frame();
    sdk_gui_tick();drain();
    check(state.travel==-64 && state.mode,"SHIFT K3 did not jump to A");
    check(static_cast<i32>(bases[1][8])==0x10000000,"SHIFT jump wrote wrong endpoint");
    shifted=false;frame();
    check(morphcapture::sdk_morph_hud_physical_press(2),"Late SHIFT K3 click not swallowed");
    sdk_gui_tick();check(state.base[0][0]==0x30000000,"Late SHIFT click overwrote independent S");
    shifted=true;frame();held_keys[12]=held_keys[13]=1;frame();sdk_gui_tick();drain();
    check(!state.mode,"SHIFT K3/K4 chord did not bypass");verify_all(0x30000000);
    held_keys[12]=held_keys[13]=0;frame();shifted=false;frame();
    // Latch holds after track release; INSTRUMENT clears it.
    shifted=true;held_keys[1]=1;frame();held_keys[1]=0;shifted=false;held_keys[14]=0;frame();
    check(order.to==219 && sdk_morph_hud_frame_authorized(0),"Latch did not retain HUD");
    held_keys[2]=1;frame();check(!sdk_morph_hud_frame_authorized(0),"INSTRUMENT did not clear latch");
    held_keys[2]=0;frame();check(!sdk_morph_hud_frame_authorized(0),"Cleared latch reappeared");
    held_keys[14]=1;frame();
    // V1 latch starts with no held track and no prior frame authorization.
    held_keys[14]=0;frame();shifted=true;frame();held_keys[1]=1;frame();
    held_keys[1]=0;shifted=false;frame();
    check(order.to==219 && sdk_morph_hud_frame_authorized(0),"Global SHIFT+TEMPO latch requires a prior held HUD");
    selected=1;frame();check(sdk_morph_hud_frame_authorized(1),"V1 latch did not follow instrument track");selected=0;frame();
    held_keys[2]=1;frame();held_keys[2]=0;frame();
    check(!sdk_morph_hud_frame_authorized(0),"INSTRUMENT did not clear global latch");
    // SHIFT then Track must leave ALL four encoder clicks with stock menus.
    shifted=true;frame();held_keys[14]=1;frame();
    check(order.to==0 && !sdk_morph_hud_frame_authorized(0),"SHIFT-first stock menu was captured by Morph");
    for(u32 k=0;k<4;++k)check(!morphcapture::sdk_morph_hud_physical_press(k),"Stock-menu encoder click swallowed");
    held_keys[14]=0;shifted=false;frame();held_keys[14]=1;frame();
    // Track then SHIFT reveals the original shifted parameters and edits them.
    shifted=true;frame();
    check(sdk_morph_hud_snapshot_read(0,&snap) && snap.shifted,"Track-first SHIFT view absent");
    check(snap.a[12]==state.scene[0][0][12] && snap.a[13]==state.scene[0][0][13],"Shift scene values wrong");
    di.track=0;morphcapture::display(ds,di,df);
    bool shift_title=false;for(u32 i=0;i<23;++i)if(df.texts[i].chars[0]=='S' && df.texts[i].chars[1]=='H' && df.texts[i].chars[2]=='I')shift_title=true;
    check(shift_title,"SHIFT title not rendered");
    const i32 old_shift=state.scene[0][0][12];
    check(!morphcapture::sdk_morph_hud_raw_turn(0,0,1,0),"Unsupported SHIFT M2 K1 edit accepted");
    check(morphcapture::sdk_morph_hud_raw_turn(0,1,1,0),"SHIFT M2 K2 edit denied");sdk_gui_tick();
    check(state.scene[0][0][12]==old_shift+0x02000000,"SHIFT M2 K2 did not edit slot 13");
    module_word=2;frame();const i32 old_send=state.scene[0][0][18];
    check(morphcapture::sdk_morph_hud_raw_turn(0,0,1,0),"SHIFT M3 send edit denied");sdk_gui_tick();
    check(state.scene[0][0][18]==old_send+0x02000000,"SHIFT M3 did not edit slot 20");
    module_word=0;frame();check(!morphcapture::sdk_morph_hud_raw_turn(0,0,1,0),"M1 SHIFT fine alias should remain read-only as V1");
    shifted=false;module_word=1;frame();
    // Several GUI input ticks can occur between rendered frames. Do not drop them.
    sdk_gui_tick();const i32 old_tick_scene=state.scene[0][0][8];
    raw_words[0]=100;sdk_gui_tick();raw_words[0]=101;sdk_gui_tick();
    check(state.scene[0][0][8]==old_tick_scene+0x02000000,"Raw ticks between render frames lost a detent");
    // Do not drop continued K4 travel during the multi-tick write/verify cycle.
    for(u32 n=0;n<12u;++n) {
      check(morphcapture::sdk_morph_hud_raw_turn(0,3,1,1),"Continuous sweep offer denied");
      sdk_gui_tick();check(state.travel==static_cast<i32>(n+1u),"In-flight sweep dropped a detent");
    }
    drain();
    const i32 saved_a=state.scene[0][0][0],saved_b=state.scene[1][0][0],other_s=state.base[1][0];
    // Actual scramble completion is delivered after stock changes its stored values.
    for(u32 p=0;p<control::track_parameters;++p)bases[0][p]=0x23456789u;
    const u32 scramble_sets=sets;
    check(sdk_morph_scramble_done!=nullptr,"Scramble subscriber missing");
    sdk_morph_scramble_done(0);sdk_gui_tick();
    check(state.base[0][0]==0x23456789 && state.base[1][0]==other_s,"Scramble S refresh crossed tracks");
    check(state.scene[0][0][0]==saved_a && state.scene[1][0][0]==saved_b,"Scramble overwrote A/B");
    check(!state.mode && !state.origin_ready && state.phase==morphcapture::idle,"Scramble retained old morph trajectory");
    check(sets==scramble_sets && bases[0][0]==0x23456789u,"Scramble refresh wrote to stock");
    check(sdk_morph_hud_snapshot_read(0,&snap) && snap.s[0]==0x23456789,"HUD S did not refresh after scramble");
    // A scramble also cancels a partially applied sweep before more writes.
    check(morphcapture::sdk_morph_hud_raw_turn(0,3,1,1),"Second sweep denied");sdk_gui_tick();
    for(u32 p=0;p<control::track_parameters;++p)bases[0][p]=0x34567890u;
    const u32 pending_sets=sets;sdk_morph_scramble_done(0);sdk_gui_tick();
    check(sets==pending_sets && state.base[0][0]==0x34567890,"Pending morph overwrote scramble");
    // Read failure must not partly replace S.
    const i32 saved_s=state.base[0][0];bases[0][1]=0xffffffffu;
    sdk_morph_scramble_done(0);sdk_gui_tick();
    check(state.base[0][0]==saved_s && state.fault==14u,"Failed scramble read partially changed S");
    bases[0][1]=0x45678901u;
    aux_events.turn(3,1);sdk_gui_tick();
    check(state.base[0][1]==0x45678901 && !state.fault,"Scramble refresh did not recover");
    check(!state.mode && state.phase==morphcapture::idle,"Recovery replayed stale movement");
    frame(77u);check(sdk_morph_gui_route_trace(0)==77u && sdk_morph_gui_route_trace(2)==7u,"Route trace missed selected HUD");
    selected=11u;frame(219u);check(sdk_morph_gui_route_trace(0)==77u,"AUX erased instrument route trace");selected=0;frame();
    // Registered-screen and stock-overlay guards still close input ownership.
    frame(0);check(!sdk_morph_hud_frame_authorized(0),"Page zero retained authorization");
    module_word=0;frame(1);
    check(order.from==1u && order.to==219u && sdk_morph_hud_frame_authorized(0),"WaveTbl M1 page 1 denied HUD");
    check(sdk_morph_gui_route_trace(0)==1u && sdk_morph_gui_route_trace(2)==7u,"WaveTbl route trace wrong");
    check(sdk_morph_hud_snapshot_read(0,&snap) && snap.m1.primary[0]==0u,"Page 1 M1 values unavailable");
    // Other stock and SDK-assigned pages use the same generic gesture gate.
    for(u32 page=2u;page<256u;++page)if(page!=219u){frame(page);check(order.to==219u,"Generic instrument page denied HUD");}
    // SHIFT-first remains stock-owned even when its selected page is 1.
    held_keys[14]=0;shifted=false;frame(1);shifted=true;frame(1);held_keys[14]=1;frame(1);
    check(order.to==0u && !sdk_morph_hud_frame_authorized(0),"Page 1 SHIFT-first menu intercepted");
    for(u32 k=0;k<4;++k)check(!morphcapture::sdk_morph_hud_physical_press(k),"Page 1 stock menu click swallowed");
    shifted=false;frame(1);shifted=true;frame(1);
    check(order.to==219u,"Page 1 Track-first SHIFT lost HUD");
    shifted=false;module_word=1;frame();
    registered=false;frame();check(!sdk_morph_hud_frame_authorized(0),"Unregistered page authorized");registered=true;
    held_keys[15]=1;frame();check(!sdk_morph_hud_frame_authorized(0),"Other held track authorized");held_keys[15]=0;
    module_word=3;frame();check(!sdk_morph_hud_frame_authorized(0),"Unsupported page authorized");module_word=1;
    // A stored session is revalidated outside the callback too.
    project_loading=1;frame();check(!sdk_morph_hud_snapshot_read(0,&snap),"Loading project exposed stale scene");project_loading=0;
    project_bytes[6]=1;check(!sdk_morph_hud_snapshot_read(0,&snap),"Changed scene exposed stale scene");project_bytes[6]=0;
    pattern[0]=1;check(!sdk_morph_hud_snapshot_read(0,&snap),"Changed block exposed stale scene");pattern[0]=0;
    aux_engines[11]=0;check(!sdk_morph_hud_snapshot_read(0,&snap),"Absent owner exposed stale scene");aux_engines[11]=42;
    frame();check(sdk_morph_hud_snapshot_read(0,&snap),"Valid session did not recover after rejected reads");
    snap.abi=12345;
    bases[0][0]=0xffffffffu;
    check(!sdk_morph_hud_snapshot_read(0,&snap) && snap.abi==12345,"Failed CURRENT read partially published snapshot");
    bases[0][0]=0x30000000;
    check(!sdk_morph_hud_snapshot_read(8,&snap),"Invalid instrument track read accepted");
    check(!sdk_morph_hud_snapshot_read(0,nullptr),"Null snapshot accepted");
    project_word=0;check(!sdk_morph_hud_snapshot_read(0,&snap),"Absent project exposed snapshot");project_word=kProjectAt;
    snap.abi=6789;invalidate_on_value_read=true;
    check(!sdk_morph_hud_snapshot_read(0,&snap) && snap.abi==6789,"Session changed during read but snapshot was published");
    project_bytes[6]=0;
    check(!sdk_morph_stored_once_submit(0,0,123),"GUI testing unlocked writer");
    // Synthetic synth metadata with real SDK packing, queue and controller.
    engines[3]=77;fill(0x10000000);bases[3][36]=0;bases[3][37]=0;
    morphcapture::control_reset(state);sdk_gui_tick();
    check(state.base[3][26]==55 && state.base[3][27]==2,"SHIFT defaults not captured on unselected track");
    check(state.base[3][28]==0 && state.base[0][26]==0,"Absent SHIFT or ordinary track mishandled");
    bases[3][36]=opxy::platform::synth_shift::with(0,0,95,shift_schema.defaults);
    bases[3][36]=opxy::platform::synth_shift::with(bases[3][36],1,3,shift_schema.defaults);
    bases[3][37]=opxy::platform::synth_shift::with(0,7,87,shift_schema.defaults);
    capture(1);capture(3);aux_events.turn(3,4);sdk_gui_tick();
    state.travel=64;morphcapture::schedule(state,false);drain();
    check(opxy::platform::synth_shift::field(bases[3][36],0,shift_schema.defaults)==95,"SHIFT B endpoint wrong");
    capture(3);drain();
    check(opxy::platform::synth_shift::field(bases[3][36],0,shift_schema.defaults)==55,"SHIFT S restoration wrong");
    check(opxy::platform::synth_shift::field(bases[3][36],1,shift_schema.defaults)==2,"Packed neighbour lost");
    check(opxy::platform::synth_shift::field(bases[3][37],7,shift_schema.defaults)==0,"Second SHIFT page not restored");
    shift_schema.choices[1]=5;check(!morph_check(morph_session),"Schema change accepted");
    shift_schema.choices[1]=4;engines[3]=78;check(!morph_check(morph_session),"Engine change accepted");
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
    for(u32 step=0;step<128u;++step)host_aux_masks[step][0]=host_aux_masks[step][1]=0;
    morphcapture::recorded_reference=true;state.mode=1u;state.phase=morphcapture::idle;
    state.travel=0;morphcapture::movement_lane.primed=false;
    value(11,3)=morphcapture::automation::encode(64);sdk_gui_tick();
    check(state.travel==0,"Erased lane continued driving playback");
    playing[0]=0;
    // Gate33: actual controller/SDK, with stock input/displayed-LFO doubles.
    selected=11u;playing[0]=0;cv_lfo_on=0;telemetry25=0;
    host_recording=false;
    for(u32 t=0;t<8u;++t)engines[t]=0;
    morphcapture::control_reset(state);fill(0x30000000);sdk_gui_tick();
    fill(0x10000000);capture(0);fill(0x60000000);capture(1);
    fill(0x30000000);capture(2);
    page_key=2;sdk_gui_tick();
    check(morphcapture::aux_page==2u && state.active_tracks==255u,"M2 did not show default all-track routing");
    aux_events.turn(0,-1);sdk_gui_tick();
    check(state.active_tracks==254u && !state.mode,"M2 turn affected movement or failed to disable track 1");
    capture(2);
    check(morphcapture::route_bank==1u && state.active_tracks==254u,"M2 click must bank without SET S");
    aux_events.turn(3,-1);sdk_gui_tick();
    check(state.active_tracks==126u,"M2 second bank did not disable track 8");
    page_key=1;sdk_gui_tick();
    // Captures still include disabled tracks.
    bases[0][0]=0x17000000;capture(0);
    check(state.scene[0][0][0]==0x17000000 && state.captured_mask[0]==255u,"Disabled track could not be captured");
    bases[0][0]=0x30000000;
    capture(3);state.travel=64;morphcapture::schedule(state,false);drain();
    check(bases[0][0]==0x30000000 && bases[7][0]==0x30000000 && bases[6][0]==0x60000000,"Route mask failed to isolate morph writes");
    bases[0][0]=0x23000000;capture(3);drain();
    check(bases[0][0]==0x23000000 && bases[6][0]==0x30000000,"BYPASS wrote disabled track or missed selected S");
    page_key=2;sdk_gui_tick();const u32 bypass_routing_writes=writes;
    aux_events.turn(3,1);sdk_gui_tick();
    check(!state.mode && state.phase==morphcapture::idle && writes==bypass_routing_writes,"Routing in BYPASS restored sound unexpectedly");
    aux_events.turn(3,-1);sdk_gui_tick();page_key=1;sdk_gui_tick();
    // Save format 2 uses existing reserved header bits; v1 remains loadable.
    opxy::u8 routed[morphcapture::kSavedBytes]={};
    check(morphcapture::control_save(state,routed,sizeof routed)==sizeof routed,"Routing save failed");
    morphcapture::control_load(state,routed,sizeof routed);sdk_gui_tick();
    check(state.active_tracks==126u && !state.mode,"Routing persistence failed");
    morphcapture::put32(routed+4,1u);morphcapture::put32(routed+28,0u);
    morphcapture::put32(routed+sizeof routed-4,morphcapture::saved_checksum(routed,sizeof routed-4));
    morphcapture::control_load(state,routed,sizeof routed);sdk_gui_tick();
    check(state.active_tracks==255u,"Legacy Gate32 bank must load with all tracks active");
    morphcapture::put32(routed+4,2u);morphcapture::put32(routed+28,2u);
    morphcapture::put32(routed+sizeof routed-4,morphcapture::saved_checksum(routed,sizeof routed-4));
    morphcapture::control_load(state,routed,sizeof routed);
    check(!state.base_ready,"Unknown routing save flag accepted");sdk_gui_tick();
    // LFO page uses the supported SDK shared LFO; no generated stock recording.
    fill(0x10000000);capture(0);fill(0x60000000);capture(1);fill(0x30000000);capture(2);
    check(hosts[11]->shared_lfo,"Test machine is not configured with shared LFO");
    cv_lfo_on=1;telemetry25=0xab030001u;
    value(11,3)=morphcapture::automation::encode(0);lfo[11][3]=0x20000000;
    const u32 before_lfo_recorded=host_recorded;
    capture(3);drain();
    check(state.travel==32 && bases[7][0]>0x30000000u,"Stopped-transport LFO failed to move selected tracks");
    check(host_recorded==before_lfo_recorded,"LFO wrote an automation lane");
    // Continuously changing targets must let the final track and readback finish.
    bool saw_idle=false;
    for(u32 tick=0;tick<180u;++tick){
      lfo[11][3]=(tick&1u)?0x10000000:0x30000000;sdk_gui_tick();
      saw_idle|=state.phase==morphcapture::idle;
      check(!state.fault,"Continuous LFO faulted the guarded transaction");
    }
    check(saw_idle && bases[7][0]!=0x30000000u,"LFO starved later tracks or verification");
    capture(3);drain();
    for(u32 tick=0;tick<40;++tick)sdk_gui_tick();
    check(!state.mode && bases[7][0]==0x30000000u,"LFO undid manual bypass");
    // Wrong LFO destination cannot trigger movement; zero crossings still count.
    telemetry25=0xab020001u;lfo[11][3]=0;capture(3);sdk_gui_tick();
    check(state.travel==0,"Unrelated LFO destination moved Morph");
    telemetry25=0xab030000u;value(11,3)=morphcapture::automation::encode(-64);sdk_gui_tick();drain();
    check(state.travel==-64 && bases[7][0]==0x10000000u,"Zero-amount valid LFO destination lost A endpoint");
    value(11,3)=morphcapture::automation::encode(64);sdk_gui_tick();drain();
    check(state.travel==64 && bases[7][0]==0x60000000u,"LFO B endpoint not exact");
    // A page change drops queued capture/movement; switching participation
    // during a send must not write excluded later tracks or poison readback.
    state.travel=-64;morphcapture::schedule(state,false);sdk_gui_tick();
    page_key=2;aux_events.click(0);aux_events.turn(3,4);sdk_gui_tick();
    check(morphcapture::aux_page==2u && morphcapture::route_bank==0u,"Page switch reinterpreted an old gesture");
    capture(0); // second routing bank
    const u32 final_before_disable=bases[7][0];
    aux_events.turn(3,-1);sdk_gui_tick();drain();
    check(!(state.active_tracks&128u) && bases[7][0]==final_before_disable,"Mid-send exclusion still wrote the final track");
    aux_events.turn(3,1);sdk_gui_tick();drain();
    check((state.active_tracks&128u) && bases[7][0]==0x10000000u,"Re-enabled track did not catch up to movement");
    page_key=1;sdk_gui_tick();
    // All tracks disabled must finish with no writes, even for SHIFT fields.
    state.active_tracks=0u;const u32 writes_before_disabled=writes;
    state.travel=-64;morphcapture::schedule(state,false);drain();
    check(writes==writes_before_disabled,"All-disabled collection still wrote sound values");
    cv_lfo_on=0;telemetry25=0;lfo[11][3]=0;
    {
    // Gate34: unchanged destinations are still bounded to 16 inspections.
    cv_lfo_on=0;telemetry25=0;lfo[11][3]=0;selected=11;
    morphcapture::control_reset(state);fill(0x30000000);sdk_gui_tick();
    capture(3);state.travel=0;morphcapture::schedule(state,false);
    host_batch_reads=0;sdk_gui_tick();
    check(host_batch_reads<=16u && state.cursor==16u,"Unchanged send scanned beyond the inspection budget");
    drain();
    state.phase=morphcapture::verify_first;state.cursor=0;
    host_batch_reads=0;sdk_gui_tick();
    check(host_batch_reads<=16u && state.cursor==16u,"Verification exceeded inspection budget");drain();
    i32 untouched[2]={123,456};const u32 idx[2]={0,1};
    check(!sdk_morph_stored_batch_read(0,idx,2,untouched) && untouched[0]==123 && untouched[1]==456,
      "Out-of-callback batch published values");
    // A context change during a batch must not publish a partial capture.
    const i32 old_a=state.scene[0][0][0];
    invalidate_on_value_read=true;capture(0);
    check(state.scene[0][0][0]==old_a,"Interrupted capture published batch data");
    project_bytes[6]=0;sdk_gui_tick();
    // New routing graphics are visible only on AUX; both banks share the mask.
    morphcapture::Frame route_frame={};morphcapture::DisplayState ds={};opxy::draw::Input di={};di.track=11;
    page_key=2;sdk_gui_tick();morphcapture::display(ds,di,route_frame);
    check(route_frame.nodes[16].x==148*opxy::draw::unit && route_frame.nodes[52].x==0,
      "Routing bank indication is wrong");
    di.track=0;morphcapture::display(ds,di,route_frame);
    for(u32 n=8;n<63;++n)check(route_frame.nodes[n].x==640*opxy::draw::unit,"AUX artwork leaked into instrument HUD");
    }
    {
     selected=11;page_key=2;sdk_gui_tick();
     morphcapture::Frame vf={};morphcapture::DisplayState vs={};opxy::draw::Input vi={};vi.track=11;
     const u32 masks[]={0u,1u,85u,255u};
     for(u32 m=0;m<4;++m)for(u32 bank=0;bank<2;++bank){
      state.active_tracks=masks[m];morphcapture::route_bank=bank;sdk_gui_tick();
      morphcapture::display(vs,vi,vf);check(opxy::draw::valid(vf),"Routing frame invalid");
      for(u32 t=0;t<8;++t){
       const bool on=(masks[m]&(1u<<t))!=0;
       check(vf.nodes[(on?36u:44u)+t].x==static_cast<i32>(148+32*t)*opxy::draw::unit,"Wrong routing toggle pose");
       check(std::strcmp(vf.texts[18+t].chars,on?"on":"off")==0,"Wrong routing status label");
      }
     }
     page_key=1;sdk_gui_tick();morphcapture::display(vs,vi,vf);
     check(vf.nodes[61].x==304*opxy::draw::unit && vf.nodes[61].y==119*opxy::draw::unit,"M1 logo misplaced");
     check(vf.texts[22].y==vf.texts[23].y && vf.texts[22].y==78*opxy::draw::unit,"M1 capture labels misaligned");
     check(vf.texts[27].chars[0]=='A' && vf.texts[30].chars[0]=='M',"M1 header not restored after routing");
     for(u32 t=18;t<22;++t)check(!vf.texts[t].chars[0],"M2 status leaked onto M1");
    }
    std::printf("Gate33 additional state: %zu; page/bank/last-LFO statics: %zu; payload %u bytes\n",
      sizeof(state)-4404u,sizeof(morphcapture::aux_page)+sizeof(morphcapture::route_bank)+sizeof(morphcapture::lfo_was_active),morphcapture::kSavedBytes);
    std::printf("Gate32 identified new static bytes (before linker padding): %zu; persistence bytes: %u\n",
      sizeof(morphcapture::recorded_origin)+sizeof(morphcapture::movement_lane)+
      sizeof(morphcapture::restored_schema)+sizeof(morphcapture::restored_patterns)+
      sizeof(morphcapture::recorded_reference)+sizeof(morphcapture::transport_playing)+sizeof(morphcapture::restored_pending),morphcapture::kSavedBytes);
    std::printf("PASS: %u boundary, scene, compositor, snapshot and isolation checks; state=%zu bytes\n",checks,sizeof state);
}
