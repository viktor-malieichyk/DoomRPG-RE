#import "SaveArchive.h"
#include <assert.h>
int main(void) { @autoreleasepool {
 NSFileManager* fm=NSFileManager.defaultManager;
 NSString* root=[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString];
 NSString* game=[root stringByAppendingPathComponent:@"Game"];
 [fm createDirectoryAtPath:game withIntermediateDirectories:YES attributes:nil error:nil];
 NSMutableDictionary* files=[NSMutableDictionary dictionary];
 unsigned char bytes[]={23,0,0,0,1,2,3,4};
 for(NSString* n in @[@"Config",@"Player",@"Player2",@"World"]) {
  files[n]=[NSData dataWithBytes:bytes length:sizeof(bytes)];
  [files[n] writeToFile:[game stringByAppendingPathComponent:n] atomically:YES];
 }
 NSData* sentinel=[@"game asset" dataUsingEncoding:NSUTF8StringEncoding];
 [sentinel writeToFile:[game stringByAppendingPathComponent:@"DoomRPG.zip"] atomically:YES];
 NSError* e=nil; NSData* original=DRPGExportSave(game,&e); assert(original && !e);
 assert(DRPGReadSave(original,&e).count==4);
 assert(!DRPGReadSave([original subdataWithRange:NSMakeRange(0,10)],&e));
 NSMutableDictionary* bad=[files mutableCopy]; [bad removeObjectForKey:@"World"];
 NSData* broken=[NSPropertyListSerialization dataWithPropertyList:@{@"format":@1,@"files":bad} format:NSPropertyListBinaryFormat_v1_0 options:0 error:nil];
 assert(!DRPGReadSave(broken,&e));
 assert(!DRPGRestoreSave(bad,game,&e));
 assert([[NSData dataWithContentsOfFile:[game stringByAppendingPathComponent:@"Player"]] isEqual:files[@"Player"]]);
 bad=[files mutableCopy]; bad[@"Player"]=[@"replacement player" dataUsingEncoding:NSUTF8StringEncoding];
 e=nil; assert(DRPGRestoreSave(bad,game,&e) && !e);
 assert([[NSData dataWithContentsOfFile:[game stringByAppendingPathComponent:@"Player"]] isEqual:bad[@"Player"]]);
 assert([[NSData dataWithContentsOfFile:[game stringByAppendingPathComponent:@"DoomRPG.zip"]] isEqual:sentinel]);
 NSArray* dirs=[fm contentsOfDirectoryAtPath:root error:nil]; assert(dirs.count==2);
 NSString* backup=[dirs[0] isEqual:@"Game"] ? dirs[1] : dirs[0];
 assert([[NSData dataWithContentsOfFile:[[root stringByAppendingPathComponent:backup] stringByAppendingPathComponent:@"Player"]] isEqual:files[@"Player"]]);
 [fm removeItemAtPath:root error:nil];
 puts("iOS save tests passed: round trip, corrupt/missing data rejection, restore, old-save backup, asset preservation");
 } return 0; }
