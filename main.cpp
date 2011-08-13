#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include "main.h"
#include "map.h"
#include "sprites.h"
#include "player.h"
#include "utils.h"
#include "weapons.h"
#include "enemy.h"
#include "menu.h"

#define MAX_RANKS		14
const char* ranks[] = {
	"Sir Suck-A-Lot",
	"Dust Bunny",
	"Seat Warmer",
	"Cytoplasm Stealer",
	"Membrane Mauler",
	"Biology Bigshot",
	"Captain Cleansor",
	"Cell Crusher",
	"Bacteria Badass",
	"Endoplasmic Reticulum Eliminator",
	"Julius Nucleus",
	"Microscopic Madman",
	"Membrane Master",
	"God's Gift to Science",
};


BITMAP *backbuffer;
BITMAP *missileimage, *missileimage2;
BITMAP *explosion_trail, *smoke_trail;
SAMPLE *mg_sound;
SAMPLE *blaster_sound;
SAMPLE *boom_sound;
SAMPLE *rocket_sound;
SAMPLE *laser_sound;
SAMPLE *fbomb_sound, *freeze1_sound, *freeze2_sound;
SAMPLE *pop1_sound, *pop2_sound;
SAMPLE *bonus_sound;
SAMPLE *fireworks_sound;
SAMPLE *stretch_sound;
SAMPLE *level_sound;
SAMPLE *highlight_sound, *select_sound;
SAMPLE *wave_sound[4];
SAMPLE *respawn_sound[2];
BITMAP *icons[MAX_WEAPONS];
BITMAP *cellbody[4];
BITMAP *ships[5];
BITMAP *bg[3];
BITMAP *title_text, *credits_text, *enter_text, *victory_text;
BITMAP *bubbleimage;
BITMAP *number[10];
BITMAP *level_txt, *wave_txt, *final_txt;
BITMAP *battery_hud;
BITMAP *fbomb_hud;
GUI *ship_label=NULL;
GUI *start_button=NULL;
FSOUND_STREAM* game_music[MAX_MUSIC];
FSOUND_STREAM* victory_music=0;
SPAWNDATA spawndata;
int cur_music = 0;
int music_count = 0;
char game_mode = MODE_STORY;
long survive_time = 0;
long wave_survive_time = 0;
long wave_duration = 0;
volatile long tix=0;
int gui_colour=255;
long gcount=0;
int enemy_count = 0;
float total_lvl = 0, lvl = 0;
int cur_level, cur_wave;
int game_state = STATE_TITLE;
int ts_ang;
int bonus_count=0;
int win_pause=0;
int config[MAX_CONFIG];
FONT *font2;
int cur_menu=0;
bool game_running=true;
bool paused=false;
float screen_shake=0;
int fade_count=0;
int fade_mode=FADE_NONE;
int fade_colour=0;
int fade_speed=3;
int ld_count=0;
int rcount=0;
int new_ship=0;
long last=0;

long timeGetTime()
{
    //timespec ts;
    // clock_gettime(CLOCK_MONOTONIC, &ts); // Works on FreeBSD
    //clock_gettime(CLOCK_REALTIME, &ts); // Works on Linux
    //return (ts.tv_sec / 1000) + (ts.tv_nsec / 1000000);

	timeval time;
	gettimeofday(&time, NULL);
	return time.tv_usec / 1000 + time.tv_sec * 1000;
}

bool FSOUND_IsPlaying(int id)
{
	return true;
}
void FSOUND_Stream_Play(int id, FSOUND_STREAM * sample)
{
}
void FSOUND_SetSFXMasterVolume(float volume)
{
}
void FSOUND_Stream_Close(FSOUND_STREAM * sample)
{
}
void FSOUND_Close()
{
}


bool init_game();
void draw_minimap();
void draw_hud();
void play_sound(SAMPLE *sfx, int x, int y);
void draw_bigword(char type, int x, int y, int r, int g, int b);
void draw_number(int num, int x, int y, int r, int g, int b);
void gradientrect(BITMAP *bmp, int x1, int y1, int x2, int y2, int c1, int c2, int c3, int c4);
void create_main_menu();
void create_options_menu();
void create_credits();
void create_shipselect();
void create_ship_gain(int s);
void create_survival_rank();
void create_game_over();
void draw_rating_bars(int x, int y, int rating);
void additiverect(int x1, int y1, int x2, int y2, int col, int alpha);
void ditherrect(int x, int y, int w, int h, int col);
void update_survival_spawn();
void clear_level();
SPRITE* create_enemy_pop(int x, int y, int genre);
BITMAP* generate_ship_portrait(int s, int rot, bool scanlines);


bool is_survival()
{
	return game_mode == MODE_SURVIVAL;
}


void inc_tix()
{
     tix++;
}
END_OF_FUNCTION(inc_tix);

int main()
{
	init_game();


	// Create player
	SPRITE *S = new SPRITE();
    S->type = SPRITE_PLAYER;
    S->x = random(level->w);
    S->y = random(level->h);
	S->xv = S->yv = S->lastx = S->lasty = 0;
    S->friction = 0.98f;
    S->bounce = 0.5f;
    S->lifetime = -1;
    add_sprite(S);
    player = S;
	set_ship_type(1);
	S->playerdata.alive = false;
	S->playerdata.rx = -1000000000;
	S->playerdata.ry = -1000000000;
	S->enemydata.frozen = 0;
	S->playerdata.ramming_power = 3.0f;
	respawn_wait = 240;

	generate_bg(bg[random(3)], 0, 0, 70);
    apply_texture_to_map(level_fg);
    generate_map(0);
	curweapon = 0;
	cur_level = -1;
	cur_wave = 0;
	total_lvl = 2000*cur_level;
	
	for(int b=0; b < 6; b++)
	{
		int x = random(level->w-200)+100;
		int y = random(level->h-200)+100;
		int g = random(12);

		for(int a=0; a < random(20)+1; a++)
		{
		    create_enemy(NULL, random(100)+x-50, random(100)+y-50, g);
		}
	}

	ts_ang = 45;
	mapx = random(level->w);
	mapy = random(level->h);

	// Generate the main menu
	gui_colour = makecol(170,120,0);
	create_main_menu();

    last = timeGetTime();
	while(game_running)
    {
        while(tix > 0)
        {
			tix--;
			spf = 0;

			if(!paused)
			{
				if(game_state == STATE_VICTORY && gcount % 15 == 0)
					spawn_particles(50, mapx+random(SCREEN_W), mapy+random(SCREEN_H), 1, random(128)+128,random(128)+128,random(128)+128,255, 96,96,96,0, 0,359, 1.0f,2.0f, 0.99f, 80, false, BLEND_ADD);

				update_sprites();
				laser_delay = maxf(0, laser_delay-1);
				gcount++;
				bonus_count = maxf(0, bonus_count-1);
				win_pause = maxf(0, win_pause-1);
				screen_shake = maxf(0, screen_shake-1);
				ld_count = maxf(0, ld_count-1);

				if(!FSOUND_IsPlaying(0) && game_state != STATE_VICTORY)
				{
					FSOUND_Stream_Play(0, game_music[cur_music]);
				}
				if(win_pause > 0 && game_state == STATE_GAME)
				{
					float per = (float)win_pause / 300.0f;
					FSOUND_SetSFXMasterVolume(config[CFG_MUSIC_VOLUME] * per);
				}
				if(game_state == STATE_VICTORY)
					FSOUND_SetSFXMasterVolume(config[CFG_MUSIC_VOLUME]);

				if(is_survival())
				{
					survive_time += timeGetTime() - last;
					wave_survive_time += timeGetTime() - last;
					update_survival_spawn();

					if( (wave_survive_time / 1000 * 60) >= wave_duration && fade_mode == FADE_NONE && game_state == STATE_GAME)
					{
						perform_fade(6, makecol(255,64,64));
					}
				}
				last = timeGetTime();
			}
			
			// Prevent screen-shake while game is paused (or in menu)
			if(paused || game_state == STATE_TITLE)
			{
				screen_shake = 0;
			}
			rcount++;

			if(is_survival() && !player->playerdata.alive && cur_wave > 1 && game_state == STATE_GAME)
				create_survival_rank();

			if(key[KEY_ESC])			
			{
				if((game_state == STATE_GAME && cur_menu==0)
					|| (game_state == STATE_TITLE && cur_menu==4))
				{
					clear_gui();
					create_main_menu();
					paused = true;
				}
			}
			
			if( (key[KEY_ENTER] || key[KEY_ESC]) && game_state == STATE_VICTORY)
			{
				create_main_menu();
				stop_sample(fireworks_sound);
				game_state = STATE_TITLE;
				paused = true;
			}
			
			if(enemy_count <= 0 && game_state == STATE_GAME && fade_mode == FADE_NONE && !paused)
			{
				if(is_survival())
				{
					perform_fade(6, makecol(255,64,64));
				}
				else if(cur_level != 4)
				{
					if(cur_wave < 4)
						perform_fade(2, makecol(255,255,255));
					else
						perform_fade(2, makecol(0,0,0));
				}
			}

			// Play "Wave X!" sound effects
			if(ld_count == 165)
				play_sample(level_sound, config[CFG_SFX_VOLUME], 128, 1000, 0);
			if(ld_count == 120 && cur_level != 4 && !is_survival())
			{
				play_sample(wave_sound[cur_wave-1], config[CFG_SFX_VOLUME], 128, 1000, 0);			
			}

			// Scroll camera around map when a menu is active
			if(cur_menu != 0)
			{
				mapx += cos( ts_ang * (3.14159f/180.0f) ) * 4;
				if(mapx+SCREEN_W >= level->w)
				{
					mapx = level->w-SCREEN_W;
					ts_ang = 180 - ts_ang;
				}
				if(mapx <= 0)
				{
					mapx = 0;
					ts_ang = 180 - ts_ang;
				}
				mapy += sin( ts_ang * (3.14159f/180.0f) ) * 4;
				if(mapy+SCREEN_H >= level->h)
				{
					mapy = level->h-SCREEN_H;
					ts_ang = 360 - ts_ang;
				}
				if(mapy <= 0)
				{
					mapy = 0;
					ts_ang = 360 - ts_ang;
				}
			}
        
			if(game_state == STATE_GAME && cur_menu == 0)
			{
				mapx = (int)S->x - SCREEN_W/2 - S->image->w/2;
				mapy = (int)S->y - SCREEN_H/2 - S->image->h/2;
			}
			mapx = maxf(0, minf(mapx, level->w-SCREEN_W));
			mapy = maxf(-32, minf(mapy, level->h-SCREEN_H+32));

			// Fader updates
			if(fade_mode != FADE_NONE)
			{
				if(fade_mode == FADE_OUT)
				{
					fade_count += fade_speed;
				}
				else if(fade_mode == FADE_IN)
				{
					fade_count -= fade_speed;
				}
				if(fade_count > 255)
				{
					fade_mode = FADE_IN;
					fade_count = 255;
					// If enemies are all gone, or (in suvival) we've run out of time on the wave
					if(enemy_count <= 0 || ((wave_survive_time / 1000 * 60) >= wave_duration && is_survival()))
					{
						if(is_survival())
							start_survival();
						else
							start_level();
					}
					// Player has finished 'dieing'
					else if(!player->playerdata.alive && !is_survival() && !paused)
					{
						if(player->playerdata.lives < 0)
							create_game_over();
						else
						{
							player->x = player->lastx = player->playerdata.rx;
							player->y = player->lasty = player->playerdata.ry;
						}
					}
				}
				else if(fade_count < 0)
				{
					fade_mode = FADE_NONE;
					fade_count = 0;
				}

				if(fade_colour == makecol(0,0,0) && fade_mode == FADE_OUT)
				{
					float per = float(255-fade_count) / 255.0f;
					FSOUND_SetSFXMasterVolume( config[CFG_MUSIC_VOLUME] * per );					
				}
				else
					FSOUND_SetSFXMasterVolume( config[CFG_MUSIC_VOLUME] );
			}
			if(cur_level == 4 && enemy_count <= 0 && !is_survival())
				start_level();		
		}

        draw_game();

        // TEMP: Make screenshots every few seconds for possible Good Shots(tm)!
		/*
		static int scr=0;
		if(gcount % 60*25==0)
		{
			char buf[32];
			sprintf(buf, "Screenshots\\mm_scr%d.bmp", scr);
			save_bmp(buf, backbuffer, NULL);
			scr++;
		}*/

		acquire_screen();
        blit(backbuffer, screen, 0,0, 0,0, SCREEN_W,SCREEN_H);
        release_screen();
    }

	save_config();

	for(int a=0; a < MAX_MUSIC; a++)
		FSOUND_Stream_Close(game_music[a]);
	FSOUND_Close();

    return 0;
}
END_OF_MAIN()




bool init_game()
{
    allegro_init();

	// Load game configuration data from file (settings.cfg)
	load_config();

    install_keyboard();
    install_mouse();
    install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
    install_timer();
    install_int_ex(inc_tix, BPS_TO_TIMER(60));
    set_color_depth( config[CFG_COLOUR_DEPTH] );
	if(config[CFG_FULLSCREEN])
	    set_gfx_mode(GFX_AUTODETECT, 480, 320, 0,0);
	else
		//set_gfx_mode(GFX_AUTODETECT_WINDOWED, 480, 320, 0,0);
		set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0,0);
	set_window_title("Membrane Massacre :: v2.5");

	font2 = load_font("data/font.pcx", NULL, NULL);
    
    backbuffer = create_bitmap(SCREEN_W,SCREEN_H);
    bg[0] = load_bitmap("data/bg1.bmp", NULL);
    bg[1] = load_bitmap("data/bg2.bmp", NULL);
    bg[2] = load_bitmap("data/bg3.bmp", NULL);
    level_bg = generate_bg(bg[random(3)], 0,0,64);
    level_fg = load_bitmap("data/stomach.bmp", NULL);
    level = create_bitmap(1280*2,960*2);
	level_shadow = create_bitmap(1280*2,960*2);
	cellbody[0] = load_bitmap("data/cell.bmp", NULL);
	cellbody[1] = load_bitmap("data/cell2.bmp", NULL);
	cellbody[2] = load_bitmap("data/cell3.bmp", NULL);
	cellbody[3] = load_bitmap("data/cell4.bmp", NULL);
	title_text = load_bitmap("data/title2.bmp", NULL);
	victory_text = load_bitmap("data/victory.bmp", NULL);
	explosion_trail = load_bitmap("data/explosion_trail.bmp", NULL);
	smoke_trail = load_bitmap("data/smoke_trail.bmp", NULL);
	bubbleimage = load_bitmap("data/bubble.bmp", NULL);
    
    mg_sound = load_wav("data/mg.wav");
    blaster_sound = load_wav("data/blaster.wav");
    boom_sound = load_wav("data/boom.wav");
    rocket_sound = load_wav("data/rocket.wav");
    laser_sound = load_wav("data/laser.wav");
    fbomb_sound = load_wav("data/fbomb.wav");
    freeze1_sound = load_wav("data/freeze1.wav");
    freeze2_sound = load_wav("data/freeze2.wav");
    pop1_sound = load_wav("data/pop1.wav");
    pop2_sound = load_wav("data/pop2.wav");
    bonus_sound = load_wav("data/bonus.wav");
	fireworks_sound = load_wav("data/fireworks.wav");
	stretch_sound = load_wav("data/stretch.wav");
	level_sound = load_wav("data/levellock.wav");
	highlight_sound = load_wav("data/highlight.wav");
	select_sound = load_wav("data/select.wav");
	wave_sound[0] = load_wav("data/wave1.wav");
	wave_sound[1] = load_wav("data/wave2.wav");
	wave_sound[2] = load_wav("data/wave3.wav");
	wave_sound[3] = load_wav("data/wave4.wav");
	respawn_sound[0] = load_wav("data/respawn.wav");
	respawn_sound[1] = load_wav("data/respawnblast.wav");

	for(int a=0; a < 10; a++)
	{
		char buf[32];
		sprintf(buf, "data/%d.bmp", a);
		number[a] = load_bitmap(buf, NULL);
	}
	level_txt = load_bitmap("data/level.bmp", NULL);
	final_txt = load_bitmap("data/final.bmp", NULL);
	wave_txt = load_bitmap("data/wave.bmp", NULL);
    
    icons[0] = load_bitmap("data/icon_blaster.bmp",NULL); 
    icons[1] = load_bitmap("data/icon_mgs.bmp",NULL);
    icons[2] = load_bitmap("data/icon_missiles.bmp",NULL); 
    icons[3] = load_bitmap("data/icon_laser.bmp",NULL); 
    icons[4] = load_bitmap("data/icon_fbomb.bmp",NULL); 
    
    srand( time(NULL) );
  
	ships[0] = load_bitmap("data/ship1.bmp", NULL);
	ships[1] = load_bitmap("data/ship2.bmp", NULL);
	ships[2] = load_bitmap("data/ship3.bmp", NULL);
	ships[3] = load_bitmap("data/ship4.bmp", NULL);
	ships[4] = load_bitmap("data/ship5.bmp", NULL);
    missileimage = load_bitmap("data/missile.bmp", NULL);
    missileimage2 = load_bitmap("data/missile2.bmp", NULL);
	battery_hud = load_bitmap("data/battery.bmp", NULL);
	fbomb_hud = load_bitmap("data/icon_fbomb2.bmp", NULL);


	// Init FMOD
	//FSOUND_Init(44100, 64, 0);
	//FSOUND_SetSFXMasterVolume( config[CFG_MUSIC_VOLUME]);

	/*
	game_music[0] = FSOUND_Stream_Open("data/Track_0.mp3", 0, 0,0);
	game_music[1] = FSOUND_Stream_Open("data/Track_1.mp3", 0, 0,0);
	game_music[2] = FSOUND_Stream_Open("data/Track_2.mp3", 0, 0,0);
	game_music[3] = FSOUND_Stream_Open("data/Track_3.mp3", 0, 0,0);
	game_music[4] = FSOUND_Stream_Open("data/Track_4.mp3", 0, 0,0);
	game_music[5] = FSOUND_Stream_Open("data/Track_5.mp3", 0, 0,0);
	game_music[6] = FSOUND_Stream_Open("data/Track_6.mp3", 0, 0,0);
	game_music[7] = FSOUND_Stream_Open("data/victory.mp3", 0, 0,0);
	*/
	music_count = 0;

//	FSOUND_PlaySound(0, game_music[random(5)]);

	return true;
}




void draw_game()
{
    clear_bitmap(backbuffer);
    
    // Screen shake
	int rmx = mapx;
	int rmy = mapy;
	if(screen_shake > 0 && !paused)
	{
		mapx += random(screen_shake*2) - screen_shake;
		mapy += random(screen_shake*2) - screen_shake;
	}
	
	draw_map();

    draw_sprites();
    
	if(game_state == STATE_GAME)
	{
//		draw_hud();
//		draw_minimap();
	}
	else if(game_state == STATE_VICTORY)
		draw_sprite(backbuffer, victory_text, 0,0);

	

	// Draw screen fade effect
	if(fade_mode != FADE_NONE)
	{
		drawing_mode(DRAW_MODE_TRANS, NULL, 0,0);
		set_trans_blender(0,0,0,fade_count);
		rectfill(backbuffer,0,0,SCREEN_W,SCREEN_H,fade_colour);
		drawing_mode(DRAW_MODE_SOLID, NULL, 0,0);
	}


	// Level/Wave notifications
	if(ld_count > 0)
	{
		int r1=255, g1=0, b1=0;
		int r2=0, g2=255, b2=255;

		if(!is_survival())
		{
			if(cur_level == 5)
			{
				r1 = 255; g1 = 255; b1 = 0;
			}
			if(cur_wave == 4)
			{
				r2 = 0; g2 = 0; b2 = 255;
			}
		}
		
		// Fade-In
		if(ld_count >= 160)
		{
			float per = 1 - (float(ld_count-160) / 40.0f);
			// Level notification
			if(is_survival())
			{
				int bx = 480*per-140;
				draw_bigword('W', bx-140, 280, r2,g2,b2);
				draw_number(cur_wave, bx, 280, r2,g2,b2);
			}
			else
			{
				if(cur_wave == 1)
				{
					int by = 240*per-100;
					if(cur_level == 4)
					{
						draw_bigword('L', 330, by, r1,g1,b1);
						draw_bigword('F', 220, by, r1,g1,b1);
					}
					else
					{
						draw_bigword('L', 300-level_txt->w/2, by, r1,g1,b1);
						draw_number(cur_level, 300+80, by, r1,g1,b1);
					}
				}
				// Wave notification
				if(cur_level != 4)
				{
					int bx = 480*per-140;
					if(cur_wave == 4)
					{
						draw_bigword('F', bx-170, 280, r2,g2,b2);
						draw_bigword('W', bx-60, 280, r2,g2,b2);
					}
					else
					{
						draw_bigword('W', bx-140, 280, r2,g2,b2);
						draw_number(cur_wave, bx, 280, r2,g2,b2);
					}
				}
			}
		}

		// Fade-Out
		else if(ld_count <= 40)
		{
			float per = (float(ld_count) / 40.0f);
			// Level notification
			if(is_survival())
			{
				int bx = 480*(1-per)+380;
				draw_bigword('W', bx-140, 280, r2,g2,b2);
				draw_number(cur_wave, bx, 280, r2,g2,b2);
			}
			else
			{
				if(cur_wave == 1)
				{
					int by = 240*per-100;
					if(cur_level == 4)
					{
						draw_bigword('L', 330, by, r1,g1,b1);
						draw_bigword('F', 220, by, r1,g1,b1);
					}
					else
					{
						draw_bigword('L', 300-level_txt->w/2, by, r1,g1,b1);
						draw_number(cur_level, 300+80, by, r1,g1,b1);
					}
				}
				// Wave notification
				if(cur_level != 4)
				{
					int bx = 480*(1-per)+380;
					if(cur_wave == 4)
					{
						draw_bigword('F', bx-170, 280, r2,g2,b2);
						draw_bigword('W', bx-60, 280, r2,g2,b2);
					}
					else
					{
						draw_bigword('W', bx-140, 280, r2,g2,b2);
						draw_number(cur_wave, bx, 280, r2,g2,b2);
					}
				}
			}
		}

		// Stationary
		else
		{
			// Level notification
			if(is_survival())
			{
				int bx = 380;
				draw_bigword('W', bx-140, 280, r2,g2,b2);
				draw_number(cur_wave, bx, 280, r2,g2,b2);
			}
			else
			{
				if(cur_wave == 1)
				{
					int by = 140;
					if(cur_level == 4)
					{
						draw_bigword('L', 330, by, r1,g1,b1);
						draw_bigword('F', 220, by, r1,g1,b1);
					}
					else
					{
						draw_bigword('L', 300-level_txt->w/2, by, r1,g1,b1);
						draw_number(cur_level, 300+80, by, r1,g1,b1);
					}
				}
				// Wave notification
				if(cur_level != 4)
				{
					int bx = 380;
					if(cur_wave == 4)
					{
						draw_bigword('F', bx-170, 280, r2,g2,b2);
						draw_bigword('W', bx-60, 280, r2,g2,b2);
					}
					else
					{
						draw_bigword('W', bx-140, 280, r2,g2,b2);
						draw_number(cur_wave, bx, 280, r2,g2,b2);
					}
				}
			}
		}
	}


	//int col = makecol(24,90,120);
	//int col = makecol(128,0,0);
	//int col = makecol(32,120,32);
	//int col = makecol(170,120,0);
	draw_guis(backbuffer, gui_colour);

	// Draw ship portrait for ship-gain window
	if(cur_menu == 5)
	{
		BITMAP *tmp = generate_ship_portrait(new_ship, rcount, false);
		stretch_sprite(backbuffer, tmp, 320-tmp->w/4,255, tmp->w/2, tmp->h/2);
		destroy_bitmap(tmp);
	}
	// Draw extra components for ship selection menu
	if(cur_menu == 4)
	{
		int s = player->playerdata.shiptype;
		// Update ship-name GUI label
		if(ship_label)
		{
			if(shipdata[s-1].owned)
			{
				if(s == 1) strcpy(ship_label->text, "Fighter X-11");
				if(s == 2) strcpy(ship_label->text, "Crimson Wing");
				if(s == 3) strcpy(ship_label->text, "Indigo Avenger");
				if(s == 4) strcpy(ship_label->text, "Juggernaut");
				if(s == 5) strcpy(ship_label->text, "Doubleheader");
			}
			else
				strcpy(ship_label->text, "Not Unlocked");
		}

		// Draw rating bars (thrust, top-speed, hull strength, ammo regeneration rate)
		int tr = shipdata[s-1].thrust_rating;
		int sr = shipdata[s-1].topspeed_rating;
		int hr = shipdata[s-1].hull_rating;
		int ar = shipdata[s-1].ammo_rating;
		if(!shipdata[s-1].owned)
		{
			tr = sr = hr = ar = 0;
			start_button->disabled = true;
		}
		else start_button->disabled = false;
        int x = SCREEN_W/2;
		draw_rating_bars(x-260, 200, tr);
		draw_rating_bars(x-260, 300, hr);
		draw_rating_bars(x-100, 200, sr);
		draw_rating_bars(x-100, 300, ar);

		// Draw ship portrait
		BITMAP *tmp = generate_ship_portrait(s, rcount, true);
		draw_sprite(backbuffer, tmp, x+80,190);
		destroy_bitmap(tmp);

		// Draw notches along the bottom of the portrait for reference of ship
		for(int a=0; a < 5; a++)
		{
			int x1 = x+95+30*a;
			int y1 = 355;
			int y2 = 355+18;
			int x2 = x+95+18+30*a;
			int col1 = gui_colour;
			if(!shipdata[a].owned)
				col1 = makecol(128,128,128);
			if(s-1 == a)
				col1 = makecol(minf(255,getr(col1)*1.7),minf(255,getg(col1)*1.7),minf(255,getb(col1)*1.7));
			int col2 = makecol(getr(col1)*0.5,getg(col1)*0.5,getb(col1)*0.5);
			gradientrect(backbuffer, x1,y1,x2,y2, col1, col1, col2, col2);
			additiverect(x1-1,y1-1,x2+1,y2+1, col1, 128);
			if(s-1 == a)
			{
				additiverect(x1-2,y1-2,x2+2,y2+2, col1, 64);
				additiverect(x1-3,y1-3,x2+3,y2+3, col1, 32);
			}
		}
	}

	// Draw game logo
	if(cur_menu != 0)
		stretch_sprite(backbuffer, title_text, SCREEN_W/2-title_text->w/2,10, title_text->w*1.0,title_text->h*1.0);

	// Mouse cursor
	thickline(backbuffer, mouse_x-7,mouse_y, mouse_x-3,mouse_y, 2, 255,255,255, false);
	thickline(backbuffer, mouse_x+7,mouse_y, mouse_x+3,mouse_y, 2, 255,255,255, false);
	thickline(backbuffer, mouse_x,mouse_y-7, mouse_x,mouse_y-3, 2, 255,255,255, false);
	thickline(backbuffer, mouse_x,mouse_y+7, mouse_x,mouse_y+3, 2, 255,255,255, false);
}


void draw_beamline(BITMAP *buf, int x, int y, int d)
{
	if(x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H)
		return;
	putpixel_alpha(buf, x,y, makecol(getr32(d),getg32(d),getb32(d)), geta32(d));
}

void thickline(BITMAP *buf, int x1, int y1, int x2, int y2, int width, int r, int g, int b, bool fade)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    float ang = atan2((float)dy,(float)dx);
    
    if(fade && width <= 1)
		do_line(buf, x1,y1, x2,y2, makeacol32(r,g,b,128), draw_beamline);
	else
		line(buf, x1,y1, x2,y2, makecol(r,g,b));
    for(int a=1; a < width; a++)
    {
        int nr, ng, nb, na;
        nr = r; ng = g; nb = b; na = 255;
        if(fade)
			na = (1-((float)a / (float)(width))) * 128.0f;
        
        dx = cos( ang + (3.14159f/2.0f) ) * a * 0.5f;
        dy = sin( ang + (3.14159f/2.0f) ) * a * 0.5f;
		if(fade)
			do_line(buf, x1+dx,y1+dy, x2+dx,y2+dy, makeacol32(nr,ng,nb,na), draw_beamline);
		else
			line(buf, x1+dx,y1+dy, x2+dx,y2+dy, makecol(nr,ng,nb));

        dx = cos( ang - (3.14159f/2.0f) ) * a * 0.5f;
        dy = sin( ang - (3.14159f/2.0f) ) * a * 0.5f;
		if(fade)
			do_line(buf, x1+dx,y1+dy, x2+dx,y2+dy, makeacol32(nr,ng,nb,na), draw_beamline);
		else
			line(buf, x1+dx,y1+dy, x2+dx,y2+dy, makecol(nr,ng,nb));
    }
}


void draw_minimap()
{
    // Mini-map
    BITMAP *tmp = create_bitmap(150,150);
    clear_bitmap(tmp);
    masked_stretch_blit(level, tmp, 0,0, level->w,level->h,  0,0, 150,150);
    for(int y=0; y < 150; y++)
    {
        for(int x=0; x < 150; x++)
        {
            if( getpixel(tmp,x,y) != makecol(0,0,0) )
                putpixel(tmp,x,y, makecol( minf(255,bg_r*2), minf(255,bg_g*2), minf(255,bg_b*2)));
        }        
    }

    for(SPRITE *S = spritelist; S != NULL; S = S->next)
    {
		if(S->dead) continue;

        int col, rad=1;
        switch(S->type)
        {
			case SPRITE_PLAYER: col = makecol(255,255,255); break;
            case SPRITE_BULLET: col = makecol(96,96,96); break;
            case SPRITE_BLASTER: col = makecol(0,255,160); break;
            case SPRITE_MISSILE: col = makecol(255,255,0); break;
            case SPRITE_SCATTER_MISSILE: col = makecol(200,200,0); break;
            case SPRITE_ICE: col = makecol(0,128,255); break;
			case SPRITE_ENEMY: col = makecol(255,0,0); break;
			case SPRITE_ENEMY_BLASTER: col = makecol(0,255,255); break;
            default: col = 0; break;
        }
		if(S == boss)
			rad = 6;
		else if(S->type == SPRITE_ENEMY)
		{
			rad = S->enemydata.radius / 20;
			if( S->enemydata.splitcount <= 20 && S->enemydata.splitcount > 0 )
			{
				col = makecol(255,255,0);
			}
			if( S->enemydata.atecount != 0 )
			{
				col = makecol(255,255,255);
			}
		}
        if(col != 0)
            circlefill(tmp, (S->x * (150.0f/level->w)), (S->y * (150.0f/level->h)), rad, col);
		else if(S->type == SPRITE_BEAM)
		{
			line(tmp, S->beamdata.x1 * (150.0f / (float)level->w), S->beamdata.y1 * (150.0f / (float)level->h),
					S->beamdata.x2 * (150.0f / (float)level->w), S->beamdata.y2 * (150.0f / (float)level->h),
					makecol(180,0,0));
		}
    }
        
    int x = SCREEN_W-150-2;
	int y = 0;
	if(mapx == level->w-SCREEN_W)
	{
		x = 2;
		y = 32;
	}
	blit(tmp, backbuffer, 0,0, x,y, 150,150);
	destroy_bitmap(tmp);

	draw_outline(backbuffer, x-2,y-2,154,154, makecol(230,230,230));
	draw_outline(backbuffer, x-1,y-1,152,152, makecol(180,180,180));
	draw_outline(backbuffer, x,y,150,150, makecol(130,130,130));
}


void draw_hud()
{
	rectfill(backbuffer, 3,3, SCREEN_W-3,32-3, makecol(0,0,0));
	//gradientrect(backbuffer, 0,0, 640,32, makecol(100,100,100),makecol(32,32,32),0,0);
	//gradientrect(backbuffer, 0,480-32, 640,480, makecol(100,100,100),makecol(32,32,32),0,0);
	rectfill(backbuffer, 3,SCREEN_H-32+3, SCREEN_W-3,SCREEN_H-3, makecol(0,0,0));
    
    int col = makecol(255,255,255);
    char *name;
    
    switch(curweapon)
    {
        case WEAPON_BLASTER:
		{
            col = makecol(0,255,170);
            strcpy(name, "Blaster");
			if(player->playerdata.shiptype == SHIP_DOUBLEHEADER)
				strcpy(name, "2x Blasters");
			float per = 1 - (float)weapons[0].waitcount / (float)weapons[0].firerate;
			if(absf(weapons[0].waitcount-weapons[0].firerate) < 9)
			{
				per = 1 - (float)absf(weapons[0].waitcount-weapons[0].firerate) / 10.0f;
			}
			rectfill(backbuffer, 160,15, 144+per*83,19, makecol(0,255,0));
			for(int a=0; a < 8; a++)
			{
				rect(backbuffer, 160-a,15-a, 144+per*83+a,19+a, makecol(0, (1-(float)a/10.0f)*255, 0));
			}
			draw_sprite(backbuffer, battery_hud, 152,7);
            break;
		}
        case WEAPON_MACHINEGUNS:
		{
            col = makecol(120,120,120);
            strcpy(name, "MachineGuns");
			if(player->playerdata.shiptype == SHIP_JUGGERNAUT)
				strcpy(name, "Minigun");
			if(player->playerdata.shiptype == SHIP_DOUBLEHEADER)
				strcpy(name, "2x Miniguns");
			int clips = weapons[WEAPON_MACHINEGUNS].ammo / 50;
			int rounds = weapons[WEAPON_MACHINEGUNS].ammo % 50;
			if(weapons[WEAPON_MACHINEGUNS].ammo == weapons[WEAPON_MACHINEGUNS].maxammo)
				rounds = 50;
			for(int a=0; a < 50; a++)
			{
				int col = makecol(200,200,200);
				if(a > rounds)
					col = makecol(100,100,100);

				line(backbuffer, 152+2*a,10, 152+2*a,14, col);
				line(backbuffer, 152+2*a,17, 152+2*a,22, col);
			}
			textprintf_ex(backbuffer, font2, 255,5, makecol(170,170,170), -1, "x%d", clips);
            break;
		}
        case WEAPON_MISSILE:
		{
            col = makecol(225,225,0);
            strcpy(name, "Missiles");
			if(player->playerdata.shiptype == SHIP_DOUBLEHEADER)
				col = makecol(255,200,0);
			if(player->playerdata.shiptype == SHIP_JUGGERNAUT)
				col = makecol(200,255,0);
			int x = 120;
			int ammo = weapons[WEAPON_MISSILE].ammo;
			for(int a=0; a < 5; a++)
			{
				if(ammo > a*5)   rotate_sprite(backbuffer, missileimage, x,   13, itofix(-64));
				if(ammo > a*5+1) rotate_sprite(backbuffer, missileimage, x+4, 11, itofix(-64));
				if(ammo > a*5+2) rotate_sprite(backbuffer, missileimage, x+8, 9, itofix(-64));
				if(ammo > a*5+3) rotate_sprite(backbuffer, missileimage, x+12,11, itofix(-64));
				if(ammo > a*5+4) rotate_sprite(backbuffer, missileimage, x+16,13, itofix(-64));
				x += 40;
			}
            break;
		}
        case WEAPON_LASER:
		{
            col = makecol(255,0,0);
            strcpy(name, "Laser");
			if(player->playerdata.shiptype == SHIP_INDIGOAVENGER)
				col = makecol(255,0,255);
			float per = maxf(0.1f, (float)weapons[WEAPON_LASER].ammo / (float)weapons[WEAPON_LASER].maxammo);
			rectfill(backbuffer, 160,15, 144+per*83,19, makecol(255,0,0));
			for(int a=0; a < 8; a++)
			{
				rect(backbuffer, 160-a,15-a, 144+per*83+a,19+a, makecol((1-(float)a/10.0f)*255, 0, 0));
			}
			draw_sprite(backbuffer, battery_hud, 152,7);
            break;
		}
        case WEAPON_FREEZE_BOMB:
		{
            col = makecol(0,170,255);
            strcpy(name, "Freeze Bomb");
			if(player->playerdata.shiptype == SHIP_CRIMSONWING)
			{
				strcpy(name, "Fire Bomb");
				col = makecol(255,190,0);
			}
			int x = 153;
			int ammo = weapons[WEAPON_FREEZE_BOMB].ammo;
			for(int a=0; a < ammo; a++)
			{
				for(int b=10; b >= 0; b--)
				{
					int col = ((float)b / 10.0f) * 80;
					if(player->playerdata.shiptype == SHIP_CRIMSONWING)
						circlefill(backbuffer, x+fbomb_hud->w/2, fbomb_hud->h/2+6, b*1.5f, makecol(col,col*0.6,0));
					else
						circlefill(backbuffer, x+fbomb_hud->w/2, fbomb_hud->h/2+6, b*1.5f, makecol(0,col,col));
				}
				draw_sprite(backbuffer, fbomb_hud, x, 6);
				x += 32;
			}
            break;
		}
    }
    if(bonus_count > 0)
		col = makecol(random(256),random(256),random(256));
	textprintf_ex(backbuffer, font2, 32+10+8,8, col, -1, "%s", name);
    draw_sprite(backbuffer, icons[curweapon], 11,0);
	for(int a=0; a < 5; a++)
	{
		if(weapons[a].owned)
		{
			int col;
			switch(a)
			{
				case WEAPON_BLASTER: col = makecol(0,255,0); break;
				case WEAPON_MACHINEGUNS: col = makecol(160,160,160); break;
				case WEAPON_MISSILE: col = makecol(255,255,0); break;
				case WEAPON_LASER: col = makecol(255,0,0); break;
				case WEAPON_FREEZE_BOMB: col = makecol(0,255,255); break;
			}
			rectfill(backbuffer, 7,4+a*5, 10,4+a*5+3, col);
		}
	}
//    textprintf_ex(backbuffer, font2, 170,8, col, -1, "%d / %d", weapons[curweapon].ammo, weapons[curweapon].maxammo);
	textprintf_ex(backbuffer, font2, 330,8, makecol(170,255,170), -1, "Enemies Left: %d", enemy_count);

	//float lp = (1 - (lvl / total_lvl)) * 100;
	//textprintf_ex(backbuffer, font, 510,10, makecol(255,170,170), -1, "Body Left: %d%%", (int)lp);

	textprintf_ex(backbuffer, font2, 8,SCREEN_H-23, makecol(255,200,0), -1, "Ship Health");
	int width = (player_health * 1.5);
	col = makecol( player_health + 100, player_health/2+16, 0 );
	gradientrect(backbuffer, 120,SCREEN_H-26,120+width,SCREEN_H-6, makecol(150,32,0), col,col, makecol(100,16,0));
	//rectfill(backbuffer, 120,SCREEN_H-26,120+width,SCREEN_H-6, makecol(200,64,0));
	rect(backbuffer, 120,SCREEN_H-26,270,SCREEN_H-6, makecol(0,0,255));
	textprintf_ex(backbuffer, font2, 120+60,SCREEN_H-23, makecol(255,255,255), -1, "%d%%", (int)player_health);
//	textprintf_ex(backbuffer, font, 100,100, makecol(255,255,255), -1, "Collision Angles: %d", num_collisions);


	// Draw counter showing time survived and time left
	if(is_survival())
	{
		int mins = (survive_time / 1000) / 60;
		int secs = (survive_time / 1000) % 60;
		char info[15];
		if(secs < 10)
			sprintf(info, "%d:0%d", mins, secs);
		else
			sprintf(info, "%d:%d", mins, secs);
		textprintf_ex(backbuffer, font2, 410,SCREEN_H-24, makecol(255,255,255), -1, "%s", info);
		float per = ((float)(wave_survive_time/1000.0f*60) / (float)wave_duration) * 150;
		for(int a=0; a < per; a += 2)
		{
			if((gcount / 30) % 2 == 0 && wave_duration-(wave_survive_time/1000*60) <= 600)
			{
				line(backbuffer, 450+a,SCREEN_H-26, 450+a,SCREEN_H-6, makecol(160,0,0));
				line(backbuffer, 450+a,SCREEN_H-20, 450+a,SCREEN_H-12, makecol(200,0,0));
			}
			else
			{
				line(backbuffer, 450+a,SCREEN_H-26, 450+a,SCREEN_H-6, makecol(0,160,160));
				line(backbuffer, 450+a,SCREEN_H-20, 450+a,SCREEN_H-12, makecol(80,200,200));
			}
				line(backbuffer, 450+a,SCREEN_H-15, 450+a,SCREEN_H-17, makecol(255,255,255));
		}
		rect(backbuffer, 450, SCREEN_H-28, 600,SCREEN_H-4, makecol(255,0,0));
	}
	else
	{
		textprintf_ex(backbuffer, font2, 410,SCREEN_H-24, makecol(255,255,255), -1, "Lives: ");
		for(int a=0; a < minf(5,player->playerdata.lives); a++)
		{
			int y = SCREEN_H-29;
			if(player->playerdata.shiptype == 1) y++;
			if(player->playerdata.shiptype == 2) y++;
			if(player->playerdata.shiptype == 3) y += 1;
			if(player->playerdata.shiptype == 4) y--;
			if(player->playerdata.shiptype == 5) y -= 3;
			rotate_sprite(backbuffer, player->image, 465+34*a, y, itofix(-64));
		}
	}

	draw_outline(backbuffer, 2,2, SCREEN_W-4,32-4, makecol(230,230,230));
	draw_outline(backbuffer, 1,1, SCREEN_W-2,32-2, makecol(180,180,180));
	draw_outline(backbuffer, 0,0, SCREEN_W,32, makecol(130,130,130));	
	draw_outline(backbuffer, 2,SCREEN_H-32+2, SCREEN_W-4,32-4, makecol(230,230,230));
	draw_outline(backbuffer, 1,SCREEN_H-32+1, SCREEN_W-2,32-2, makecol(180,180,180));
	draw_outline(backbuffer, 0,SCREEN_H-32, SCREEN_W,32, makecol(130,130,130));
}


void start_level()
{
	if(is_survival())
		return;
	if(win_pause > 0 || game_state != STATE_GAME)
		return;
	if(win_pause <= 0 && game_state == STATE_GAME && cur_level == 4)
	{
		game_state = STATE_VICTORY;
		play_sample(fireworks_sound, config[CFG_SFX_VOLUME], 128, 1000, -1);
		FSOUND_Stream_Play(0, game_music[7]);
		return;
	}

	// On 'Impossible', player gets all weapons
	if(config[CFG_DIFFICULTY] == DIFF_IMPOSSIBLE)
	{
		for(int a=0; a < 5; a++)
			weapons[a].owned = true;
	}

	clear_level();
	cur_wave++;
	ld_count = 200;
	if(cur_wave > 4 || cur_level <= 0)
	{
		cur_wave = 1;
		cur_level++;
		player->playerdata.lives++;

		switch(cur_level)
		{
			case 0: cur_music = 0; break;
			case 1: cur_music = 2; break;
			case 2: cur_music = 1; break;
			case 3: cur_music = 3; break;
			case 4: cur_music = 4; break;
		}
		FSOUND_Stream_Play(0, game_music[cur_music]);

		destroy_bitmap(level_fg);
		destroy_bitmap(level_bg);

		if(cur_level == 1) { level_fg = load_bitmap("data/brain.bmp", NULL); level_bg = generate_bg(bg[random(3)], 0, 0, 70); }
		else if(cur_level == 2) { level_fg = load_bitmap("data/stomach.bmp", NULL); level_bg = generate_bg(bg[random(3)], 0, 70, 0); }
		else if(cur_level >= 3) { level_fg = load_bitmap("data/heart.bmp", NULL); level_bg = generate_bg(bg[random(3)], 70, 0, 0); }
		else  { level_fg = load_bitmap("data/stomach.bmp", NULL); level_bg = generate_bg(bg[random(3)], 70, 0, 0); }
		apply_texture_to_map(level_fg);
		generate_map(0);
		//rectfill(level, 0,0, level->w,level->h, makecol(255,0,255));
		//rectfill(level_shadow, 0,0, level->w,level->h, makecol(255,0,255));
		
		total_lvl = 4000*cur_level;

		for(int a=0; a < 5; a++)
			weapons[a].ammo = weapons[a].maxammo;
		player_health = 100;

		player->x = player->lastx = random(level->w-64)+32;
		player->y = player->lasty = random(level->h-64)+32;
		player->playerdata.alive = false;
		player->playerdata.rx = player->x;
		player->playerdata.ry = player->y;
		respawn_wait = 240;
		
		if(cur_level == 4)
		{
			// Do boss!
			create_boss(level->w/2, level->h/2);
			bonus_count = 0;
			return;
		}
	}

	// Award new weapons (if at certain level/wave)
	if(config[CFG_DIFFICULTY] != DIFF_IMPOSSIBLE)
	{
		int gun = -1;
		if(cur_level == 1 && cur_wave == 3)
			gun = 1;
		if(cur_level == 2 && cur_wave == 1)
			gun = 2;
		if(cur_level == 2 && cur_wave == 4)
			gun = 3;
		if(cur_level == 3 && cur_wave == 2)
			gun = 4;
		if(gun != -1)
		{
			bonus_count = 80;
			weapons[gun].owned = true;
			curweapon = gun;
			play_sound(bonus_sound, player->x, player->y);
		}
	}


	// Spawn enemies based on current wave/level, and difficulty
	int lvl = ((cur_level-1)*4) + cur_wave;
	int diff_quota = maxf(60, (lvl*40 + 20) * diff_mod());

	// On the 4th wave, spawn a sub-boss and build an 'arena'
	if(cur_wave == 4)
	{
		destroy_terrain(level->w/2,level->h/2,650);
		diff_quota = 0;
		SPRITE *E;
		if(cur_level == 1)
		{
			E = create_enemy(NULL, level->w/2-50, level->h/2, SPECIES_STALKER);
			E->enemydata.health *= diff_mod();
			E = create_enemy(NULL, level->w/2, level->h/2+50, SPECIES_STALKER);
			E->enemydata.health *= diff_mod();
		}
		if(cur_level == 2)
		{
			E = create_enemy(NULL, level->w/2-50, level->h/2, SPECIES_MECHA);
			E->enemydata.health *= diff_mod();
			E = create_enemy(NULL, level->w/2+50, level->h/2, SPECIES_MECHA);
			E->enemydata.health *= diff_mod();
		}
		if(cur_level == 3)
		{
			E = create_enemy(NULL, level->w/2, level->h/2-50, SPECIES_FREEZER);
			E->enemydata.health *= diff_mod();
			E = create_enemy(NULL, level->w/2, level->h/2+50, SPECIES_FREEZER);
			E->enemydata.health *= diff_mod();
		}
	}

	int swarms=0;
	while(diff_quota > 0)
	{
		if(random(50) == 32)
			diff_quota--;
		int i = random(MAX_SPECIES);
		int bat = species[i].batch_size;
		int num = maxf(1, random(bat*0.4f) - bat*0.2f + bat);
		if(species[i].min_level > lvl || species[i].difficulty*num > diff_quota)
			continue;

		int x = random(level->w-200)+100;
		int y = random(level->h-200)+100;
		if(swarms == 0 && cur_wave == 1)
		{
			x = player->x + random(400) - 200;
			y = player->y + random(400) - 200;
		}

		swarms++;
		if(swarms > cur_level*2)
			num = 1;
		for(int a=0; a < num; a++)
		{
			int distrib = species[i].radius * num;
			create_enemy(NULL, random(distrib*2)+x-distrib, random(distrib*2)+y-distrib, i);
		}

		diff_quota -= species[i].difficulty * num;
	}
}


void start_survival()
{
	cur_wave++;
	wave_survive_time = 0;
	ld_count = 200;
	if(cur_music != 5)
	{
		cur_music = 5;
		FSOUND_Stream_Play(0, game_music[cur_music]);
	}

	// Change background every 5 waves
	if(cur_wave % 5 == 0)
	{
		destroy_bitmap(level_bg);
		level_bg = generate_bg( bg[random(3)], random(256), random(256), random(256) );
	}

	// Refill 10% of ammo
	for(int a=0; a < 5; a++)
		weapons[a].ammo = minf(weapons[a].maxammo, weapons[a].ammo+(weapons[a].maxammo*0.1));
	// Award health based on clearance speed (minimum of 30hp refill)
	player_health = minf(100, player_health+30);

	// Initialization settings for first wave
	if(cur_wave == 1)
	{
		player->x = player->lastx = level->w/2;
		player->y = player->lasty = level->h/2;
		player->playerdata.alive = false;
		player->playerdata.rx = player->x;
		player->playerdata.ry = player->y;
		respawn_wait = 240;
		destroy_terrain(level->w/2,level->h/2,900);
	}
	
	// Spawn enemies based on current wave, and difficulty	
	spawndata.x = level->w/2 - 400 + random(800);
	spawndata.y = level->h/2 - 400 + random(800);
	spawndata.type = random(6)+1;
	switch(spawndata.type)
	{
		case SPAWN_SINEWAVE_HORZ:
		case SPAWN_SINEWAVE_VERT:
		{
			spawndata.amplitude = 200+random(250);
			spawndata.radius = random(500)+200;
			spawndata.wavelength = random(360*5)+360;
			break;
		}
		case SPAWN_CIRCLE:
		{
			spawndata.radius = random(600)+300;
			break;
		}
		case SPAWN_SPIRAL:
		{
			spawndata.wavelength = random(360*2)+360*1.5;
			spawndata.amplitude = random(250)+200;
			spawndata.radius = random(250)+300;
			break;
		}
		case SPAWN_LINE:
		{
			spawndata.amplitude = random(400)+400;
			spawndata.wavelength = random(360);
			break;
		}
		case SPAWN_VORTEX:
		{
			spawndata.amplitude = random(8)+2;
			break;
		}
	}

	// Prevent Freezer Cells from spawning (too hard/annoying for Survival)
	do
		spawndata.enemy_genre = random(MAX_SPECIES);
	while(spawndata.enemy_genre == SPECIES_FREEZER);

	// Set number of enemies and time limit for wave
	spawndata.enemycount = maxf(1, (50+cur_wave*30) / species[spawndata.enemy_genre].difficulty);
	spawndata.spawntime = spawndata.spawncount = random(300)+60;
	wave_duration = 60*60 - cur_wave*30;

	// Award new ships based on wave accomplished
	if(cur_wave == 7 && !shipdata[1].owned)
		create_ship_gain(2);
	if(cur_wave == 14 && !shipdata[2].owned)
		create_ship_gain(3);
	if(cur_wave == 21 && !shipdata[3].owned)
		create_ship_gain(4);
	if(cur_wave == 28 && !shipdata[4].owned)
		create_ship_gain(5);
}



void play_sound(SAMPLE *sfx, int x, int y)
{
	int compx = player->x + player->image->w/2;
	int compy = player->y + player->image->h/2;
	if(game_state == STATE_TITLE)
	{
		compx = mapx + SCREEN_W/2;
		compy = mapy + SCREEN_H/2;
	}

	// Too far?
	if( absf(x-compx) > SCREEN_W || absf(y-compy) > SCREEN_H )
		return;

	// TODO: Distance fading?
	play_sample(sfx, config[CFG_SFX_VOLUME], 128, 1000, 0);
}


void gradientrect(BITMAP *bmp, int x1, int y1, int x2, int y2, int c1, int c2, int c3, int c4)
{
	V3D_f *verts[4];
	
	verts[0] = new V3D_f();
	verts[0]->x = x1;
	verts[0]->y = y1;
	verts[0]->c = c1;

	verts[1] = new V3D_f();
	verts[1]->x = x2;
	verts[1]->y = y1;
	verts[1]->c = c2;

	verts[2] = new V3D_f();
	verts[2]->x = x2;
	verts[2]->y = y2;
	verts[2]->c = c3;

	verts[3] = new V3D_f();
	verts[3]->x = x1;
	verts[3]->y = y2;
	verts[3]->c = c4;

	quad3d_f(bmp, POLYTYPE_GCOL, NULL, verts[0], verts[1], verts[2], verts[3]);

    for(int b=0; b < 4; b++)
		delete verts[b];
}


void circle_alpha(BITMAP *bmp, int x, int y, int col)
{
	int r1 = getr32(col);
	int g1 = getg32(col);
	int b1 = getb32(col);
	int col2 = getpixel(backbuffer, x, y);
	int r2 = getr(col2);
	int g2 = getg(col2);
	int b2 = getb(col2);
	int fr = (r1+r2) * ((float)geta32(col) / 255.0f);
	int fg = (g1+g2) * ((float)geta32(col) / 255.0f);
	int fb = (b1+b2) * ((float)geta32(col) / 255.0f);
	putpixel(bmp, x, y, makecol(fr,fg,fb));
}

void putpixel_alpha(BITMAP *bmp, int x, int y, int col, int alpha)
{
	int r1 = getr(col);
	int g1 = getg(col);
	int b1 = getb(col);
	int col2 = getpixel(bmp, x, y);
	int r2 = getr(col2);
	int g2 = getg(col2);
	int b2 = getb(col2);
	float per = (float)alpha / 255.0f;
	int fr = (r1 * per) + (r2 * (1-per));
	int fg = (g1 * per) + (g2 * (1-per));
	int fb = (b1 * per) + (b2 * (1-per));
	putpixel(bmp, x, y, makecol(fr,fg,fb));
}

void putpixel_add(BITMAP *bmp, int x, int y, int col, int alpha)
{
	int r1 = getr(col);
	int g1 = getg(col);
	int b1 = getb(col);
	int col2 = getpixel(bmp, x, y);
	int r2 = getr(col2);
	int g2 = getg(col2);
	int b2 = getb(col2);
	int fr = minf(255.0f, (float)(r1+r2));
	int fg = minf(255.0f, (float)(g1+g2));
	int fb = minf(255.0f, (float)(b1+b2));
	putpixel_alpha(bmp, x,y, makecol(fr,fg,fb), alpha);
}


void action_newgame()
{
	cur_level = 0;
	cur_wave = 0;
	clear_level();
	game_state = STATE_GAME;
	player->playerdata.alive = false;
	player->playerdata.rx = player->x;
	player->playerdata.ry = player->y;
	player->playerdata.lives = 1;
	cur_menu=0;
	bonus_count = 0;
	win_pause = 0;
	screen_shake = 0;
	paused = false;
	curweapon = 0;
	
	if(is_survival())
	{
		for(int a=1; a < 5; a++)
		{
			weapons[a].owned = true;
			weapons[a].waitcount = 0;
			weapons[a].ammo = weapons[a].maxammo;
		}
		survive_time = 0;
		start_survival();
	}
	else
	{
		for(int a=1; a < 5; a++)
		{
			weapons[a].owned = false;
			weapons[a].waitcount = 0;
			weapons[a].ammo = weapons[a].maxammo;
		}
		start_level();
	}

	clear_gui();
	perform_fade(10, makecol(255,255,255));	
}

void action_exit()
{
	game_running = false;
}

void action_resume()
{
	last = timeGetTime();
	clear_gui();
	paused = false;
	cur_menu = 0;
}

void action_options()
{
	clear_gui();
	create_options_menu();
}

void action_returnmenu()
{
	// Force return to title if player is out of lives
	if(player->playerdata.lives < 0)
		game_state = STATE_TITLE;

	clear_gui();
	create_main_menu();
}

void action_credits()
{
	clear_gui();
	create_credits();
}

void action_shipselect()
{
	clear_gui();
	create_shipselect();
}

void create_main_menu()
{
	int y = 85;
    const int WIN_WIDTH = 350;//SCREEN_W * 0.73f;
    const int WIN_HEIGHT = SCREEN_H * 0.52f;

    int numButtons = 5;
	if(game_state == STATE_GAME) {
        numButtons++;
    }
	int BUTTON_SPACING = WIN_HEIGHT / numButtons - 3;

	add_window(SCREEN_W/2-WIN_WIDTH/2,y, WIN_WIDTH,WIN_HEIGHT);

    y += WIN_HEIGHT / 20;
	int button_x = SCREEN_W/2 - 200/2;
	if(game_state == STATE_GAME)
	{
		add_button(button_x,y+BUTTON_SPACING, 200,24, "Resume Game", ACTION_RESUME);
		y += BUTTON_SPACING;
	}
	add_button(button_x,y, 200,24, "New Game", ACTION_SHIPSELECT_STORY);
	add_button(button_x,y+BUTTON_SPACING, 200,24, "Survival Mode", ACTION_SHIPSELECT_SURVIVAL);
	//add_button(button_x,y+BUTTON_SPACING, 200,24, "Infinite Dungeon", 0)->disabled = true;
	add_button(button_x,y+BUTTON_SPACING*2, 200,24, "Game Options", ACTION_OPTIONS);
	add_button(button_x,y+BUTTON_SPACING*3, 200,24, "Credits", ACTION_CREDITS);
	add_button(button_x,y+BUTTON_SPACING*4, 200,24, "Exit", ACTION_EXIT);
	cur_menu = 1;

	add_window(10,SCREEN_H-55, SCREEN_W-20,45);
	info_label = add_label(SCREEN_W/2,SCREEN_H-35, "");
}


void create_options_menu()
{
	int y = 85;
    const int WIN_WIDTH = 540;
    const int WIN_HEIGHT = 385;
    int x = SCREEN_W/2 - WIN_WIDTH/2;
	add_window(x,y, WIN_WIDTH,WIN_HEIGHT);

    const int numButtons = 9;
    const int labelX = SCREEN_W/2 - 140;
    const int optionX = SCREEN_W/2 + 145;
	int s = (WIN_HEIGHT-20-60) / numButtons;

    y += 35;
	add_option(optionX,y, 150,15, "Normal", OPTION_DIFFICULTY);
	add_label(labelX,y, "Difficulty Level");
	
	add_option(optionX,y+s, 150,15, "Relative", OPTION_CONTROLSTYLE);
	add_label(labelX,y+s, "Ship Control Style");

	add_option(optionX,y+s*2, 150,15, "Enabled", OPTION_MOUSEWHEEL);
	add_label(labelX,y+s*2, "Mouse-Wheel Weapon Select");

	add_option(optionX,y+s*3, 150,15, "50%", OPTION_SOUNDVOLUME);
	add_label(labelX,y+s*3, "Sound Volume");
	
	add_option(optionX,y+s*4, 150,15, "50%", OPTION_MUSICVOLUME);
	add_label(labelX,y+s*4, "Music Volume");

	add_option(optionX,y+s*5, 150,15, "Fullscreen", OPTION_FULLSCREEN);
	add_label(labelX,y+s*5, "Window Mode");

	add_option(optionX,y+s*6, 150,15, "Enabled", OPTION_BLENDING);
	add_label(labelX,y+s*6, "Particle Blending");

	add_option(optionX,y+s*7, 150,15, "Enabled", OPTION_MEMBRANES);
	add_label(labelX,y+s*7, "Translucent Membranes");

	add_option(optionX,y+s*8, 150,15, "Enabled", OPTION_LIGHTFLASHES);
	add_label(labelX,y+s*8, "Light Flashes");

	add_button(SCREEN_W/2-220/2, y+s*9+12, 220,25, "Return to Main Menu", ACTION_RETURNMENU);

	cur_menu = 2;
}

void create_credits()
{
    const int WIN_WIDTH = 380;
    int x = SCREEN_W/2 - WIN_WIDTH/2;
	int y = 125;
	add_window(x,90, WIN_WIDTH,380);

    x = SCREEN_W/2;
	add_label(x, 110, "Credits", makecol(255,255,0));
	add_label(x, 118, "___________", makecol(200,200,0));
	add_label(x, 155, "Programming & Design", makecol(0,255,0));
	add_label(x, 180, "Stephen \"HopeDagger\" Whitmore");
	add_label(x, 225, "Music & Misc. Art", makecol(0,255,0));
	add_label(x, 250, "Dean \"Draffurd\" Yeats");
	add_label(x, 295, "Additional Art", makecol(0,255,0));
	add_label(x, 320, "Mark \"Prinz Eugn\" Simpson");
	add_label(x, 365, "Most Backgrounds & Sound Effects", makecol(0,255,0));
	add_label(x, 390, "Google Image Search & findsounds.com");

	add_button(SCREEN_W/2-220/2, 425, 220,25, "Return to Main Menu", ACTION_RETURNMENU);
	cur_menu = 3;
}


void create_shipselect()
{
	int y = 125;
    const int WIN_WIDTH = 580;
	add_window(SCREEN_W/2-WIN_WIDTH/2,90, WIN_WIDTH,380);

    int x = SCREEN_W/2;
	add_label(x, 110, "Ship Selection", makecol(255,255,0));
	add_label(x, 118, "_____________________", makecol(200,200,0));
	
	add_label(x-240, 170, "Thrust", makecol(255,255,255));
	add_label(x-65, 170, "Top Speed", makecol(255,255,255));
	add_label(x-220, 270, "Hull Strength", makecol(255,255,255));
	add_label(x-55, 270, "Ammo Regen.", makecol(255,255,255));

	ship_label = add_label(x+160, 170, "Ship Name", makecol(255,255,255));
	add_label(x+160, 176, "______________", makecol(200,200,200));

	start_button = add_button(x-85, 375, 160,25, "Start Game", ACTION_NEWGAME);
	add_option(x-5,435, 420,15, "Previous                   Next", OPTION_SHIPSELECT);
	cur_menu = 4;
}


void perform_fade(int speed, int col)
{
	fade_mode = FADE_OUT;
	fade_count = 0;
	fade_speed = speed;
	fade_colour = col;
}


void draw_number(int num, int x, int y, int r, int g, int b)
{
	char buf[100];
	sprintf(buf, "%d", num);
	//itoa(num, buf, 10);
	set_trans_blender(r,g,b,255);
	for(int a=0; a < strlen(buf); a++)
	{
		int n = buf[a] - 48;
		if(n >= 0 && n <= 9)
			draw_gouraud_sprite(backbuffer, number[n], x+(a*28), y, 255,255, 128,128);
	}
}

void draw_bigword(char type, int x, int y, int r, int g, int b)
{
	set_trans_blender(r,g,b,255);
	if(type == 'W')
		draw_gouraud_sprite(backbuffer, wave_txt, x, y, 255,255, 128,128);
	else if(type == 'L')
		draw_gouraud_sprite(backbuffer, level_txt, x, y, 255,255, 128,128);
	else if(type == 'F')
		draw_gouraud_sprite(backbuffer, final_txt, x, y, 255,255, 128,128);
}


float diff_mod()
{
	if(config[CFG_DIFFICULTY] == DIFF_EASY)
		return 0.8f;
	else if(config[CFG_DIFFICULTY] == DIFF_HARD)
		return 1.2f;
	else if(config[CFG_DIFFICULTY] == DIFF_IMPOSSIBLE)
		return 1.7f;
	else
		return 1.00f;
}


void draw_rating_bars(int x, int y, int rating)
{
	int space = 20;
	int width = 12;
	int height = 28;

	for(int a=0; a < 5; a++)
	{
		if(a >= rating)
		{
			rect(backbuffer, x+space*a, y, x+space*a+width, y+height, makecol(128,128,128));
			gradientrect(backbuffer, x+space*a+1, y+1, x+space*a+width, y+height, makecol(200,200,200), makecol(160,160,160), makecol(64,64,64), makecol(100,100,100));
		}
		if(a < rating)
		{
			gradientrect(backbuffer, x+space*a, y, x+space*a+width, y+height, makecol(200,0,0), makecol(160,70,0), makecol(32,200,0), makecol(60,200,0));

			// Add green glow to bar
			additiverect(x+space*a-1, y-1, x+space*a+width+1, y+height+1, makecol(0,255,0), 255);
			additiverect(x+space*a-2, y-2, x+space*a+width+2, y+height+2, makecol(0,255,0), 128);
			additiverect(x+space*a-3, y-3, x+space*a+width+3, y+height+3, makecol(0,255,0), 64);
			additiverect(x+space*a-4, y-4, x+space*a+width+4, y+height+4, makecol(0,255,0), 32);
			additiverect(x+space*a-5, y-5, x+space*a+width+5, y+height+5, makecol(0,255,0), 16);
		}
	}
}


void additiverect(int x1, int y1, int x2, int y2, int col, int alpha)
{
	for(int x=x1; x < x2; x++)
	{
		for(int y=y1; y < y2; y++)
		{
			if(x == x1 || y == y1 || x == x2-1 || y == y2-1)
				putpixel_add(backbuffer, x, y, col, alpha);
		}
	}
}

void ditherrect(int x, int y, int w, int h, int col)
{
	for(int yy=2; yy < h-2; yy++)
	{
		float bri = 1 - ((float)yy / (float)(h)) + 0.3f;
		int ccol = makecol( getr(col)*bri, getg(col)*bri, getb(col)*bri );

		for(int xx=2; xx < w-2; xx++)
		{
			if(xx % 2 == yy % 2 && xx+yy > 7)
			{
				putpixel(backbuffer, x+xx,y+yy, col);
			}
		}
	}
}


void update_survival_spawn()
{
	spawndata.spawncount--;
	if(spawndata.spawncount <= 0)
		return;

	int interval = maxf(1,spawndata.spawntime / spawndata.enemycount);
	int enemies_left = spawndata.spawncount / interval;
	if(spawndata.spawncount % interval != 0 && spawndata.spawncount != spawndata.spawntime-1)
		return;

	switch(spawndata.type)
	{
		case SPAWN_SINEWAVE_HORZ:
		{
			// Wavelength = Total angle-amount to cover
			// Amplitude = Sine wave amplitude
			// Radius = Width of sine wave
			float per = 1 - ((float)enemies_left / (float)spawndata.enemycount);
			float ang = (spawndata.wavelength * per) * (3.14159f / 180.0f);
			int x = spawndata.x - (spawndata.radius/2) + spawndata.radius * per;
			int y = spawndata.y + sin(ang) * spawndata.amplitude;

			SPRITE *E = create_enemy_pop(x, y, spawndata.enemy_genre);
			break;
		}
		case SPAWN_SINEWAVE_VERT:
		{
			// Wavelength = Total angle-amount to cover
			// Amplitude = Sine wave amplitude
			// Radius = Height of sine wave
			float per = 1 - ((float)enemies_left / (float)spawndata.enemycount);
			float ang = (spawndata.wavelength * per) * (3.14159f / 180.0f);
			int y = spawndata.x - (spawndata.radius/2) + spawndata.radius * per;
			int x = spawndata.y + sin(ang) * spawndata.amplitude;

			SPRITE *E = create_enemy_pop(x, y, spawndata.enemy_genre);
			break;
		}
		case SPAWN_CIRCLE:
		{
			// Radius = Radius of circle
			float ep = ((float)enemies_left / (float)spawndata.enemycount) * 360.0f;
			ep *= 3.14159f / 180.0f;
			SPRITE *E = create_enemy_pop(
				spawndata.x + cos(ep) * spawndata.radius,
				spawndata.y + sin(ep) * spawndata.radius,
				spawndata.enemy_genre);
			break;
		}
		case SPAWN_SPIRAL:
		{
			// Wavelength = Total angle-amount to cover
			// Amplitude = Total spiral size (not counting inner Radius)
			// Radius = Initial spiral radius
			float per = 1 - ((float)enemies_left / (float)spawndata.enemycount);
			float ep = spawndata.wavelength * per;
			ep *= 3.14159f / 180.0f;
			SPRITE *E = create_enemy_pop(
				spawndata.x + cos(ep) * spawndata.radius + cos(ep) * spawndata.amplitude*per,
				spawndata.y + sin(ep) * spawndata.radius + sin(ep) * spawndata.amplitude*per,
				spawndata.enemy_genre);
			break;
		}
		case SPAWN_LINE:
		{
			// Wavelength = Angle of line
			// Amplitude = Length of line
			float per = 1 - ((float)enemies_left / (float)spawndata.enemycount);
			float ang = spawndata.wavelength * (3.14159f / 180.0f);

			SPRITE *E = create_enemy_pop(
				spawndata.x + cos(ang) * spawndata.amplitude * per,
				spawndata.y + sin(ang) * spawndata.amplitude * per,
				spawndata.enemy_genre);
			break;
		}
		case SPAWN_VORTEX:
		{
			// Amplitude = Force exerted on spawned enemies
			SPRITE *E = create_enemy_pop(spawndata.x, spawndata.y, spawndata.enemy_genre);
			if(E)
			{
				float ang = random(360) * (3.14159f / 180.0f);
				E->xv = cos(ang) * spawndata.amplitude;
				E->yv = sin(ang) * spawndata.amplitude;
			}
			break;
		}
	}
}


SPRITE* create_enemy_pop(int x, int y, int genre)
{
	create_globs(4, x, y, 0,359, 1,4, 10, species[genre].r,species[genre].g,species[genre].b);
	if(random(100) > 50)
		play_sound(pop1_sound, x,y);
	else
		play_sound(pop2_sound, x,y);

	SPRITE *E = create_enemy(NULL, x, y, genre);
	if(E)
	{
		if(random(100) > 30 ||
			spawndata.enemy_genre == SPECIES_FREEZER ||
			spawndata.enemy_genre == SPECIES_STALKER ||
			spawndata.enemy_genre == SPECIES_MECHA)
			E->enemydata.mode = MODE_CHASE;
		else
			E->enemydata.mode = MODE_SHOOT;
		E->xv = E->yv = 0;
	}
	return E;
}


void create_ship_gain(int s)
{
	play_sound(bonus_sound, player->x, player->y);

	int y = 125;
	add_window(160,130, 320,295);

	add_label(320, 150, "You have unlocked a new ship!", makecol(255,255,0));
	add_label(320, 153, "_________________________________", makecol(255,255,0));
	add_label(320, 200, "The", makecol(255,255,255));
	char* buf;
	int col = makecol(255,255,255);
	if(s == 2) { strcpy(buf, "Crimson Wing"); col = makecol(255,64,64); }
	else if(s == 3) { strcpy(buf, "Indigo Avenger"); col = makecol(255,64,255); }
	else if(s == 4) { strcpy(buf, "Juggernaut"); col = makecol(32,170,32); }
	else if(s == 5) { strcpy(buf, "Doubleheader"); col = makecol(200,180,32); }
	else strcpy(buf, "Enigma Ship");
	add_label(320, 215, buf, col);
	add_label(320, 230, "is now under your command!", makecol(255,255,255));

	add_button(200, 350, 220,25, "Return to Main Menu", ACTION_RETURNMENU);
	add_button(200, 385, 220,25, "Keep on fighting!", ACTION_RESUME);
	cur_menu = 5;
	paused = true;
	new_ship = s;
	shipdata[s-1].owned = true;
}


BITMAP* generate_ship_portrait(int s, int rot, bool scanlines)
{
	BITMAP *img = ships[s-1];

	int size2 = (img->w * 3) / 2;
	BITMAP *tmp = create_bitmap(170,160);
	gradientrect(tmp, 0,0, 170,160, makecol(0,64,0), makecol(0,160,0), makecol(0,64,0), makecol(0,64,0));
	if(shipdata[s-1].owned)
		rotate_scaled_sprite(tmp, img, 85-size2, 80-size2, itofix(rcount), ftofix(3));
	else
	{
		BITMAP *tmp2 = create_bitmap(img->w, img->h);
		clear_to_color(tmp2, makecol(255,0,255));
		for(int y=0; y < tmp2->h; y++)
		{
			for(int x=0; x < tmp2->w; x++)
			{
				if(getpixel(img, x,y) != makecol(255,0,255))
					putpixel(tmp2,x,y,0);
			}
		}
		rotate_scaled_sprite(tmp, tmp2, 85-size2, 80-size2, itofix(rcount), ftofix(3));
		destroy_bitmap(tmp2);
	}
	// Simulate scanlines on the portrait for kicks :)
	if(scanlines)
	{
		for(int a=0; a < 160; a += 2)
			line(tmp, 0,a, 170,a, makecol(0,40,0));
	}

	return tmp;
}


void create_survival_rank()
{
	int mins = (survive_time / 1000) / 60;
	int secs = (survive_time / 1000) % 60;
	char* info = new char[15];
	if(secs < 10)
		sprintf(info, "%d:0%d", mins, secs);
	else
		sprintf(info, "%d:%d", mins, secs);

	play_sound(bonus_sound, player->x, player->y);

	int y = 125;
	add_window(160,130, 320,300);

	add_label(320, 150, "You have been destroyed!", makecol(255,255,0));
	add_label(320, 153, "____________________________", makecol(255,255,0));
	add_label(320, 200, "Your Surival time was:", makecol(255,255,255));
	add_label(320, 220, info, makecol(0,255,0));
	info = new char[40];
	sprintf(info, "You made it to Wave %d!", cur_wave);
	add_label(320, 260, info, makecol(0,255,255));
	add_label(320, 310, "You have attained the rank of:", makecol(255,255,255));
	int rank = maxf(0, minf(13, minf(42, cur_wave) / 3));
	add_label(320, 335, (char*)ranks[rank], makecol(0,255,0));

	add_button(190, 385, 265,25, "Return to Main Menu", ACTION_RETURNMENU);
	cur_menu = 6;
	paused = true;
	game_state = STATE_TITLE;
}


void create_game_over()
{
	play_sound(bonus_sound, player->x, player->y);

	int y = 125;
	add_window(160,130, 320,240);

	add_label(320, 150, "You have been destroyed!", makecol(255,255,0));
	add_label(320, 153, "____________________________", makecol(255,255,0));
	add_label(320, 200, "Game Over!", makecol(255,255,255));
	add_button(190, 265, 265,25, "Exit to Main Menu", ACTION_RETURNMENU);
	add_button(190, 300, 265,25, "Restart Level", ACTION_RESTART);
	cur_menu = 7;
	paused = true;
}


void action_restartlevel()
{
	cur_wave = 4;
	cur_level--;
	player->playerdata.lives = 2;
	clear_gui();
	paused = false;
	cur_menu = 0;
	start_level();
}


void clear_level()
{
	enemy_count = 0;

    for(SPRITE *S = particlelist; S != NULL; S = S->next)
    {
        S->lifetime = 0;
    }
    for(SPRITE *S = spritelist; S != NULL; S = S->next)
    {
        if(S->type != SPRITE_PLAYER)
			S->dead = true;
    }
}
