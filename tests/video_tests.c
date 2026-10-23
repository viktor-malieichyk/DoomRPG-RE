#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Render.h"
#include "Player.h"
#include "Game.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "TextFields.h"
#include "VideoOptions.h"
#include "../src/SDL_Video.h"
extern DoomRPG_t* doomRpg;

void test_video(SDL_Surface* surface)
{
    /* The preceding navigation test returned from an artificial game menu. */
    MenuSystem_setMenu(doomRpg->menuSystem, MENU_MAIN);
    DoomCanvas_t* canvas = doomRpg->doomCanvas;
    Render_t* render = doomRpg->render;
    int initial = sdlVideo.resolutionIndex;
    /* A persisted test config can differ from the active rendering fixture. */
    for (int i = 0; i < SDL_VIDEO_MODE_COUNT; ++i)
        if (sdlVideoModes[i].width == sdlVideo.rendererW && sdlVideoModes[i].height == sdlVideo.rendererH) initial = i;
    boolean compactBefore = sdlVideo.compactText;
    char message[] = "These messages use|small text in the|same dialogue box.|More words fit on|each line without|losing paragraph|breaks. This panel|should now show a|lot more of the|message at once.|Use the gamepad to|continue reading.";
    canvas->state = ST_DIALOG;
    sdlVideo.compactText = false;
    DoomCanvas_prepareDialog(canvas, message, false);
    int normalLines = canvas->numDialogLines;
    assert(TextFields_dialogRows() == 4);
    canvas->dialogTypeLineIdx = TextFields_dialogRows();
    DoomCanvas_dialogState(canvas);
    char path[128];
    snprintf(path, sizeof(path), "source/tests/text-normal-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    sdlVideo.compactText = true;
    TextFields_refresh(canvas);
    if (sdlVideo.rendererH > sdlVideo.uiH) {
        assert(canvas->numDialogLines < normalLines);
        assert(TextFields_dialogRows() > 4);
    } else assert(TextFields_dialogRows() == 4);
    canvas->dialogTypeLineIdx = TextFields_dialogRows();
    DoomCanvas_dialogState(canvas);
    snprintf(path, sizeof(path), "source/tests/text-compact-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    Game_saveConfig(doomRpg->game, 0);
    sdlVideo.compactText = false;
    Game_loadConfig(doomRpg->game);
    assert(sdlVideo.compactText);
    sdlVideo.compactText = false;
    Game_loadConfig(NULL);
    assert(sdlVideo.compactText);
    /* Every source character must remain represented in ordered line spans. */
    for (int row = 0; row < canvas->numDialogLines; ++row) {
        assert(canvas->dialogIndexes[2 * row] >= 0);
        assert(canvas->dialogIndexes[2 * row + 1] <= (int)(126 / (7 * TextFields_scale())));
    }
    DoomCanvas_closeDialog(canvas);
    int map = render->loadMapID, x = canvas->viewX, y = canvas->viewY;
    int health = doomRpg->player->ce.param1;
    void* sprites = render->mapSprites;
    int modes[] = {17, 8, 0, 19, 15, initial};
    for (int i = 0; i < (int)SDL_arraysize(modes); ++i) {
        assert(VideoOptions_resolution(doomRpg, modes[i]));
        int width, height;
        SDL_QueryTexture(render->piDIB, NULL, NULL, &width, &height);
        assert(width == sdlVideoModes[modes[i]].width && height == sdlVideoModes[modes[i]].height);
        assert(render->screenX + render->screenWidth <= width);
        assert(render->screenY + render->screenHeight <= height);
        assert(render->loadMapID == map && render->mapSprites == sprites);
        assert(canvas->viewX == x && canvas->viewY == y && doomRpg->player->ce.param1 == health);
        Render_render(render, canvas->viewX, canvas->viewY, canvas->viewZ, canvas->viewAngle);
        DoomCanvas_drawRGB(canvas);
    }
    /* A wrap from 4K to the smallest mode must keep the selected row visible. */
    MenuSystem_setMenu(doomRpg->menuSystem, MENU_VIDEO);
    doomRpg->menuSystem->selectedIndex = 5;
    assert(VideoOptions_resolution(doomRpg, 19));
    Menu_select(doomRpg->menu, MENU_VIDEO, 5);
    assert(sdlVideo.resolutionIndex == 0);
    MenuSystem_paint(doomRpg->menuSystem);
    assert(doomRpg->menuSystem->selectedIndex >= doomRpg->menuSystem->scrollIndex);
    assert(doomRpg->menuSystem->selectedIndex < doomRpg->menuSystem->scrollIndex + doomRpg->menuSystem->maxItems);
    assert(VideoOptions_resolution(doomRpg, initial));
    /* Menu actions apply the change during selection, including the wrap. */
    MenuSystem_setMenu(doomRpg->menuSystem, MENU_INGAME_VIDEO);
    int next = (sdlVideo.resolutionIndex + 1) % SDL_VIDEO_MODE_COUNT;
    Menu_select(doomRpg->menu, MENU_INGAME_VIDEO, 5);
    assert(sdlVideo.resolutionIndex == next);
    assert(sdlVideo.rendererW == sdlVideoModes[next].width);
    assert(VideoOptions_resolution(doomRpg, initial));
    MenuSystem_setMenu(doomRpg->menuSystem, MENU_INGAME_OPTIONS);
    assert(strcmp(doomRpg->menuSystem->items[4].textField, "Compact text:") == 0);
    boolean before = sdlVideo.compactText;
    Menu_select(doomRpg->menu, MENU_INGAME_OPTIONS, 4);
    assert(sdlVideo.compactText != before);
    MenuSystem_paint(doomRpg->menuSystem);
    snprintf(path, sizeof(path), "source/tests/options-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    sdlVideo.compactText = compactBefore;
    Game_saveConfig(doomRpg->game, 0);
    puts("PASS video: live framebuffer/layout changes through 4K, state preserved, compact text reflow and persistence, menu actions");
}
