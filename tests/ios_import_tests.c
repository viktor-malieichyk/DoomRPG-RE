#include "ImportData.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char** argv) {
 assert(argc==2); FILE* file=fopen(argv[1],"rb"); assert(file);
 fseek(file,0,SEEK_END); long length=ftell(file); rewind(file);
 unsigned char* bytes=malloc(length); assert(bytes && fread(bytes,1,length,file)==length); fclose(file);
 assert(IOS_ValidateZip(bytes,length));
 assert(!IOS_ValidateZip(bytes,0)); assert(!IOS_ValidateZip(bytes,length-1));
 bytes[0]^=1; assert(!IOS_ValidateZip(bytes,length)); bytes[0]^=1;
 bytes[length/2]^=1; assert(!IOS_ValidateZip(bytes,length));
 free(bytes); puts("iOS import tests passed: valid data, empty/truncated/corrupt archive rejection"); return 0;
}
