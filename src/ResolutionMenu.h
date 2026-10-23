#ifndef DOOMRPG_RESOLUTION_MENU_H
#define DOOMRPG_RESOLUTION_MENU_H
struct MenuSystem_s;
boolean ResolutionMenu_isMenu(int menu);
void ResolutionMenu_init(struct MenuSystem_s* menu);
void ResolutionMenu_paint(struct MenuSystem_s* menu);
int ResolutionMenu_select(struct MenuSystem_s* menu, int item);
#endif
