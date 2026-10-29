#import "SaveArchive.h"
static NSArray* saveNames(void) { return @[@"Config",@"Player",@"Player2",@"World"]; }
static void invalid(NSError** error) {
 if(error)*error=[NSError errorWithDomain:@"DoomRPG" code:1 userInfo:@{NSLocalizedDescriptionKey:@"Select a complete Doom RPG save backup (.doomsave), version 1."}];
}
NSDictionary* DRPGReadSave(NSData* data,NSError** error) {
 if(!data || data.length>8*1024*1024) { invalid(error); return nil; }
 id root=[NSPropertyListSerialization propertyListWithData:data options:NSPropertyListImmutable format:nil error:error];
 if(![root isKindOfClass:NSDictionary.class] || ![root[@"format"] isEqual:@1] || ![root[@"files"] isKindOfClass:NSDictionary.class]) { invalid(error); return nil; }
 NSDictionary* files=root[@"files"];
 if(files.count!=4) { invalid(error); return nil; }
 for(NSString* n in saveNames()) {
  NSData* d=files[n];
  if(![d isKindOfClass:NSData.class] || d.length<4 || d.length>2*1024*1024) { invalid(error); return nil; }
 }
 const unsigned char* config=[files[@"Config"] bytes];
 if(config[0]!=23 || config[1] || config[2] || config[3]) { invalid(error); return nil; }
 return files;
}
NSData* DRPGExportSave(NSString* folder,NSError** error) {
 if (![folder isKindOfClass:NSString.class]) { invalid(error); return nil; }
 NSMutableDictionary* files=[NSMutableDictionary dictionary];
 for(NSString* n in saveNames()) {
  NSData* data=[NSData dataWithContentsOfFile:[folder stringByAppendingPathComponent:n] options:0 error:error];
  if(!data)return nil;
  files[n]=data;
 }
 NSData* data=[NSPropertyListSerialization dataWithPropertyList:@{@"format":@1,@"files":files} format:NSPropertyListBinaryFormat_v1_0 options:0 error:error];
 return DRPGReadSave(data,error) ? data : nil;
}
BOOL DRPGRestoreSave(NSDictionary* files,NSString* folder,NSError** error) {
 if (![folder isKindOfClass:NSString.class]) { invalid(error); return NO; }
 // Revalidate even when called outside the document picker.
 NSData* archive=[NSPropertyListSerialization dataWithPropertyList:@{@"format":@1,@"files":files} format:NSPropertyListBinaryFormat_v1_0 options:0 error:error];
 if(!DRPGReadSave(archive,error))return NO;
 NSFileManager* fm=NSFileManager.defaultManager;
 NSString* parent=folder.stringByDeletingLastPathComponent;
 NSString* stage=[parent stringByAppendingPathComponent:[@"Import " stringByAppendingString:NSUUID.UUID.UUIDString]];
 if(![fm copyItemAtPath:folder toPath:stage error:error])return NO;
 for(NSString* n in saveNames()) {
  if(![files[n] writeToFile:[stage stringByAppendingPathComponent:n] options:NSDataWritingAtomic error:error]) { [fm removeItemAtPath:stage error:nil]; return NO; }
 }
 NSString* backup=[parent stringByAppendingPathComponent:[@"Previous Saves " stringByAppendingString:NSUUID.UUID.UUIDString]];
 if(![fm moveItemAtPath:folder toPath:backup error:error]) { [fm removeItemAtPath:stage error:nil]; return NO; }
 if(![fm moveItemAtPath:stage toPath:folder error:error]) {
  [fm moveItemAtPath:backup toPath:folder error:nil]; return NO;
 }
 return YES;
}
