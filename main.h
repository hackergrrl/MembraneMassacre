#ifndef _MAIN_H_
#define _MAIN_H_

#include <allegro.h>
#include <iostream>
using namespace std;

// Difficulty settings
#define DIFF_EASY		1
#define DIFF_NORMAL		2
#define DIFF_HARD		3
#define DIFF_IMPOSSIBLE	4

// Fade modes
#define FADE_NONE		0
#define FADE_IN			1
#define FADE_OUT		2

#define MAX_MUSIC		8

#define MODE_STORY		0
#define MODE_SURVIVAL	1


extern BITMAP *backbuffer;
extern BITMAP *missileimage, *missileimage2;
extern BITMAP *bubbleimage;
extern BITMAP *explosion_trail, *smoke_trail;
extern BITMAP *ships[5];
extern long gcount;
extern SAMPLE *mg_sound;
extern SAMPLE *blaster_sound;
extern SAMPLE *boom_sound;
extern SAMPLE *rocket_sound;
extern SAMPLE *laser_sound;
extern SAMPLE *fbomb_sound, *freeze1_sound, *freeze2_sound;
extern SAMPLE *pop1_sound, *pop2_sound;
extern SAMPLE *bonus_sound;
extern SAMPLE *highlight_sound, *select_sound;
extern SAMPLE *stretch_sound;
extern SAMPLE *respawn_sound[2];
extern BITMAP *cellbody[4];
extern FONT *font2;
extern int enemy_count;
extern float total_lvl, lvl;
extern int win_pause;
extern float screen_shake;
extern char game_mode;
extern long survive_time;
extern int game_state;

void draw_game();
void thickline(BITMAP *buf, int x1, int y1, int x2, int y2, int width, int r, int g, int b, bool fade);
void start_level();
void start_survival();
void action_newgame();
void action_exit();
void action_resume();
void action_options();
void action_returnmenu();
void action_credits();
void action_shipselect();
void action_restartlevel();
void play_sound(SAMPLE *sfx, int x, int y);
void circle_alpha(BITMAP *bmp, int x, int y, int alpha);
void putpixel_alpha(BITMAP *bmp, int x, int y, int col, int alpha);
void putpixel_add(BITMAP *bmp, int x, int y, int col, int alpha);
void perform_fade(int speed, int col);
float diff_mod();
bool is_survival();
void update_survival_spawn();

// Game states
#define STATE_TITLE		0
#define STATE_GAME		1
#define STATE_VICTORY	2


// Enemy spawning data for Survival mode
#define SPAWN_SINEWAVE_HORZ		1
#define SPAWN_SINEWAVE_VERT		2
#define SPAWN_CIRCLE			3
#define SPAWN_SPIRAL			4
#define SPAWN_LINE				5
#define SPAWN_VORTEX			6

struct SPAWNDATA
{
	int x, y;
	char type;  // Spawn style
	int spawncount, spawntime;
	char enemy_genre;  // Species to spawn
	int enemycount;  // Enemies left to spawn

	int amplitude, wavelength;  // For sine wave spawns
	int radius;  // For spiral, concentric, and circle
};

typedef int FSOUND_STREAM;
long timeGetTime();
bool FSOUND_IsPlaying(int id);
void FSOUND_Stream_Play(int id, FSOUND_STREAM * sample);
void FSOUND_SetSFXMasterVolume(float volume);
void FSOUND_Stream_Close(FSOUND_STREAM * sample);
void FSOUND_Close();

#endif
