#ifndef DOOMRPG_VIDEO_OPTIONS_H
#define DOOMRPG_VIDEO_OPTIONS_H
struct DoomRPG_s;
boolean VideoOptions_resolution(struct DoomRPG_s* game, int index);
boolean VideoOptions_toggleHud(struct DoomRPG_s* game);
boolean VideoOptions_fullscreen(boolean enabled);
#ifdef DOOMRPG_IOS
boolean VideoOptions_mobileLayout(struct DoomRPG_s* game);
#endif
#endif
