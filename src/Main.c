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
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern DoomRPG_t* doomRpg;
#ifdef DOOMRPG_IOS
int SDL_main(int argc, char* args[])
#else
int main(int argc, char* args[])
#endif
{
	SDL_Event ev;
	int		UpTime = 0;
	int		mouseTime = 0;
	int		mouse_Button;

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

	const Uint8* state = SDL_GetKeyboardState(NULL);

	
	
	while (doomRpg->closeApplet != true)
	{

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
					if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
						//printf("MESSAGE:Resizing window...\n");
						//SDL_Rect rect = { 0,0,640,480};
						//SDL_RenderSetViewport(sdlVideo.renderer, &rect);
						//resizeWindow();
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

                case SDL_KEYDOWN:
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
        if (IOS_Paused()) { SDL_Delay(15); continue; }
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
		}
	}

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
