#include <SDL.h>
#include <stdio.h>
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Game.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "SDL_Video.h"
#include "Controls.h"

static const char* actions[12] = {
    "Move Forward", "Move Backward", "Turn Left", "Turn Right",
    "Move Left", "Move Right", "Next Weapon", "Previous Weapon",
    "Attack/Talk/Use", "Pass Turn", "Automap", "Open Menu/Back"
};

boolean Controls_isMenu(int menu)
{
    return menu == MENU_BINDINGS || menu == MENU_INGAME_BINDINGS ||
           menu == MENU_MAIN_HELP_ABOUT || menu == MENU_INGAME_HELP_ABOUT;
}

static void saveBindings(MenuSystem_t* menu)
{
    SDL_memcpy(keyMapping, keyMappingTemp, sizeof(keyMapping));
    Game_saveConfig(menu->doomRpg->game, 0);
}

void Controls_init(MenuSystem_t* menu, int parent)
{
    menu->oldMenu = parent;
    menu->type = parent >= MENU_INGAME_BEGIN || parent == MENU_NONE ? 1 : 7;
    menu->imgBG = NULL;
    menu->numItems = 0;
    menu->selectedIndex = 0;
    menu->scrollIndex = 0;
    menu->setBind = false;
    SDL_memcpy(keyMappingTemp, keyMapping, sizeof(keyMapping));
    MenuItem_Set(&menu->items[menu->numItems++], "Save & Back", 0, -1);
    for (int action = 0; action < 12; ++action)
        MenuItem_Set(&menu->items[menu->numItems++], (char*)actions[action], 0, action);
    MenuItem_Set(&menu->items[menu->numItems++], "Restore Defaults", 0, -1);
    menu->paintMenu = true;
}

void Controls_open(DoomRPG_t* game)
{
    MenuSystem_t* menu = game->menuSystem;
    if (game->doomCanvas->state != ST_MENU && game->doomCanvas->state != ST_PLAYING) return;
    if (Controls_isMenu(menu->menu)) return;
    int parent = menu->menu;
    boolean inGame = parent >= MENU_INGAME_BEGIN || game->doomCanvas->state != ST_MENU;
    game->doomCanvas->numEvents = 0;
    MenuSystem_setMenu(menu, inGame ? MENU_INGAME_BINDINGS : MENU_BINDINGS);
    menu->oldMenu = parent;
    menu->inputWaitRelease = true;
}

int Controls_gamepadMenuKey(int button)
{
    switch (button) {
    case CONTROLLER_BUTTON_A: return AVK_MENU_SELECT;
    case CONTROLLER_BUTTON_B:
    case CONTROLLER_BUTTON_BACK:
    case CONTROLLER_BUTTON_START: return AVK_MENU_OPEN;
    case CONTROLLER_BUTTON_DPAD_UP:
    case CONTROLLER_BUTTON_LAXIS_UP: return AVK_MENU_UP;
    case CONTROLLER_BUTTON_DPAD_DOWN:
    case CONTROLLER_BUTTON_LAXIS_DOWN: return AVK_MENU_DOWN;
    case CONTROLLER_BUTTON_DPAD_LEFT:
    case CONTROLLER_BUTTON_LAXIS_LEFT: return AVK_MENU_PAGE_UP;
    case CONTROLLER_BUTTON_DPAD_RIGHT:
    case CONTROLLER_BUTTON_LAXIS_RIGHT: return AVK_MENU_PAGE_DOWN;
    default: return 0;
    }
}

boolean Controls_inputReleased(const Uint8* state)
{
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
        if (state[i]) return false;
    return SDL_GetMouseState(NULL, NULL) == 0 &&
           DoomRPG_controllerButton() == CONTROLLER_BUTTON_INVALID;
}

static int device(int binding)
{
    return binding & (IS_CONTROLLER_BUTTON | IS_MOUSE_BUTTON);
}

boolean Controls_assign(int action, int binding)
{
    if (action < 0 || action >= 12 || binding < 0) return false;
    int kind = device(binding);
    if ((!kind && binding >= SDL_NUM_SCANCODES) ||
        (kind == IS_CONTROLLER_BUTTON && (binding & ~kind) >= CONTROLLER_BUTTON_MAX) ||
        (kind == IS_MOUSE_BUTTON && (binding & ~kind) >= MOUSE_BUTTON_MAX)) return false;
    /* Replace this device's binding and remove conflicts in other actions.
       Keyboard and gamepad assignments remain independent. */
    for (int row = 0; row < 12; ++row) {
        int next = 0;
        for (int slot = 0; slot < KEYBINDS_MAX; ++slot) {
            int old = keyMappingTemp[row].keyBinds[slot];
            if (old < 0 || old == binding || (row == action && device(old) == kind)) continue;
            keyMappingTemp[row].keyBinds[next++] = old;
        }
        if (row == action) {
            next = SDL_min(next, KEYBINDS_MAX - 1);
            keyMappingTemp[row].keyBinds[next++] = binding;
        }
        while (next < KEYBINDS_MAX) keyMappingTemp[row].keyBinds[next++] = -1;
    }
    return true;
}

void Controls_capture(MenuSystem_t* menu, int mouse, const Uint8* state)
{
    if (menu->inputWaitRelease) {
        if (Controls_inputReleased(state)) menu->inputWaitRelease = false;
        return;
    }
    int pad = DoomRPG_controllerButton();
    if (state[SDL_SCANCODE_ESCAPE] || pad == CONTROLLER_BUTTON_START || pad == CONTROLLER_BUTTON_BACK) {
        menu->setBind = false;
        menu->inputWaitRelease = true;
        return;
    }
    int binding = -1;
    for (int i = 1; i < SDL_NUM_SCANCODES; ++i)
        if (state[i] && i != SDL_SCANCODE_F1) { binding = i; break; }
    if (binding < 0 && mouse != MOUSE_BUTTON_INVALID) binding = mouse | IS_MOUSE_BUTTON;
    if (binding < 0 && pad != CONTROLLER_BUTTON_INVALID) binding = pad | IS_CONTROLLER_BUTTON;
    if (binding >= 0 && Controls_assign(menu->items[menu->bindIndx].action, binding)) {
        saveBindings(menu);
        menu->setBind = false;
        menu->inputWaitRelease = true;
        menu->paintMenu = true;
    }
}

int Controls_select(MenuSystem_t* menu, int item)
{
    if (item == 0) {
        saveBindings(menu);
        return menu->oldMenu;
    }
    if (item == menu->numItems - 1) {
        SDL_memcpy(keyMappingTemp, keyMappingDefault, sizeof(keyMapping));
        saveBindings(menu);
    } else if (item > 0 && item < menu->numItems - 1) {
        menu->setBind = true;
        menu->bindIndx = item;
        menu->inputWaitRelease = true;
    }
    return menu->menu;
}

void Controls_formatBindings(int action, char* text, size_t size)
{
    text[0] = '\0';
    for (int slot = 0; slot < KEYBINDS_MAX; ++slot) {
        int key = keyMappingTemp[action].keyBinds[slot];
        if (key < 0) continue;
        const char* name;
        char padName[48];
        if (key & IS_CONTROLLER_BUTTON) {
            const char* button = SDL_GameControllerGetNameButton(key & ~IS_CONTROLLER_BUTTON);
            if (SDL_strncmp(button, "Gamepad ", 8) == 0) button += 8;
            SDL_snprintf(padName, sizeof(padName), "Pad %s", button);
            name = padName;
        } else if (key & IS_MOUSE_BUTTON) {
            name = SDL_MouseGetNameButton(key & ~IS_MOUSE_BUTTON);
        } else if (key < SDL_NUM_SCANCODES) {
            name = SDL_GetScancodeName(key);
        } else continue;
        if (text[0]) SDL_strlcat(text, " / ", size);
        SDL_strlcat(text, name, size);
    }
    if (!text[0]) SDL_strlcpy(text, "Unbound", size);
}

void Controls_paint(MenuSystem_t* menu)
{
    DoomCanvas_t* canvas = menu->doomRpg->doomCanvas;
    int width = canvas->displayRect.w;
    int height = canvas->displayRect.h;
    int columns = SDL_max(1, (width - 28) / 7);
    menu->maxItems = SDL_max(1, (height - 64) / 28);
    if (menu->selectedIndex < menu->scrollIndex) menu->scrollIndex = menu->selectedIndex;
    if (menu->selectedIndex >= menu->scrollIndex + menu->maxItems)
        menu->scrollIndex = menu->selectedIndex - menu->maxItems + 1;
    DoomRPG_setClipFalse(menu->doomRpg);
    DoomRPG_setColor(menu->doomRpg, 0x101828);
    DoomRPG_fillRect(menu->doomRpg, 0, 0, width, height);
    DoomRPG_setFontColor(menu->doomRpg, 0xffffffff);
    DoomCanvas_drawString1(canvas, "Controls / Help", width / 2, 3, 16);
    DoomCanvas_drawString1(canvas, "D-pad/Arrows move", width / 2, 17, 16);
    for (int index = menu->scrollIndex; index < menu->numItems && index < menu->scrollIndex + menu->maxItems; ++index) {
        int y = 34 + (index - menu->scrollIndex) * 28;
        boolean selected = index == menu->selectedIndex;
        if (selected) {
            DoomRPG_setColor(menu->doomRpg, 0x304868);
            DoomRPG_fillRect(menu->doomRpg, 3, y - 1, width - 6, 27);
        }
        DoomRPG_setFontColor(menu->doomRpg, selected ? 0xffffff80 : 0xffffffff);
        DoomCanvas_drawFont(canvas, menu->items[index].textField, 10, y, 0, 0, columns, false);
        if (index > 0 && index < menu->numItems - 1) {
            char bindings[512];
            Controls_formatBindings(menu->items[index].action, bindings, sizeof(bindings));
            int length = (int)SDL_strlen(bindings);
            int start = selected && length > columns ? (SDL_GetTicks() / 500) % (length - columns + 1) : 0;
            DoomRPG_setFontColor(menu->doomRpg, 0xff80c0ff);
            DoomCanvas_drawFont(canvas, bindings, 17, y + 12, 0, start, columns, false);
        }
    }
    DoomRPG_setFontColor(menu->doomRpg, 0xffffffff);
    DoomCanvas_drawString1(canvas, "A/Enter: edit", width / 2, height - 27, 16);
    DoomCanvas_drawString1(canvas, "B/Esc: save/back", width / 2, height - 14, 16);
    if (menu->setBind) {
        DoomRPG_setColor(menu->doomRpg, 0x101828);
        DoomRPG_fillRect(menu->doomRpg, 0, 0, width, height);
        DoomCanvas_drawString1(canvas, "Change binding", width / 2, height / 2 - 36, 16);
        DoomRPG_setFontColor(menu->doomRpg, 0xffffff80);
        DoomCanvas_drawFont(canvas, menu->items[menu->bindIndx].textField, width / 2, height / 2 - 17, 16, 0, -1, false);
        DoomRPG_setFontColor(menu->doomRpg, 0xffffffff);
        DoomCanvas_drawString1(canvas, "Press key/button", width / 2, height / 2 + 3, 16);
        DoomCanvas_drawString1(canvas, "Esc/Start: cancel", width / 2, height / 2 + 20, 16);
    }
}
