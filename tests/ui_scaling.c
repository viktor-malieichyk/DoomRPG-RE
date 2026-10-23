#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Render.h"
#include "Menu.h"
#include "MenuSystem.h"
#include "Sound.h"
#include "Z_Zone.h"
#include "Z_Zip.h"
#include "VideoOptions.h"
#include "../src/SDL_Video.h"
extern DoomRPG_t *doomRpg;
void test_controls(SDL_Surface* surface);
void test_video(SDL_Surface* surface);
#ifdef _WIN32
static LONG WINAPI testException(EXCEPTION_POINTERS* error)
{
    fprintf(stderr, "FAIL test exception 0x%lx at %p\n", error->ExceptionRecord->ExceptionCode, error->ExceptionRecord->ExceptionAddress);
    ExitProcess(1);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
#ifdef _WIN32
    SetUnhandledExceptionFilter(testException);
    SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
#endif
    if (argc > 1 && strcmp(argv[1], "--window") == 0) {
        SDL_SetMainReady();
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
        SDL_InitVideo();
        assert(sdlVideo.window && sdlVideo.renderer);
        assert(!SDL_GetRelativeMouseMode() && !SDL_GetWindowGrab(sdlVideo.window));
        assert(VideoOptions_fullscreen(true));
        SDL_PumpEvents();
        assert(!SDL_GetRelativeMouseMode() && !SDL_GetWindowGrab(sdlVideo.window));
        assert(SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE);
        assert(VideoOptions_fullscreen(false));
        assert(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
        assert(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
        fluidSynth.settings = new_fluid_settings();
        fluidSynth.synth = new_fluid_synth(fluidSynth.settings);
        Z_Init();
        openZipFile("DoomRPG.zip", &zipFile);
        assert(DoomRPG_Init());
        assert(VideoOptions_resolution(doomRpg, 15));
        SDL_PumpEvents();
        int w, h;
        SDL_GetWindowSize(sdlVideo.window, &w, &h);
        assert(w == 1280 && h == 720);
        assert(VideoOptions_fullscreen(true));
        assert(VideoOptions_resolution(doomRpg, 8));
        assert(SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE);
        assert(!SDL_GetRelativeMouseMode() && !SDL_GetWindowGrab(sdlVideo.window));
        assert(VideoOptions_fullscreen(false));
        SDL_PumpEvents();
        SDL_GetWindowSize(sdlVideo.window, &w, &h);
        assert(w == 320 && h == 240);
        DoomRPG_FreeAppData(doomRpg);
        closeZipFile(&zipFile);
        Mix_CloseAudio();
        delete_fluid_synth(fluidSynth.synth);
        delete_fluid_settings(fluidSynth.settings);
        SDL_Close();
        puts("PASS real window: live resize, fullscreen cursor hidden, windowed cursor visible, no capture");
        return 0;
    }
    int index = argc > 1 ? atoi(argv[1]) : 13;
    assert(index >= 0 && index < SDL_VIDEO_MODE_COUNT);
    assert(sdlVideoModes[8].width == 320 && sdlVideoModes[8].height == 240);
    for (int mode = 14; mode < SDL_VIDEO_MODE_COUNT; ++mode)
        assert(sdlVideoModes[mode].width * 9 == sdlVideoModes[mode].height * 16);
    SDL_SetMainReady();
    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    assert(SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) == 0);
    assert(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
    fluidSynth.settings = new_fluid_settings();
    assert(fluidSynth.settings);
    fluidSynth.synth = new_fluid_synth(fluidSynth.settings);
    assert(fluidSynth.synth);
    sdlVideo.resolutionIndex = index;
    sdlVideo.rendererW = sdlVideoModes[index].width;
    sdlVideo.rendererH = sdlVideoModes[index].height;
    SDL_ConfigureUISize();
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, sdlVideo.rendererW, sdlVideo.rendererH, 32, SDL_PIXELFORMAT_ARGB8888);
    assert(surface);
    sdlVideo.renderer = SDL_CreateSoftwareRenderer(surface);
    assert(sdlVideo.renderer);
    puts("renderer ready");
    Z_Init();
    openZipFile("DoomRPG.zip", &zipFile);
    puts("archive ready");
    assert(DoomRPG_Init());
    doomRpg->sound->soundEnabled = false;
    /* Configuration is loaded by startup; keep the test's active mode. */
    sdlVideo.resolutionIndex = index;
    DoomCanvas_t *canvas = doomRpg->doomCanvas;
    Render_t *render = doomRpg->render;
    assert(render->screenX + render->screenWidth <= sdlVideo.rendererW);
    assert(render->screenY + render->screenHeight <= sdlVideo.rendererH);
    assert(render->screenWidth == canvas->screenRect.w * sdlVideo.rendererW / sdlVideo.uiW);
    /* Adjacent native framebuffer pixels must survive UI compositing. */
    Render_setup(render, &canvas->displayRect);
    for (int y = 0; y < sdlVideo.rendererH; ++y) {
        Uint16 *row = (Uint16 *)(render->framebuffer + y * render->pitch);
        for (int x = 0; x < sdlVideo.rendererW; ++x)
            row[x] = (x & 1) ? 0xf800 : 0x001f;
    }
    DoomCanvas_drawRGB(canvas);
    Uint32 *pixels = (Uint32 *)((Uint8 *)surface->pixels + 10 * surface->pitch);
    assert(pixels[10] != pixels[11]);
    MenuSystem_setMenu(doomRpg->menuSystem, MENU_VIDEO);
    MenuSystem_paint(doomRpg->menuSystem);
    char path[80];
    snprintf(path, sizeof(path), "source/tests/menu-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    DoomRPG_clearGraphics(doomRpg);
    strcpy(canvas->dialogBuffer, "Resolution scaled\nReadable dialogue\nFour lines fit\nContinue playing");
    canvas->numDialogLines = 4;
    canvas->dialogTypeLineIdx = 4;
    int start = 0;
    for (int i = 0; i < 4; ++i) {
        int end = start;
        while (canvas->dialogBuffer[end] && canvas->dialogBuffer[end] != '\n') ++end;
        canvas->dialogIndexes[i * 2] = start;
        canvas->dialogIndexes[i * 2 + 1] = end - start;
        start = end + 1;
    }
    DoomCanvas_dialogState(canvas);
    snprintf(path, sizeof(path), "source/tests/dialog-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    MenuSystem_setMenu(doomRpg->menuSystem, MENU_MAIN);
    MenuSystem_paint(doomRpg->menuSystem);
    snprintf(path, sizeof(path), "source/tests/main-alignment-%dx%d.bmp", sdlVideo.rendererW, sdlVideo.rendererH);
    assert(SDL_SaveBMP(surface, path) == 0);
    test_controls(surface);
    test_video(surface);
    printf("PASS %dx%d UI %dx%d 3D %dx%d\n", sdlVideo.rendererW, sdlVideo.rendererH, sdlVideo.uiW, sdlVideo.uiH, render->screenWidth, render->screenHeight);
    return 0;
}
