#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "Game.h"
#include "Controls.h"
#include "../src/SDL_Video.h"
extern DoomRPG_t* doomRpg;

static void padButton(SDL_Joystick* pad, int button, int value)
{
    assert(SDL_JoystickSetVirtualButton(pad, button, value) == 0);
    SDL_JoystickUpdate();
    SDL_GameControllerUpdate();
}

static void pollFrame(const Uint8* keys)
{
    DoomRPG_pollInput(doomRpg, MOUSE_BUTTON_INVALID, keys);
    DoomCanvas_runInputEvents(doomRpg->doomCanvas);
}

static void testHeldMenuInput(SDL_Joystick* pad)
{
    Uint8 keys[SDL_NUM_SCANCODES] = {0};
    MenuSystem_t* menu = doomRpg->menuSystem;
    DoomCanvas_t* canvas = doomRpg->doomCanvas;
    MenuSystem_setMenu(menu, MENU_NONE);
    boolean renderOnly = canvas->renderOnly;
    canvas->renderOnly = true; /* Isolate input from turn/door animation. */
    pollFrame(keys);
    keys[SDL_SCANCODE_ESCAPE] = 1;
    pollFrame(keys);
    assert(menu->menu == MENU_INGAME);
    for (int frame = 0; frame < 60; ++frame) { pollFrame(keys); assert(menu->menu == MENU_INGAME); }
    keys[SDL_SCANCODE_ESCAPE] = 0;
    pollFrame(keys);
    keys[SDL_SCANCODE_ESCAPE] = 1;
    pollFrame(keys);
    assert(menu->menu == MENU_NONE);
    for (int frame = 0; frame < 60; ++frame) { pollFrame(keys); assert(menu->menu == MENU_NONE); }
    keys[SDL_SCANCODE_ESCAPE] = 0;
    pollFrame(keys);
    padButton(pad, SDL_CONTROLLER_BUTTON_START, 1);
    pollFrame(keys);
    assert(menu->menu == MENU_INGAME);
    for (int frame = 0; frame < 60; ++frame) { pollFrame(keys); assert(menu->menu == MENU_INGAME); }
    padButton(pad, SDL_CONTROLLER_BUTTON_START, 0);
    pollFrame(keys);
    padButton(pad, SDL_CONTROLLER_BUTTON_START, 1);
    pollFrame(keys);
    assert(menu->menu == MENU_NONE);
    for (int frame = 0; frame < 60; ++frame) { pollFrame(keys); assert(menu->menu == MENU_NONE); }
    padButton(pad, SDL_CONTROLLER_BUTTON_START, 0);
    pollFrame(keys);
    MenuSystem_setMenu(menu, MENU_MAIN);
    pollFrame(keys);
    menu->selectedIndex = 1;
    keys[SDL_SCANCODE_RETURN] = 1;
    pollFrame(keys);
    assert(menu->menu == MENU_MAIN_OPTIONS);
    for (int frame = 0; frame < 60; ++frame) { pollFrame(keys); assert(menu->menu == MENU_MAIN_OPTIONS); }
    keys[SDL_SCANCODE_RETURN] = 0;
    pollFrame(keys);
    keys[SDL_SCANCODE_RETURN] = 1;
    pollFrame(keys);
    assert(menu->menu == MENU_MAIN);
    for (int frame = 0; frame < 60; ++frame) { pollFrame(keys); assert(menu->menu == MENU_MAIN); }
    keys[SDL_SCANCODE_RETURN] = 0;
    pollFrame(keys);
    canvas->renderOnly = renderOnly;
    MenuSystem_setMenu(menu, MENU_MAIN_HELP_ABOUT);
    pollFrame(keys);
    puts("PASS held input: Escape/Start open once, close on next press; Enter cannot activate the next menu while held");
}

void test_controls(SDL_Surface* surface)
{
    Uint8 keys[SDL_NUM_SCANCODES] = {0};
    keyMapping_t original[12];
    SDL_memcpy(original, keyMapping, sizeof(original));
    MenuSystem_t* menu = doomRpg->menuSystem;
    MenuSystem_setMenu(menu, MENU_MAIN_HELP_ABOUT);
    assert(Controls_isMenu(menu->menu));
    assert(menu->numItems == 14);
    assert(menu->oldMenu == MENU_MAIN);
    assert(strcmp(menu->items[1].textField, "Move Forward") == 0);
    Controls_paint(menu);
    char path[120];
    snprintf(path, sizeof(path), "source/tests/controls-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    assert(!SDL_GetRelativeMouseMode());
    assert(strcmp(SDL_GetHint(SDL_HINT_MOUSE_AUTO_CAPTURE) ? SDL_GetHint(SDL_HINT_MOUSE_AUTO_CAPTURE) : "0", "0") == 0);

    /* Use an actual SDL virtual gamepad to exercise polling and navigation. */
    int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 6, SDL_CONTROLLER_BUTTON_MAX, 0);
    assert(index >= 0);
    SDL_Joystick* pad = SDL_JoystickOpen(index);
    assert(pad);
    char guid[40], mapping[512];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(pad), guid, sizeof(guid));
    snprintf(mapping, sizeof(mapping), "%s,Agent UI Test,a:b0,b:b1,x:b2,y:b3,back:b4,start:b6,leftshoulder:b9,rightshoulder:b10,dpup:b11,dpdown:b12,dpleft:b13,dpright:b14,leftx:a0,lefty:a1,lefttrigger:a4,righttrigger:a5,", guid);
    assert(SDL_GameControllerAddMapping(mapping) >= 0);
    assert(SDL_JoystickSetVirtualAxis(pad, 4, -32768) == 0);
    assert(SDL_JoystickSetVirtualAxis(pad, 5, -32768) == 0);
    SDL_JoystickUpdate();
    SDL_RefreshController();
    assert(sdlController.gGameController);
    sdlController.deadZoneLeft = sdlController.deadZoneRight = 25;
    assert(DoomRPG_controllerButton() == CONTROLLER_BUTTON_INVALID);
    testHeldMenuInput(pad);
    padButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN, 1);
    int event = DoomRPG_getEventKey(MOUSE_BUTTON_INVALID, keys);
    assert(event == AVK_MENU_DOWN);
    DoomCanvas_handleMenuEvents(doomRpg->doomCanvas, event);
    assert(menu->selectedIndex == 1);
    padButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN, 0);
    padButton(pad, SDL_CONTROLLER_BUTTON_A, 1);
    assert(DoomRPG_getEventKey(MOUSE_BUTTON_INVALID, keys) == AVK_MENU_SELECT);
    DoomCanvas_handleMenuEvents(doomRpg->doomCanvas, AVK_MENU_SELECT);
    assert(menu->setBind && menu->inputWaitRelease);
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    assert(menu->setBind); /* A used to open the editor must not bind itself. */
    padButton(pad, SDL_CONTROLLER_BUTTON_A, 0);
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    assert(!menu->inputWaitRelease);
    padButton(pad, SDL_CONTROLLER_BUTTON_Y, 1);
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    assert(!menu->setBind && menu->inputWaitRelease);
    char bindings[512];
    Controls_formatBindings(0, bindings, sizeof(bindings));
    assert(strstr(bindings, "Pad Y"));
    padButton(pad, SDL_CONTROLLER_BUTTON_Y, 0);
    assert(Controls_inputReleased(keys));
    menu->inputWaitRelease = false;

    /* Keyboard rebinding preserves the pad assignment and survives reload. */
    Controls_select(menu, 1);
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    keys[SDL_SCANCODE_F] = 1;
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    assert(!menu->setBind);
    keys[SDL_SCANCODE_F] = 0;
    Game_loadConfig(doomRpg->game);
    Controls_formatBindings(0, bindings, sizeof(bindings));
    assert(strstr(bindings, "F") && strstr(bindings, "Pad Y"));
    keys[SDL_SCANCODE_F] = 1;
    assert(DoomRPG_getEventKey(MOUSE_BUTTON_INVALID, keys) & AVK_MENU_UP);
    keys[SDL_SCANCODE_F] = 0;

    /* Cancel leaves the map intact, including when using a gamepad. */
    menu->inputWaitRelease = false;
    Controls_select(menu, 2);
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    padButton(pad, SDL_CONTROLLER_BUTTON_START, 1);
    Controls_capture(menu, MOUSE_BUTTON_INVALID, keys);
    assert(!menu->setBind);
    padButton(pad, SDL_CONTROLLER_BUTTON_START, 0);
    padButton(pad, SDL_CONTROLLER_BUTTON_B, 1);
    assert(DoomRPG_getEventKey(MOUSE_BUTTON_INVALID, keys) == AVK_MENU_OPEN);
    padButton(pad, SDL_CONTROLLER_BUTTON_B, 0);
    menu->inputWaitRelease = false;
    menu->selectedIndex = 1;
    Controls_paint(menu);
    for (int i = 0; i < 12; ++i) MenuSystem_scrollDown(menu);
    assert(menu->selectedIndex == 13);
    assert(menu->scrollIndex > 0);
    Controls_select(menu, 13);
    assert(memcmp(keyMapping, keyMappingDefault, sizeof(keyMapping)) == 0);
    Controls_paint(menu);
    snprintf(path, sizeof(path), "source/tests/controls-bottom-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);

    SDL_memcpy(keyMapping, original, sizeof(original));
    SDL_memcpy(keyMappingTemp, original, sizeof(original));
    Game_saveConfig(doomRpg->game, 0);
    SDL_GameControllerClose(sdlController.gGameController);
    sdlController.gGameController = NULL;
    SDL_JoystickClose(pad);
    assert(SDL_JoystickDetachVirtual(index) == 0);
    MenuSystem_setMenu(menu, MENU_INGAME_HELP_ABOUT);
    assert(menu->oldMenu == MENU_INGAME && Controls_isMenu(menu->menu));
    MenuSystem_setMenu(menu, MENU_NONE);
    Controls_open(doomRpg);
    assert(menu->menu == MENU_INGAME_BINDINGS && menu->oldMenu == MENU_NONE);
    MenuSystem_back(menu);
    assert(menu->menu == MENU_NONE && doomRpg->doomCanvas->state == ST_PLAYING);
    /* Numeric keys must be assignable rather than intercepted as cheats. */
    assert(Controls_assign(0, SDL_SCANCODE_1));
    SDL_memcpy(keyMapping, keyMappingTemp, sizeof(keyMapping));
    keys[SDL_SCANCODE_1] = 1;
    assert(DoomRPG_getEventKey(MOUSE_BUTTON_INVALID, keys) == keyMapping[0].avk_action);
    keys[SDL_SCANCODE_1] = 0;
    SDL_memcpy(keyMapping, original, sizeof(original));
    SDL_memcpy(keyMappingTemp, original, sizeof(original));
    Game_saveConfig(doomRpg->game, 0);
    puts("PASS controls: virtual gamepad navigation/capture/cancel, keyboard rebind, persistence, defaults, scrolling");
}
