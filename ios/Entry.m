#define SDL_MAIN_HANDLED
#import <UIKit/UIKit.h>
#include <SDL.h>
#import "SDL_uikitappdelegate.h"
extern int SDL_main(int,char**);
static BOOL sceneReady, started;
@interface SDLUIKitDelegate (Launch)
-(void)postFinishLaunch;
@end
@interface GameDelegate : SDLUIKitDelegate
@end
@implementation SDLUIKitDelegate (DoomRPGDelegate)
+(NSString*)getAppDelegateClassName { return @"GameDelegate"; }
@end
@implementation GameDelegate
-(void)postFinishLaunch {
 if(!sceneReady || started)return;
 started=YES; [super postFinishLaunch];
}
@end
@interface GameScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow* window;
@end
@implementation GameScene
-(void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
 sceneReady=YES;
 [(GameDelegate*)UIApplication.sharedApplication.delegate performSelector:@selector(postFinishLaunch) withObject:nil afterDelay:0];
}
@end
int main(int argc,char** argv) { return SDL_UIKitRunApp(argc,argv,SDL_main); }
