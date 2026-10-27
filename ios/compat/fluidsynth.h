#pragma once
#include <stddef.h>
typedef struct fluid_settings_t { double gain; } fluid_settings_t;
typedef struct fluid_synth_t fluid_synth_t;
typedef struct fluid_player_t fluid_player_t;
typedef struct fluid_audio_driver_t { int unused; } fluid_audio_driver_t;
#define FLUID_PLAYER_PLAYING 1
fluid_settings_t* new_fluid_settings(void);
void delete_fluid_settings(fluid_settings_t*);
int fluid_settings_setnum(fluid_settings_t*,const char*,double);
fluid_synth_t* new_fluid_synth(fluid_settings_t*);
void delete_fluid_synth(fluid_synth_t*);
fluid_audio_driver_t* new_fluid_audio_driver(fluid_settings_t*,fluid_synth_t*);
void delete_fluid_audio_driver(fluid_audio_driver_t*);
int fluid_is_soundfont(const char*);
int fluid_synth_sfload(fluid_synth_t*,const char*,int);
fluid_player_t* new_fluid_player(fluid_synth_t*);
void delete_fluid_player(fluid_player_t*);
int fluid_player_add_mem(fluid_player_t*,const void*,size_t);
int fluid_player_get_status(fluid_player_t*);
int fluid_player_set_loop(fluid_player_t*,int);
int fluid_player_play(fluid_player_t*);
int fluid_player_stop(fluid_player_t*);
int fluid_player_seek(fluid_player_t*,int);
