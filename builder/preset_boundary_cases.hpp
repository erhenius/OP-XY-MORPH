void check_presets() {
 using namespace morphcapture;
 const ControlState initial=state;
 const u32 old_sets=sets,old_writes=writes;
 selected=11u;page_key=3u;sdk_gui_tick();capture(3);
 check(preset_panel,"M3 K4 did not open presets");
 // Distinct A/B, with a distinct destination S/origin to detect contamination.
 for(u32 b=0;b<2;++b)for(u32 t=0;t<8;++t)for(u32 p=0;p<26;++p)
  state.scene[b][t][p]=static_cast<i32>(0x10000000u+b*0x10000000u+t*100u+p);
 const ControlState source=state;
 capture(2);check(preset_status==4u && preset_files.empty(),"Save was not confirmed");capture(2);
 check(preset_status==1u,"Preset save failed");
 check(preset_files.count("/fat32/morph-ab-01a.mab")==1u,"Wrong preset path");
 check(preset_files.begin()->second.size()==morph_presets::bytes,"Wrong preset size");
 for(u32 b=0;b<2;++b)for(u32 t=0;t<8;++t)for(u32 p=0;p<26;++p)state.scene[b][t][p]=0x40000000;
 capture(1);check(preset_status==2u,"Same instrument recall failed");
 check(std::memcmp(state.scene,source.scene,sizeof state.scene)==0,"A/B roundtrip changed values");
 check(std::memcmp(state.base,source.base,sizeof state.base)==0,"Preset recall replaced S");
 check(std::memcmp(state.origin,source.origin,sizeof state.origin)==0,"Preset recall replaced origin");
 // Change source instrument identity on track 1; checksummed valid file.
 auto& disk=preset_files["/fat32/morph-ab-01a.mab"];
 morph_presets::put(disk.data()+24,999u);
 morph_presets::put(disk.data()+morph_presets::bytes-4,morph_presets::checksum(disk.data()));
 for(u32 b=0;b<2;++b)for(u32 p=0;p<34;++p)state.scene[b][0][p]=p>=26?0:0x40000000;
 capture(1);check(preset_status==3u && preset_partial==1u,"Mismatch not reported per track");
 for(u32 b=0;b<2;++b)for(u32 p=0;p<34;++p) {
  const i32 expected=p<8?0x40000000:p<26?source.scene[b][0][p]:0;
  check(state.scene[b][0][p]==expected,"Mismatch overwrote M1/SHIFT or missed M2/M3");
 }
 check(std::memcmp(state.scene[0][1],source.scene[0][1],sizeof state.scene[0][1])==0,"Matching track not restored");
 // Corruption/oversize must leave the entire controller state untouched.
 ControlState before=state;disk[50]^=1u;capture(1);
 check(preset_status==6u && std::memcmp(&before,&state,sizeof state)==0,"Corrupt preset mutated controller");
 disk[50]^=1u;disk.push_back(0);capture(1);
 check(preset_status==6u && std::memcmp(&before,&state,sizeof state)==0,"Overlong preset accepted");disk.pop_back();
 // Failed replacement keeps the prior valid generation available.
 capture(2);preset_short_write=true;capture(2);preset_short_write=false;
 check(preset_status==7u,"Short save reported success");capture(1);
 check(preset_status==3u,"Interrupted save destroyed previous valid preset");
 capture(2);capture(2);check(preset_status==1u,"Second generation save failed");
 check(preset_files.count("/fat32/morph-ab-01b.mab"),"Replacement did not use alternate copy");
 // Newest copy wins; a damaged latest copy falls back to the older pair.
 const i32 newest_value=state.scene[0][0][0];state.scene[0][0][0]=123;
 capture(1);check(preset_status==2u && state.scene[0][0][0]==newest_value,"Older generation won recall");
 auto& latest=preset_files["/fat32/morph-ab-01b.mab"];latest[20]^=1;
 capture(1);check(preset_status==3u,"Damaged latest copy did not fall back");latest[20]^=1;
 // Empty slot, last slot path and confirmation expiry.
 aux_events.turn(0,1);sdk_gui_tick();capture(1);check(preset_status==6u,"Empty slot loaded stale data");
 preset_slot=15;capture(2);cycles+=400u*1000000u;sdk_gui_tick();
 cycles+=400u*1000000u;sdk_gui_tick();cycles+=400u*1000000u;sdk_gui_tick();
 check(!preset_confirm_us,"Save confirmation never expired");capture(2);
 check(preset_status==4u && !preset_files.count("/fat32/morph-ab-16a.mab"),"Expired confirmation saved");capture(2);
 check(preset_status==1u && preset_files.count("/fat32/morph-ab-16a.mab"),"Final slot inaccessible");
 preset_slot=0;
 // LIVE and transport restrictions perform no I/O and cannot change S/A/B.
 u32 opens=preset_opens;state.mode=1;before=state;capture(1);
 check(preset_status==5u && preset_opens==opens && std::memcmp(state.scene,before.scene,sizeof state.scene)==0,"LIVE recall was allowed");
 check(std::memcmp(state.base,before.base,sizeof state.base)==0,"LIVE preset changed S");state.mode=0;
 playing[1]=1;before=state;capture(1);check(preset_status==2u && preset_opens>opens,"Playback recall refused");check(std::memcmp(state.base,before.base,sizeof state.base)==0,"Playback recall changed S");opens=preset_opens;capture(2);check(preset_status==5u && preset_opens==opens,"Playback save allowed");playing[1]=0;
 // Ordinary GUI ticks never scan preset files; leaving page cancels save.
 for(u32 n=0;n<100;++n)sdk_gui_tick();check(preset_opens==opens,"Background preset I/O");
 capture(2);page_key=1;sdk_gui_tick();check(!preset_panel && !preset_confirm_us,"Page change kept save armed");
 check(sets==old_sets && writes==old_writes,"Preset operations wrote synth parameters");
 // Files survive controller/project reset; recall preserves the new S.
 control_reset(state);sdk_gui_tick();page_key=3;sdk_gui_tick();capture(3);
 const ControlState restarted=state;capture(1);
 check(preset_status==2u,"Saved preset lost across controller reset");
 check(std::memcmp(state.base,restarted.base,sizeof state.base)==0,"Recall imported the old project's S");
 check(state.scene[0][0][0]==newest_value,"Reset recalled wrong A bank");
 // Restore fixture without resetting the SDK session used by the remaining tests.
 state=initial;preset_slot=preset_status=preset_partial=preset_confirm_us=0;preset_panel=false;
 aux_page=1;selected=0;preset_files.clear();
}
