#define SDL_MAIN_HANDLED
#import <UIKit/UIKit.h>
#include <SDL.h>
#include "Platform.h"
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
-(void)sceneWillResignActive:(UIScene*)scene {
 IOS_ResetTouch();
 IOS_SetActive(0);
 SDL_OnApplicationWillResignActive();
}
-(void)sceneDidEnterBackground:(UIScene*)scene {
 SDL_OnApplicationDidEnterBackground();
}
-(void)sceneWillEnterForeground:(UIScene*)scene {
 SDL_OnApplicationWillEnterForeground();
}
-(void)sceneDidBecomeActive:(UIScene*)scene {
 IOS_ResetTouch();
 IOS_RefreshLayout();
 IOS_SetActive(1);
 SDL_OnApplicationDidBecomeActive();
}
-(void)windowScene:(UIWindowScene*)scene didUpdateCoordinateSpace:(id<UICoordinateSpace>)previousCoordinateSpace interfaceOrientation:(UIInterfaceOrientation)previousInterfaceOrientation traitCollection:(UITraitCollection*)previousTraitCollection {
 IOS_ResetTouch();
 IOS_RefreshLayout();
}
-(void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
 sceneReady=YES;
 [(GameDelegate*)UIApplication.sharedApplication.delegate performSelector:@selector(postFinishLaunch) withObject:nil afterDelay:0];
}
@end
int main(int argc,char** argv) { return SDL_UIKitRunApp(argc,argv,SDL_main); }
