/* Validate an imported game archive before replacing the working copy. */
#include "ImportData.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
static uint16_t u16(const unsigned char* p) { return p[0] | p[1]<<8; }
static uint32_t u32(const unsigned char* p) { return (uint32_t)u16(p) | (uint32_t)u16(p+2)<<16; }
int IOS_ValidateZip(const void* bytes,size_t size) {
 const unsigned char* p=bytes;
 if(!p || size<22 || size>64*1024*1024)return 0;
 size_t end=size-22, stop=size>65557?size-65557:0;
 while(u32(p+end)!=0x06054b50) { if(end==stop)return 0; --end; }
 if(u16(p+end+4) || u16(p+end+6) || u16(p+end+8)!=u16(p+end+10) || end+22+u16(p+end+20)!=size)return 0;
 unsigned count=u16(p+end+10); size_t at=u32(p+end+16), total=0;
 if(!count || count>4096 || at>end || u32(p+end+12)!=end-at)return 0;
 const char* required[]={"entities.db","sintable.bin","palettes.bin","intro.bsp","5039.mid","5040.mid","5043.mid"};
 unsigned found=0;
 for(unsigned i=0;i<count;i++) {
  if(at>end || end-at<46 || u32(p+at)!=0x02014b50)return 0;
  unsigned flags=u16(p+at+8), method=u16(p+at+10), n=u16(p+at+28);
  size_t compressed=u32(p+at+20), unpacked=u32(p+at+24), local=u32(p+at+42);
  size_t next=at+46+n+u16(p+at+30)+u16(p+at+32);
  if(next>end || !n || flags&1 || (method!=0 && method!=8) || unpacked>8*1024*1024 || (total+=unpacked)>64*1024*1024)return 0;
  for(unsigned r=0;r<sizeof(required)/sizeof(*required);r++) if(strlen(required[r])==n && !memcmp(p+at+46,required[r],n))found|=1u<<r;
  if(local>at || at-local<30 || u32(p+local)!=0x04034b50)return 0;
  size_t payload=local+30+u16(p+local+26)+u16(p+local+28);
  if(payload>at || compressed>at-payload)return 0;
  unsigned char* output=malloc(unpacked?unpacked:1); if(!output)return 0;
  int valid=0;
  if(method==0) { valid=compressed==unpacked; if(valid)memcpy(output,p+payload,unpacked); }
  else {
   z_stream stream={0}; stream.next_in=(Bytef*)(p+payload); stream.avail_in=(uInt)compressed;
   stream.next_out=output; stream.avail_out=(uInt)(unpacked?unpacked:1);
   if(inflateInit2(&stream,-MAX_WBITS)==Z_OK) { valid=inflate(&stream,Z_FINISH)==Z_STREAM_END && stream.total_out==unpacked; inflateEnd(&stream); }
  }
  valid=valid && crc32(0,output,(uInt)unpacked)==u32(p+at+16);
  free(output); if(!valid)return 0;
  at=next;
 }
 return at==end && found==((1u<<(sizeof(required)/sizeof(*required)))-1);
}
