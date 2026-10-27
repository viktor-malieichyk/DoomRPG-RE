#import <Foundation/Foundation.h>
NSData* DRPGExportSave(NSString* folder, NSError** error);
NSDictionary* DRPGReadSave(NSData* data, NSError** error);
BOOL DRPGRestoreSave(NSDictionary* files, NSString* folder, NSError** error);
