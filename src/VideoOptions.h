#ifndef DOOMRPG_VIDEO_OPTIONS_H
#define DOOMRPG_VIDEO_OPTIONS_H
struct DoomRPG_s;
boolean VideoOptions_resolution(struct DoomRPG_s* game, int index);
boolean VideoOptions_fullscreen(boolean enabled);
#endif
