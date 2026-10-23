#include <SDL.h>
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Hud.h"
#include "Game.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "SDL_Video.h"
#include "VideoOptions.h"
#include "ResolutionMenu.h"

boolean ResolutionMenu_isMenu(int menu)
{
    return menu == MENU_RESOLUTION || menu == MENU_INGAME_RESOLUTION;
}

static void layout(MenuSystem_t* menu, int* top, int* bottom)
{
    DoomCanvas_t* canvas = menu->doomRpg->doomCanvas;
    boolean inGame = menu->menu == MENU_INGAME_RESOLUTION;
    *top = inGame ? canvas->hud->statusTopBarHeight : 0;
    *bottom = canvas->displayRect.h - (inGame ? canvas->hud->statusBarHeight : 0);
    menu->maxItems = SDL_max(1, (*bottom - *top - 48) / 12);
    menu->scrollIndex = SDL_min(menu->scrollIndex, SDL_max(0, menu->numItems - menu->maxItems));
    if (menu->selectedIndex < menu->scrollIndex) menu->scrollIndex = menu->selectedIndex;
    if (menu->selectedIndex >= menu->scrollIndex + menu->maxItems)
        menu->scrollIndex = menu->selectedIndex - menu->maxItems + 1;
}

void ResolutionMenu_init(MenuSystem_t* menu)
{
    menu->oldMenu = menu->menu == MENU_RESOLUTION ? MENU_VIDEO : MENU_INGAME_VIDEO;
    menu->type = menu->menu == MENU_RESOLUTION ? 7 : 1;
    menu->numItems = 0;
    menu->imgBG = NULL;
    menu->setBind = false;
    MenuItem_Set(&menu->items[menu->numItems++], "Back", 0, -1);
    for (int mode = 0; mode < SDL_VIDEO_MODE_COUNT; ++mode) {
        char label[32];
        SDL_snprintf(label, sizeof(label), "%c %d x %d", mode == sdlVideo.resolutionIndex ? '*' : ' ', sdlVideoModes[mode].width, sdlVideoModes[mode].height);
        MenuItem_Set(&menu->items[menu->numItems++], label, 0, mode);
    }
    menu->selectedIndex = sdlVideo.resolutionIndex + 1;
    menu->scrollIndex = menu->selectedIndex;
    int top, bottom;
    layout(menu, &top, &bottom);
    menu->scrollIndex = SDL_max(0, menu->selectedIndex - menu->maxItems / 2);
    layout(menu, &top, &bottom);
    menu->paintMenu = true;
}

void ResolutionMenu_paint(MenuSystem_t* menu)
{
    DoomCanvas_t* canvas = menu->doomRpg->doomCanvas;
    int top, bottom, width = canvas->displayRect.w;
    layout(menu, &top, &bottom);
    DoomRPG_setClipFalse(menu->doomRpg);
    DoomRPG_setColor(menu->doomRpg, 0x101828);
    DoomRPG_fillRect(menu->doomRpg, 0, top, width, bottom - top);
    DoomRPG_setFontColor(menu->doomRpg, 0xffffffff);
    DoomCanvas_drawString1(canvas, "Resolution", width / 2, top + 2, 16);
    DoomCanvas_drawString1(canvas, width < 240 ? "A/Enter: apply" : "A/Enter: apply  B/Esc: back", width / 2, top + 16, 16);
    for (int row = menu->scrollIndex; row < menu->numItems && row < menu->scrollIndex + menu->maxItems; ++row) {
        int y = top + 34 + (row - menu->scrollIndex) * 12;
        if (row == menu->selectedIndex) {
            DoomRPG_setColor(menu->doomRpg, 0x304868);
            DoomRPG_fillRect(menu->doomRpg, 8, y, width - 16, 12);
        }
        DoomRPG_setFontColor(menu->doomRpg, row == menu->selectedIndex ? 0xffffff80 : 0xffffffff);
        DoomCanvas_drawString1(canvas, menu->items[row].textField, 14, y, 0);
    }
    DoomRPG_setFontColor(menu->doomRpg, 0xff80c0ff);
    DoomCanvas_drawString1(canvas, "* Current", width - 12, bottom - 12, 8);
}

int ResolutionMenu_select(MenuSystem_t* menu, int item)
{
    if (item == 0) return menu->oldMenu;
    if (item < 1 || item >= menu->numItems) return menu->menu;
    if (!VideoOptions_resolution(menu->doomRpg, menu->items[item].action)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Video settings", SDL_GetError(), sdlVideo.window);
        return menu->menu;
    }
    Game_saveConfig(menu->doomRpg->game, 0);
    return menu->oldMenu;
}
