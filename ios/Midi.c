/* iOS MIDI backend: TinySoundFont/TinyMidiLoader, mixed with WAV effects. */
#include <SDL.h>
#include <SDL_mixer.h>
#include "fluidsynth.h"
#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"
struct fluid_synth_t { tsf* font; fluid_settings_t* settings; };
struct fluid_player_t { fluid_synth_t* synth; tml_message *head,*next; double time; int playing,loops; };
static fluid_player_t* active;
static SDL_mutex* mutex;
static void reset(fluid_player_t* p) {
 p->time=0; p->next=p->head; tsf_reset(p->synth->font);
 for(int c=0;c<16;c++) { tsf_channel_sounds_off_all(p->synth->font,c); tsf_channel_set_presetnumber(p->synth->font,c,0,c==9); }
}
static void music(void* unused,Uint8* bytes,int len) {
 SDL_LockMutex(mutex);
 short* out=(short*)bytes; int frames=len/4; SDL_memset(bytes,0,len);
 fluid_player_t* p=active; if(!p || !p->playing || !p->synth->font) { SDL_UnlockMutex(mutex); return; }
 tsf* f=p->synth->font; tsf_set_volume(f,(float)p->synth->settings->gain);
 while(frames>0) {
  int n=frames>64?64:frames;
  while(p->next && p->next->time<=p->time) {
   tml_message* m=p->next;
   switch(m->type) {
    case TML_PROGRAM_CHANGE: tsf_channel_set_presetnumber(f,m->channel,m->program,m->channel==9); break;
    case TML_NOTE_ON: tsf_channel_note_on(f,m->channel,m->key,m->velocity/127.f); break;
    case TML_NOTE_OFF: tsf_channel_note_off(f,m->channel,m->key); break;
    case TML_PITCH_BEND: tsf_channel_set_pitchwheel(f,m->channel,m->pitch_bend); break;
    case TML_CONTROL_CHANGE: tsf_channel_midi_control(f,m->channel,m->control,m->control_value); break;
   } p->next=m->next;
  }
  tsf_render_short(f,out,n,0); out+=n*2; frames-=n; p->time+=n*1000.0/44100;
  if(!p->next) { if(p->loops && p->head) { if(p->loops>0)--p->loops; reset(p); } else { p->playing=0; break; } }
 }
 SDL_UnlockMutex(mutex);
}
fluid_settings_t* new_fluid_settings(void) { mutex=SDL_CreateMutex(); fluid_settings_t* s=calloc(1,sizeof(*s)); if(s)s->gain=1; return s; }
void delete_fluid_settings(fluid_settings_t* s) { free(s); SDL_DestroyMutex(mutex); mutex=NULL; }
int fluid_settings_setnum(fluid_settings_t* s,const char* key,double v) { SDL_LockMutex(mutex); s->gain=v; SDL_UnlockMutex(mutex); return 1; }
fluid_synth_t* new_fluid_synth(fluid_settings_t* s) { fluid_synth_t* f=calloc(1,sizeof(*f)); if(f)f->settings=s; return f; }
void delete_fluid_synth(fluid_synth_t* f) { if(f) { if(f->font)tsf_close(f->font); free(f); } }
fluid_audio_driver_t* new_fluid_audio_driver(fluid_settings_t* s,fluid_synth_t* f) { return calloc(1,sizeof(fluid_audio_driver_t)); }
void delete_fluid_audio_driver(fluid_audio_driver_t* d) { Mix_HookMusic(NULL,NULL); active=NULL; free(d); }
int fluid_is_soundfont(const char* path) { FILE* f=fopen(path,"rb"); if(!f)return 0; fclose(f); return 1; }
int fluid_synth_sfload(fluid_synth_t* f,const char* path,int reset) { f->font=tsf_load_filename(path); if(!f->font)return -1; tsf_set_output(f->font,TSF_STEREO_INTERLEAVED,44100,-6); return 0; }
fluid_player_t* new_fluid_player(fluid_synth_t* f) { fluid_player_t* p=calloc(1,sizeof(*p)); if(p)p->synth=f; return p; }
void delete_fluid_player(fluid_player_t* p) { if(!p)return; fluid_player_stop(p); tml_free(p->head); free(p); }
int fluid_player_add_mem(fluid_player_t* p,const void* data,size_t len) { p->head=tml_load_memory(data,(int)len); p->next=p->head; return p->head?0:-1; }
int fluid_player_get_status(fluid_player_t* p) { SDL_LockMutex(mutex); int s=p && p->playing; SDL_UnlockMutex(mutex); return s; }
int fluid_player_set_loop(fluid_player_t* p,int n) { SDL_LockMutex(mutex); p->loops=n; SDL_UnlockMutex(mutex); return 0; }
int fluid_player_play(fluid_player_t* p) { if(!p||!p->head||!p->synth->font)return -1; SDL_LockMutex(mutex); if(active && active!=p)active->playing=0; reset(p); p->playing=1; active=p; SDL_UnlockMutex(mutex); Mix_HookMusic(music,NULL); return 0; }
int fluid_player_stop(fluid_player_t* p) { SDL_LockMutex(mutex); if(p)p->playing=0; if(active==p)active=NULL; SDL_UnlockMutex(mutex); return 0; }
int fluid_player_seek(fluid_player_t* p,int tick) { SDL_LockMutex(mutex); if(p && p->synth->font)reset(p); SDL_UnlockMutex(mutex); return 0; }

int IOS_ValidateSoundfont(const void* bytes,size_t size) {
 if(!bytes || size<12 || size>64*1024*1024)return 0;
 tsf* font=tsf_load_memory(bytes,(int)size);
 if(!font)return 0;
 tsf_close(font); return 1;
}
