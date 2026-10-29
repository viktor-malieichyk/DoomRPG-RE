//Using SDL and standard IO
#ifndef DOOMRPG_IOS
#define SDL_MAIN_HANDLED
#else
#include "Platform.h"
#endif
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <zlib.h>

#include "Z_Zone.h"
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Player.h"
#include "Hud.h"
#include "MenuSystem.h"
#include "SDL_Video.h"
#include "Z_Zip.h"
#include "Controls.h"
#include "Game.h"
#include "VideoOptions.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern DoomRPG_t* doomRpg;
static void GameFrame(void* unused)
{
    static int UpTime, mouseTime;
    SDL_Event ev;
    int mouse_Button;
    const Uint8* state = SDL_GetKeyboardState(NULL);
#ifdef DOOMRPG_IOS
    static int drawableW, drawableH;
    static boolean refreshDrawable = true;
#endif
    if (doomRpg->closeApplet) return;
	int currentTimeMillis = DoomRPG_GetUpTimeMS();

	mouse_Button = MOUSE_BUTTON_INVALID;

	while (SDL_PollEvent(&ev))
	{
		// check event type
		switch (ev.type) {

			// Mouse Event
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				Uint32 buttons = SDL_GetMouseState(NULL, NULL);

				if ((buttons & SDL_BUTTON_LMASK) != 0) {
					mouse_Button = MOUSE_BUTTON_LEFT;
				}
				else if ((buttons & SDL_BUTTON_MMASK) != 0) {
					mouse_Button = MOUSE_BUTTON_MIDDLE;
				}
				else if ((buttons & SDL_BUTTON_RMASK) != 0) {
					mouse_Button = MOUSE_BUTTON_RIGHT;
				}
				else if ((buttons & SDL_BUTTON_X1MASK) != 0) {
					mouse_Button = MOUSE_BUTTON_X1;
				}
				else if ((buttons & SDL_BUTTON_X2MASK) != 0) {
					mouse_Button = MOUSE_BUTTON_X2;
				}
				break;
			}

			case SDL_MOUSEWHEEL:
			{
				if (currentTimeMillis > mouseTime) {
					mouseTime = currentTimeMillis + 128;
					if (ev.wheel.y > 0) {
						mouse_Button = MOUSE_BUTTON_WHELL_UP;
					}
					else if (ev.wheel.y < 0) {
						mouse_Button = MOUSE_BUTTON_WHELL_DOWN;
					}
				}
				break;
			}


			case SDL_WINDOWEVENT:
			{
				if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                        ev.window.event == SDL_WINDOWEVENT_RESIZED ||
                        ev.window.event == SDL_WINDOWEVENT_EXPOSED) {
#ifdef DOOMRPG_IOS
                        /* Reapply after UIKit has resized the drawable. */
                        refreshDrawable = true;
#endif
                        doomRpg->doomCanvas->staleView = true;
                        doomRpg->doomCanvas->isUpdateView = true;
                        doomRpg->hud->isUpdate = true;
                        doomRpg->menuSystem->paintMenu = true;
                    }

				if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
					SDL_Log("Window %d closed", ev.window.windowID);
					closeZipFile(&zipFile);
					DoomRPG_FreeAppData(doomRpg);
					DoomRPG_CloseAudio();
					SDL_Close();
					exit(0);
					break;
				}

				break;
			}

#ifdef DOOMRPG_IOS
                case SDL_APP_DIDENTERFOREGROUND:
                    refreshDrawable = true;
                    IOS_RefreshLayout();
                    break;
#endif
#ifdef DOOMRPG_IOS
                case IOS_PIN_EVENT:
                    if (doomRpg->doomCanvas->state == ST_DIALOGPASSWORD) { int v=(int)ev.user.code; if(v<=9) DoomCanvas_handlePasswordEvents(doomRpg->doomCanvas, AVK_0+v); else if(v==10) DoomCanvas_handlePasswordEvents(doomRpg->doomCanvas, 15); else DoomCanvas_handlePasswordEvents(doomRpg->doomCanvas, AVK_SELECT); doomRpg->doomCanvas->isUpdateView=true; }
                    break;
#endif
                case SDL_KEYDOWN:
#ifndef DOOMRPG_IOS
                    if (ev.key.keysym.scancode == SDL_SCANCODE_F2 && !ev.key.repeat && !doomRpg->menuSystem->setBind) {
                        if (VideoOptions_toggleCompactUi(doomRpg)) Game_saveConfig(doomRpg->game, 0);
                    }
#endif
                    if (ev.key.keysym.scancode == SDL_SCANCODE_F1 && !ev.key.repeat && !doomRpg->menuSystem->setBind)
                        Controls_open(doomRpg);
                    break;

                case SDL_CONTROLLERDEVICEADDED:
                case SDL_CONTROLLERDEVICEREMOVED:
                case SDL_JOYDEVICEADDED:
                case SDL_JOYDEVICEREMOVED:
                    SDL_RefreshController();
                    break;

			case SDL_QUIT:
			{
				// shut down
				exit(0);
				break;
			}
		}

        }

#ifdef DOOMRPG_IOS
        if (IOS_Paused()) return;
        VideoOptions_mobileLayout(doomRpg);
        int outputW = 0, outputH = 0;
        if (SDL_GetRendererOutputSize(sdlVideo.renderer, &outputW, &outputH) == 0 &&
            outputW > 0 && outputH > 0 &&
            (refreshDrawable || outputW != drawableW || outputH != drawableH)) {
            SDL_RenderSetLogicalSize(sdlVideo.renderer, sdlVideo.rendererW, sdlVideo.rendererH);
            SDL_RenderSetIntegerScale(sdlVideo.renderer, sdlVideo.integerScaling);
            DoomRPG_setClipFalse(doomRpg);
            drawableW = outputW; drawableH = outputH; refreshDrawable = false;
            doomRpg->doomCanvas->staleView = true;
            doomRpg->doomCanvas->isUpdateView = true;
            doomRpg->hud->isUpdate = true;
            doomRpg->menuSystem->paintMenu = true;
        }
        Uint8 mobileState[SDL_NUM_SCANCODES];
        SDL_memcpy(mobileState, state, sizeof(mobileState));
        IOS_Keyboard(mobileState);
        DoomRPG_pollInput(doomRpg, mouse_Button, mobileState);
#else
        DoomRPG_pollInput(doomRpg, mouse_Button, state);
#endif

	if (currentTimeMillis > UpTime) {
		UpTime = currentTimeMillis + 15;
		DoomRPG_loopGame(doomRpg);
#ifdef DOOMRPG_IOS
        IOS_ShowPinKeypad(doomRpg->doomCanvas->state == ST_DIALOGPASSWORD);
#endif
	}
}

#ifdef DOOMRPG_IOS
int SDL_main(int argc, char* args[])
#else
int main(int argc, char* args[])
#endif
{
#ifdef DOOMRPG_IOS
    IOS_SetupStorage();
#endif
	SDL_SetMainReady();
	Z_Init();
	SDL_InitVideo();
#ifdef DOOMRPG_IOS
    IOS_Prepare();
#endif
	SDL_InitAudio();

	openZipFile("DoomRPG.zip", &zipFile);

	/*int size;
	byte* data;
	data = readZipFileEntry("c.bmp", &zipFile, &size);

	SDL_RWops* rw;
	rw = SDL_RWFromFile("c.bmp", "w");
	SDL_RWwrite(rw, data, sizeof(byte), size);
	SDL_RWclose(rw);

	SDL_free(data);

	closeZipFile(&zipFile);*/

	if (DoomRPG_Init() == 0) {
		DoomRPG_Error("Failed to initialize Doom Rpg\n");
	}

	//Hud_addMessage(doomRpg->hud, "Bienvenido a Doom RPG por GEC...");

#ifdef DOOMRPG_IOS
    /* Return control to UIKit so rotations and layout transitions can complete. */
    SDL_iPhoneSetAnimationCallback(sdlVideo.window, 1, GameFrame, NULL);
    return 0;
#else
    while (!doomRpg->closeApplet) GameFrame(NULL);
#endif

	closeZipFile(&zipFile);
	DoomRPG_FreeAppData(doomRpg);
	DoomRPG_CloseAudio();
	SDL_Close();

	return 0;
}
#ifdef _WIN32
/* CMake's WIN32 target uses this entry point with MSVC as well as MinGW. */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int show)
{
    return main(0, NULL);
}
#endif
