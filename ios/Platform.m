#import <UIKit/UIKit.h>
#import <AVFAudio/AVAudioSession.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_syswm.h>
#include "Platform.h"
#import "SaveArchive.h"
#include "ImportData.h"
static BOOL ready, paused, importingSaves, exporting;
static Uint8 touchKeys[SDL_NUM_SCANCODES];
static UIViewController* host;
static NSString* folder;
static UIView* pinPad;
@interface Mobile : NSObject <UIDocumentPickerDelegate>
-(void)menu;
@end
static Mobile* mobile;
static void notice(NSString* message) {
 UIAlertController* a=[UIAlertController alertControllerWithTitle:@"Doom RPG" message:message preferredStyle:UIAlertControllerStyleAlert];
 [a addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){
  paused=NO;
  if(!ready) dispatch_async(dispatch_get_main_queue(), ^{ [mobile menu]; });
 }]];
 [host presentViewController:a animated:YES completion:nil];
}
static BOOL assetsReady(void) {
 NSFileManager* fm=NSFileManager.defaultManager;
 return [fm fileExistsAtPath:[folder stringByAppendingPathComponent:@"DoomRPG.zip"]] && [fm fileExistsAtPath:[folder stringByAppendingPathComponent:@"gm.sf2"]];
}
@implementation Mobile
-(void)down:(UIButton*)button { touchKeys[button.tag]=1; }
-(void)up:(UIButton*)button { touchKeys[button.tag]=0; }
-(void)start {
 if(!assetsReady()) { notice(@"Import DoomRPG.zip and gm.sf2 first. Choose them from iCloud Drive or On My iPhone in Files."); return; }
 ready=YES; [host dismissViewControllerAnimated:YES completion:nil];
}
-(void)pick:(BOOL)saves {
 importingSaves=saves; exporting=NO;
 UIDocumentPickerViewController* picker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypeData] asCopy:YES];
 picker.allowsMultipleSelection=!saves; picker.delegate=self;
 [host presentViewController:picker animated:YES completion:nil];
}
-(void)exportSaves {
 NSError* error=nil;
 NSData* data=DRPGExportSave(folder,&error);
 if(!data) { notice(@"No complete saved game yet. Save using the in-game menu, then export again."); return; }
 NSURL* url=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"DoomRPG-save.doomsave"]];
 if(!data || ![data writeToURL:url options:NSDataWritingAtomic error:&error]) { notice(error.localizedDescription); return; }
 exporting=YES;
 UIDocumentPickerViewController* picker=[[UIDocumentPickerViewController alloc] initForExportingURLs:@[url] asCopy:YES]; picker.delegate=self;
 [host presentViewController:picker animated:YES completion:nil];
}
-(void)menu {
 paused=YES; memset(touchKeys,0,sizeof(touchKeys));
 UIAlertController* a=[UIAlertController alertControllerWithTitle:@"Files & iCloud Drive" message:ready ? @"Export includes the latest in-game save. To restore a backup or replace game data, reopen the app and use the startup screen." : @"Import your own DoomRPG.zip and gm.sf2 from Files or iCloud Drive. Saves stay on this device until you export them." preferredStyle:UIAlertControllerStyleAlert];
 if(!ready) {
  [a addAction:[UIAlertAction actionWithTitle:@"Play" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){[mobile start];}]];
  [a addAction:[UIAlertAction actionWithTitle:@"Import game data from Files / iCloud" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){[mobile pick:NO];}]];
  [a addAction:[UIAlertAction actionWithTitle:@"Import save backup" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){[mobile pick:YES];}]];
 }
 [a addAction:[UIAlertAction actionWithTitle:@"Export save to Files / iCloud" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){[mobile exportSaves];}]];
 if(ready)[a addAction:[UIAlertAction actionWithTitle:@"Resume" style:UIAlertActionStyleCancel handler:^(UIAlertAction* action){paused=NO;}]];
 [host presentViewController:a animated:YES completion:nil];
}
-(void)documentPickerWasCancelled:(UIDocumentPickerViewController*)controller { if(ready)paused=NO; else dispatch_async(dispatch_get_main_queue(), ^{ [mobile menu]; }); }
-(void)documentPicker:(UIDocumentPickerViewController*)controller didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls {
 if(exporting) { paused=NO; return; }
 NSError* error=nil;
 if(importingSaves) {
  NSURL* url=urls.firstObject; BOOL scoped=[url startAccessingSecurityScopedResource];
  NSNumber* size=nil; [url getResourceValue:&size forKey:NSURLFileSizeKey error:&error];
  NSData* data=size.unsignedLongLongValue<=8*1024*1024 ? [NSData dataWithContentsOfURL:url options:0 error:&error] : nil;
  if(scoped)[url stopAccessingSecurityScopedResource];
  NSDictionary* files=DRPGReadSave(data,&error);
  if(!files) { notice(error.localizedDescription ?: @"This is not a valid Doom RPG save backup."); return; }
  UIAlertController* a=[UIAlertController alertControllerWithTitle:@"Replace local saves?" message:@"A copy of the existing save files will be kept in Previous Saves in the app’s Files folder." preferredStyle:UIAlertControllerStyleAlert];
  [a addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
  [a addAction:[UIAlertAction actionWithTitle:@"Import" style:UIAlertActionStyleDestructive handler:^(UIAlertAction* action){
   NSError* e=nil;
   DRPGRestoreSave(files,folder,&e);
   chdir(folder.fileSystemRepresentation);
   notice(e ? e.localizedDescription : @"Save imported. Tap Files, then Play, and choose Continue in the game.");
  }]]; [host presentViewController:a animated:YES completion:nil];
 } else {
  NSUInteger copied=0;
  for(NSURL* url in urls) {
   NSString* n=url.lastPathComponent;
   if(![n.lowercaseString isEqual:@"doomrpg.zip"] && ![n.lowercaseString isEqual:@"gm.sf2"])continue;
   BOOL scoped=[url startAccessingSecurityScopedResource];
   NSNumber* size=nil; [url getResourceValue:&size forKey:NSURLFileSizeKey error:&error];
   NSData* data=size && size.unsignedLongLongValue<=64*1024*1024 ? [NSData dataWithContentsOfURL:url options:NSDataReadingMappedIfSafe error:&error] : nil;
   BOOL zip=[n.lowercaseString isEqual:@"doomrpg.zip"];
   BOOL valid=zip ? IOS_ValidateZip(data.bytes,data.length) : IOS_ValidateSoundfont(data.bytes,data.length);
   if(valid && [data writeToFile:[folder stringByAppendingPathComponent:zip ? @"DoomRPG.zip" : @"gm.sf2"] options:NSDataWritingAtomic error:&error])copied++;
   if(scoped)[url stopAccessingSecurityScopedResource];
  }
  notice(copied ? (assetsReady() ? @"Game data imported. Tap Files, then Play." : @"File imported. Import the other file before playing.") : @"Select DoomRPG.zip and/or a valid gm.sf2 soundfont in Files.");
 }
}
@end
static void button(UIView* view,NSString* title,int key,CGFloat x,CGFloat y,BOOL right,BOOL bottom) {
 UIButton* b=[UIButton buttonWithType:UIButtonTypeSystem]; b.translatesAutoresizingMaskIntoConstraints=NO;
 [b setTitle:title forState:UIControlStateNormal]; b.tintColor=UIColor.whiteColor;
 b.backgroundColor=[UIColor.blackColor colorWithAlphaComponent:.45]; b.layer.cornerRadius=10; b.tag=key;
 [view addSubview:b]; UILayoutGuide* g=view.safeAreaLayoutGuide;
 [NSLayoutConstraint activateConstraints:@[[b.widthAnchor constraintEqualToConstant:64],[b.heightAnchor constraintEqualToConstant:44],right ? [b.trailingAnchor constraintEqualToAnchor:g.trailingAnchor constant:-x] : [b.leadingAnchor constraintEqualToAnchor:g.leadingAnchor constant:x],bottom ? [b.bottomAnchor constraintEqualToAnchor:g.bottomAnchor constant:-y] : [b.topAnchor constraintEqualToAnchor:g.topAnchor constant:y]]];
 if(key) { [b addTarget:mobile action:@selector(down:) forControlEvents:UIControlEventTouchDown]; [b addTarget:mobile action:@selector(up:) forControlEvents:UIControlEventTouchUpInside|UIControlEventTouchUpOutside|UIControlEventTouchCancel]; }
 else [b addTarget:mobile action:@selector(menu) forControlEvents:UIControlEventTouchUpInside];
}
void IOS_SetupStorage(void) {
 SDL_SetHint(SDL_HINT_AUDIO_CATEGORY,"ambient");
 SDL_SetHint(SDL_HINT_ORIENTATIONS,"LandscapeLeft LandscapeRight");
 IOS_SetActive(1);
 NSString* docs=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;
 folder=[docs stringByAppendingPathComponent:@"Game"];
 [NSFileManager.defaultManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:nil];
 chdir(folder.fileSystemRepresentation);
}
void IOS_Prepare(void) {
 SDL_SysWMinfo info; SDL_VERSION(&info.version); SDL_GetWindowWMInfo(SDL_GetKeyboardFocus(),&info);
 host=info.info.uikit.window.rootViewController; mobile=[Mobile new];
 button(host.view,@"Files",0,8,4,YES,NO);
 button(host.view,@"Menu",SDL_SCANCODE_ESCAPE,8,4,NO,NO);
 button(host.view,@"↑",SDL_SCANCODE_UP,68,100,NO,YES);
 button(host.view,@"↓",SDL_SCANCODE_DOWN,68,4,NO,YES);
 button(host.view,@"←",SDL_SCANCODE_LEFT,8,52,NO,YES);
 button(host.view,@"→",SDL_SCANCODE_RIGHT,128,52,NO,YES);
 button(host.view,@"Use",SDL_SCANCODE_RETURN,8,52,YES,YES);
 button(host.view,@"Map",SDL_SCANCODE_TAB,8,100,YES,YES);
 button(host.view,@"Weapon",SDL_SCANCODE_Z,76,52,YES,YES);
 button(host.view,@"Wait",SDL_SCANCODE_C,8,4,YES,YES);
 button(host.view,@"Strafe L",SDL_SCANCODE_A,8,152,NO,YES);
 button(host.view,@"Strafe R",SDL_SCANCODE_D,128,152,NO,YES);
 SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
 pinPad=[UIView new]; pinPad.translatesAutoresizingMaskIntoConstraints=NO; pinPad.backgroundColor=[UIColor.blackColor colorWithAlphaComponent:.85]; pinPad.layer.cornerRadius=14; pinPad.hidden=YES; [host.view addSubview:pinPad]; UILayoutGuide* g=host.view.safeAreaLayoutGuide; [NSLayoutConstraint activateConstraints:@[[pinPad.centerXAnchor constraintEqualToAnchor:g.centerXAnchor],[pinPad.centerYAnchor constraintEqualToAnchor:g.centerYAnchor],[pinPad.widthAnchor constraintEqualToConstant:228],[pinPad.heightAnchor constraintEqualToConstant:216]]]; NSArray* labels=@[@"1",@"2",@"3",@"4",@"5",@"6",@"7",@"8",@"9",@"⌫",@"0",@"↵"]; for(int i=0;i<12;i++){ UIButton* b=[UIButton buttonWithType:UIButtonTypeSystem]; b.translatesAutoresizingMaskIntoConstraints=NO; b.tag=(i==9?10:(i==11?11:(i==10?0:i+1))); [b setTitle:labels[i] forState:UIControlStateNormal]; b.titleLabel.font=[UIFont boldSystemFontOfSize:24]; b.tintColor=UIColor.whiteColor; b.backgroundColor=[UIColor.whiteColor colorWithAlphaComponent:.16]; b.layer.cornerRadius=8; [pinPad addSubview:b]; int c=i%3,r=i/3; [NSLayoutConstraint activateConstraints:@[[b.widthAnchor constraintEqualToConstant:68],[b.heightAnchor constraintEqualToConstant:46],[b.leadingAnchor constraintEqualToAnchor:pinPad.leadingAnchor constant:6+c*74],[b.topAnchor constraintEqualToAnchor:pinPad.topAnchor constant:6+r*52]]]; [b addTarget:mobile action:@selector(pin:) forControlEvents:UIControlEventTouchUpInside]; }
 [mobile menu];
 while(!ready) { SDL_PumpEvents(); SDL_Delay(16); }
 paused=NO;
}
void IOS_Keyboard(unsigned char* keys) { for(int i=0;i<SDL_NUM_SCANCODES;i++)keys[i]|=touchKeys[i]; }
int IOS_Paused(void) { return paused || host.presentedViewController!=nil || UIApplication.sharedApplication.applicationState!=UIApplicationStateActive; }

void IOS_ResetTouch(void) { memset(touchKeys, 0, sizeof(touchKeys)); }
void IOS_RefreshLayout(void) {
 if (!host) return;
 UIWindow* window = host.view.window;
 UIWindowScene* scene = window.windowScene;
 if (!scene) return;
 /* UIScreen bounds can be portrait while a landscape scene is resuming. */
 window.frame = scene.coordinateSpace.bounds;
 host.view.frame = window.bounds;
 [host.view setNeedsLayout];
 [host.view layoutIfNeeded];
 SDL_Event event = {0};
 event.type = SDL_WINDOWEVENT;
 event.window.windowID = SDL_GetWindowID(SDL_GetKeyboardFocus());
 event.window.event = SDL_WINDOWEVENT_EXPOSED;
 SDL_PushEvent(&event);
}

void IOS_SetActive(int active) {
 AVAudioSession* session = AVAudioSession.sharedInstance;
 if (!active) { Mix_Pause(-1); Mix_PauseMusic(); }

 if (active) {
  [session setCategory:AVAudioSessionCategoryAmbient mode:AVAudioSessionModeDefault
               options:AVAudioSessionCategoryOptionMixWithOthers error:nil];
  [session setActive:YES error:nil];
 } else {
  [session setActive:NO withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error:nil];
 }
 if (active) { Mix_Resume(-1); Mix_ResumeMusic(); }
}

void IOS_ShowPinKeypad(int visible) { if(pinPad) pinPad.hidden=!visible; }
