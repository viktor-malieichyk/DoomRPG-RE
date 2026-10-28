#include <SDL.h>
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Render.h"
#include "Hud.h"
#include "MenuSystem.h"
#include "ParticleSystem.h"
#include "SDL_Video.h"
#include "TextFields.h"
#include "VideoOptions.h"

boolean VideoOptions_fullscreen(boolean enabled)
{
    if (SDL_SetWindowFullscreen(sdlVideo.window, enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) < 0) return false;
    sdlVideo.fullScreen = enabled;
    SDL_SetWindowGrab(sdlVideo.window, SDL_FALSE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(enabled ? SDL_DISABLE : SDL_ENABLE);
    return true;
}

static boolean resizeFramebuffer(DoomRPG_t* game, int index, int width, int height)
{
    if (index < 0 || index >= SDL_VIDEO_MODE_COUNT) return false;
    Render_t* render = game->render;
    DoomCanvas_t* canvas = game->doomCanvas;
    int pitch = (width * 2 + 3) & ~3;
    SDL_Texture* texture = SDL_CreateTexture(sdlVideo.renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) return false;
    byte* pixels = SDL_calloc(1, pitch * height);
    if (!pixels) { SDL_DestroyTexture(texture); SDL_OutOfMemory(); return false; }
    if (SDL_RenderSetLogicalSize(sdlVideo.renderer, width, height) < 0) {
        SDL_DestroyTexture(texture); SDL_free(pixels); return false;
    }
    boolean fullView = render->uiViewport.h == canvas->displayRect.h && render->uiViewport.w == canvas->displayRect.w;
    short floor = render->floorColor ? render->floorColor[0] : 0;
    short ceiling = render->ceilingColor ? render->ceilingColor[0] : 0;
    SDL_DestroyTexture(render->piDIB);
    SDL_free(render->framebuffer);
    render->piDIB = texture;
    render->framebuffer = pixels;
    render->pitch = pitch;
    sdlVideo.resolutionIndex = index;
    sdlVideo.rendererW = width;
    sdlVideo.rendererH = height;
    SDL_ConfigureUISize();
    SDL_RenderSetIntegerScale(sdlVideo.renderer, sdlVideo.integerScaling);
    #ifndef DOOMRPG_IOS
    if (sdlVideo.window) {
        SDL_SetWindowMinimumSize(sdlVideo.window, 128, 128);
        SDL_SetWindowSize(sdlVideo.window, width, height);
        SDL_SetWindowMinimumSize(sdlVideo.window, width, height);
    }
    #endif
    DoomRPG_setClipFalse(game);
    canvas->clipRect.w = sdlVideo.uiW;
    canvas->clipRect.h = sdlVideo.uiH;
    DoomCanvas_layout(canvas);
    if (fullView) Render_setup(render, &canvas->displayRect);
    for (int column = 0; column < render->screenWidth; ++column) {
        render->floorColor[column] = floor;
        render->ceilingColor[column] = ceiling;
    }
    render->pixels = (short*)(render->framebuffer + render->pitch * render->screenY + 2 * render->screenX);
    render->screenLeft = render->screenTop = 0;
    render->screenRight = render->screenWidth;
    render->screenBottom = render->screenHeight;
    ParticleSystem_resize(game->particleSystem);
    TextFields_refresh(canvas);
    MenuSystem_t* menu = game->menuSystem;
    menu->maxItems = SDL_max(1, (menu->type == 7 ? canvas->displayRect.h - 80 : canvas->screenRect.h) / 12);
    menu->scrollIndex = SDL_min(menu->scrollIndex, SDL_max(0, menu->numItems - menu->maxItems));
    if (menu->selectedIndex < menu->scrollIndex) menu->scrollIndex = menu->selectedIndex;
    if (menu->selectedIndex >= menu->scrollIndex + menu->maxItems)
        menu->scrollIndex = menu->selectedIndex - menu->maxItems + 1;
    canvas->staleView = true;
    canvas->isUpdateView = true;
    game->hud->isUpdate = true;
    game->menuSystem->paintMenu = true;
    DoomRPG_clearGraphics(game);
    return true;
}

boolean VideoOptions_resolution(DoomRPG_t* game, int index)
{
    if (index < 0 || index >= SDL_VIDEO_MODE_COUNT) return false;
#ifdef DOOMRPG_IOS
    return VideoOptions_mobileLayout(game);
#else
    return resizeFramebuffer(game, index, sdlVideoModes[index].width, sdlVideoModes[index].height);
#endif
}

#ifdef DOOMRPG_IOS
boolean VideoOptions_mobileLayout(DoomRPG_t* game)
{
    int width, height;
    SDL_LandscapeSize(&width, &height);
    sdlVideo.integerScaling = false;
    if (width == sdlVideo.rendererW && height == sdlVideo.rendererH) return true;
    return resizeFramebuffer(game, 11, width, height);
}
#endif

boolean VideoOptions_toggleHud(DoomRPG_t* game)
{
    sdlVideo.hideHud = !sdlVideo.hideHud;
    if (!resizeFramebuffer(game, sdlVideo.resolutionIndex, sdlVideo.rendererW, sdlVideo.rendererH)) {
        sdlVideo.hideHud = !sdlVideo.hideHud;
        return false;
    }
    return true;
}
