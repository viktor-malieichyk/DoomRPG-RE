#pragma once
void IOS_SetupStorage(void);
void IOS_Prepare(void);
void IOS_Keyboard(unsigned char* keys);
int IOS_Paused(void);

void IOS_ResetTouch(void);
void IOS_RefreshLayout(void);

void IOS_SetActive(int active);
void IOS_ShowPinKeypad(int visible);
#define IOS_PIN_EVENT (SDL_USEREVENT + 11)
