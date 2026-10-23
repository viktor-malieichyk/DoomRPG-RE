#ifndef DOOMRPG_CONTROLS_H
#define DOOMRPG_CONTROLS_H

struct DoomRPG_s;
struct MenuSystem_s;
boolean Controls_isMenu(int menu);
void Controls_init(struct MenuSystem_s* menu, int parent);
void Controls_open(struct DoomRPG_s* game);
void Controls_paint(struct MenuSystem_s* menu);
int Controls_select(struct MenuSystem_s* menu, int item);
int Controls_gamepadMenuKey(int button);
boolean Controls_inputReleased(const Uint8* state);
void Controls_capture(struct MenuSystem_s* menu, int mouse, const Uint8* state);
boolean Controls_assign(int action, int binding);
void Controls_formatBindings(int action, char* text, size_t size);
#endif
