// Stock file calls doubled for controller integration tests.
#include <map>
#include <string>
#include <vector>
#include <cstring>
static std::map<std::string,std::vector<opxy::u8> > preset_files;
static opxy::u32 preset_opens=0;
static bool preset_short_write=false,preset_close_error=false,preset_fail_open=false;
struct PresetFile {std::string name;opxy::u32 pos;};
static PresetFile preset_file;
void* host_preset_open(const char* path,const char* mode){
 ++preset_opens;if(preset_fail_open)return nullptr;
 const std::string name(path);
 if(mode[0]=='r' && !preset_files.count(name))return nullptr;
 if(mode[0]=='w')preset_files[name].clear();
 preset_file.name=name;preset_file.pos=0;return &preset_file;
}
opxy::u32 host_preset_read(void* out,opxy::u32 size,opxy::u32 count,void*){
 auto& data=preset_files[preset_file.name];opxy::u32 n=size*count;
 if(n>data.size()-preset_file.pos)n=static_cast<opxy::u32>(data.size()-preset_file.pos);
 if(n)std::memcpy(out,data.data()+preset_file.pos,n);preset_file.pos+=n;return n/size;
}
opxy::u32 host_preset_write(const void* in,opxy::u32 size,opxy::u32 count,void*){
 opxy::u32 n=size*count;if(preset_short_write)n/=2;
 const opxy::u8* p=static_cast<const opxy::u8*>(in);
 preset_files[preset_file.name].assign(p,p+n);return n/size;
}
int host_preset_close(void*){return preset_close_error?-1:0;}
