/* Include the backend so the test can exercise the actual audio callback. */
#include "../ios/Midi.c"
#include <assert.h>
int main(int argc,char** argv) {
 assert(argc==3); assert(SDL_Init(0)==0);
 fluid_settings_t* settings=new_fluid_settings(); fluid_synth_t* synth=new_fluid_synth(settings);
 assert(synth && fluid_synth_sfload(synth,argv[1],1)==0);
 FILE* file=fopen(argv[2],"rb"); assert(file); fseek(file,0,SEEK_END); long length=ftell(file); rewind(file);
 void* bytes=malloc(length); assert(fread(bytes,1,length,file)==length); fclose(file);
 fluid_player_t* player=new_fluid_player(synth); assert(fluid_player_add_mem(player,bytes,length)==0); free(bytes);
 assert(fluid_player_play(player)==0); assert(fluid_player_get_status(player)==FLUID_PLAYER_PLAYING);
 short samples[2048]; long long energy=0;
 for(int i=0;i<100;i++) { music(NULL,(Uint8*)samples,sizeof(samples)); for(int j=0;j<2048;j++)energy+=abs(samples[j]); }
 assert(energy>0); fluid_player_stop(player); assert(!fluid_player_get_status(player));
 memset(samples,1,sizeof(samples)); music(NULL,(Uint8*)samples,sizeof(samples)); for(int j=0;j<2048;j++)assert(samples[j]==0);
 delete_fluid_player(player); Mix_HookMusic(NULL,NULL); delete_fluid_synth(synth); delete_fluid_settings(settings); SDL_Quit();
 puts("iOS MIDI tests passed: parsed MIDI ownership, non-silent soundfont rendering, stop/status and silence"); return 0;
}
