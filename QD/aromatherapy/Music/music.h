#ifndef __MUSIC_H_
#define __MUSIC_H_

#include	"config.h"
#include  "utils.h"

#define VOLUME P13
#define SONG_SELECT P14
#define MUTE P11

typedef unsigned char music_status_t;
typedef unsigned char music_volume_t;
typedef unsigned char music_select_t;

typedef struct {
	music_volume_t music_volume;
	music_select_t music_select;
} music_t;


extern volatile music_t xdata music_config_set;
extern volatile	music_status_t xdata music_status;

void init_music_cotrol();
void music_config_init();
void music_select_songs(music_select_t select_number);
void music_select_volume(music_volume_t *volume_number);
void music_switch_on_off(music_status_t status_on);
#endif
