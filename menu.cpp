#include <math.h>
#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include "menu.h"
#include "main.h"
#include "utils.h"
#include "player.h"

GUI *gui;
int mouse_click = 0;
GUI *info_label = NULL;
GUI *last_hover = NULL;

void update_option(GUI *O);


void update_mouse_hover(GUI *G)
{
	if(last_hover != G && G != NULL)
	{
		play_sample(highlight_sound, config[CFG_SFX_VOLUME], 128, 1000, 0);
	}
	last_hover = G;

	if(!info_label)
		return;
	if(!G)
		strcpy(info_label->text, "Membrane Massacre v2.0  Copyright (c) 2006-2007");
	else if(!strcmp(G->text,"Resume Game"))
		strcpy(info_label->text, "Continue your journey onward!");
	else if(!strcmp(G->text,"New Game"))
		strcpy(info_label->text, "Begin a new game in 'story' mode");
	else if(!strcmp(G->text,"Survival Mode"))
		strcpy(info_label->text, "Survive against hordes of cells to gain new ships and more..");
	else if(!strcmp(G->text,"Game Options"))
		strcpy(info_label->text, "Configure controls, graphics settings, and difficulty");
	else if(!strcmp(G->text,"Credits"))
		strcpy(info_label->text, "The fine folks who made Membrane Massacre possible");
	else if(!strcmp(G->text,"Exit"))
		strcpy(info_label->text, "Don't even THINK about it!");
}


void draw_outline(BITMAP *bmp, int x, int y, int w, int h, int col)
{
	line(bmp, x+5,y, x+w-5,y, col);
	line(bmp, x+w,y+5, x+w,y+h-5, col);
	line(bmp, x+5,y+h, x+w-5,y+h, col);
	line(bmp, x,y+5, x,y+h-5, col);
	
	line(bmp, x,y+5, x+5,y, col);
	line(bmp, x+w,y+5, x+w-5,y, col);
	line(bmp, x,y+h-5, x+5,y+h, col);
	line(bmp, x+w,y+h-5, x+w-5,y+h, col);
}


void draw_window(BITMAP *bmp, int x, int y, int w, int h, int col)
{
	for(int yy=2; yy < h-2; yy++)
	{
		float bri = 1 - ((float)yy / (float)(h)) + 0.3f;
		int ccol = makecol( getr(col)*bri, getg(col)*bri, getb(col)*bri );

		for(int xx=2; xx < w-2; xx++)
		{
			if(xx % 2 == yy % 2 && xx+yy > 7)
			{
				putpixel(bmp, x+xx,y+yy, ccol);
			}
		}
	}

	draw_outline(bmp, x-1,y-1, w,h, makecol(230,230,230));
	draw_outline(bmp, x,y, w,h, makecol(200,200,200));
	draw_outline(bmp, x+1,y+1, w,h, makecol(128,128,128));
}


void draw_button(BITMAP *bmp, int x, int y, int w, int h, int col, const char *txt, bool selected, bool disabled)
{
	int col1 = makecol( minf(255,getr(col)*1.3f), minf(255,getg(col)*1.3f), minf(255,getb(col)*1.3f) );
	int col2 = makecol( getr(col)*0.7f, getg(col)*0.7f, getb(col)*0.7f );
	int col3 = makecol( minf(255,getr(col)*1.7f), minf(255,getg(col)*1.7f), minf(255,getb(col)*1.7f) );

	if(disabled)
	{
		col1 = makecol(130,130,130);
		col2 = makecol(50,50,50);
		col3 = makecol(170,170,170);
		col = makecol(100,100,100);
	}
	else if(selected)
	{
		rect(bmp, x-2,y-2, x+w+3,y+h+3, makecol(210,210,0));
		rect(bmp, x-1,y-1, x+w+2,y+h+2, makecol(255,255,0));
		col3 = makecol(255,255,255);
	}
	
	rectfill(bmp, x,y, x+w,y+h, col);
	line(bmp, x,y, x+w,y, col1);
	line(bmp, x+w,y, x+w,y+h, col1);
	line(bmp, x,y+1, x+w,y+1, col1);
	line(bmp, x+w+1,y, x+w+1,y+h, col1);	
	
	line(bmp, x,y+h, x+w,y+h, col2);
	line(bmp, x,y, x,y+h, col2);	
	line(bmp, x,y+h+1, x+w,y+h+1, col2);
	line(bmp, x+1,y, x+1,y+h, col2);

	textout_ex(bmp, font2, txt, x + (w/2) - (text_length(font2,txt)/2), y + (h/2) - (text_height(font2)/2), col3, -1);
}


void draw_guis(BITMAP *bmp, int col)
{
	if(mouse_b & 1)
	{
		if(!mouse_click)
			mouse_click = 1;
		else
			mouse_click = 2;
	}
	else
		mouse_click = 0;


	GUI *hover = NULL;
	GUI *G = gui;
	while(G != NULL)
	{
		switch(G->type)
		{
			case GUI_BUTTON:
			{
				bool sel = false;

				if(mouse_x >= G->x && mouse_x <= G->x+G->width && mouse_y >= G->y && mouse_y <= G->y+G->height)
				{
					sel = true;

					if(!G->disabled)
						hover = G;
				}

				draw_button(bmp, G->x,G->y, G->width,G->height, col, G->text, sel, G->disabled);

				if(sel && mouse_click == 1)
					click_button(G);

				break;
			}

			case GUI_OPTION:
			{
				update_option(G);
				int txtw = G->width;
				int txth = G->height;
				int startx = G->x - (G->width/2);
				int starty = G->y - (G->height/2);
				draw_button(bmp, startx,starty, txtw,txth+9, col, G->text, false, false);
				break;
			}


			case GUI_LABEL:
			{
				int w = text_length(font2,G->text);
				int h = text_height(font2);
				textout_ex(bmp, font2, G->text, G->x - (w/2), G->y - (h/2), G->col, -1);
				break;
			}

			case GUI_WINDOW:
			{
				draw_window(bmp, G->x,G->y, G->width,G->height, col);
				break;
			}
		}

		G = G->next;
	}
	update_mouse_hover(hover);
}


GUI *add_button(int x, int y, int w, int h, const char *txt, int act)
{
	GUI *B = new GUI();
	B->type = GUI_BUTTON;
	B->action = act;
	B->x = x;
	B->y = y;
	B->width = w;
	B->height = h;
    B->text[0] = '\0';
    strcpy(B->text, txt);
	
	GUI *G = gui;
	while(G != NULL)
	{
		if(G->next == NULL)
		{
			G->next = B;
			B->next = NULL;
			return B;
		}
		G = G->next;
	}
}


GUI *add_option(int x, int y, int w, int h, const char *txt, int act)
{
	GUI *B = new GUI();
	B->type = GUI_OPTION;
	B->action = act;
	B->x = x;
	B->y = y;
	B->width = w;
	B->height = h;
    B->text[0] = '\0';
    strcpy(B->text, txt);
	
	GUI *G = gui;
	while(G != NULL)
	{
		if(G->next == NULL)
		{
			G->next = B;
			B->next = NULL;
			break;
		}
		G = G->next;
	}

	int startx = x - (w/2);
	int starty = y - (h/2);
	int bw = w * 0.12;
	add_button(startx-bw-2,starty, bw,9+h, "<", act);
	add_button(startx+w+2,starty, bw,9+h, ">", act);

	return B;
}


GUI *add_window(int x, int y, int w, int h)
{
	GUI *B = new GUI();
	B->type = GUI_WINDOW;
	B->x = x;
	B->y = y;
	B->width = w;
	B->height = h;
	
	B->next = gui;
	gui = B;
	return B;
}

GUI *add_label(int x, int y, const char *txt, int col)
{
	GUI *B = new GUI();
	B->type = GUI_LABEL;
	B->x = x;
	B->y = y;
    B->text[0] = '\0';
    strcpy(B->text, txt);
	B->col = col;
	
	GUI *G = gui;
	while(G != NULL)
	{
		if(G->next == NULL)
		{
			G->next = B;
			B->next = NULL;
			return B;
		}
		G = G->next;
	}
}

void remove_gui(GUI *G)
{
	GUI *S = gui;

	while(S != NULL)
	{
		if(S->next == G)
		{
			S->next = G->next;
			delete G;
			return;
		}

		G = G->next;
	}
}

void click_button(GUI *B)
{
	if(B->disabled)
		return;

	switch(B->action)
	{
		case ACTION_NEWGAME:
		{
			action_newgame();
			break;
		}
		case ACTION_EXIT:
		{
			action_exit();
			break;
		}
		case ACTION_RESUME:
		{
			action_resume();
			break;
		}
		case ACTION_OPTIONS:
		{
			action_options();
			break;
		}
		case ACTION_RETURNMENU:
		{
			action_returnmenu();
			break;
		}
		case ACTION_CREDITS:
		{
			action_credits();
			break;
		}
		case ACTION_SHIPSELECT_STORY:
		{
			game_mode = MODE_STORY;
			action_shipselect();
			break;
		}
		case ACTION_SHIPSELECT_SURVIVAL:
		{
			game_mode = MODE_SURVIVAL;
			action_shipselect();
			break;
		}
		case ACTION_RESTART:
		{
			action_restartlevel();
			break;
		}

		case OPTION_DIFFICULTY:
		{
			if(!strcmp(B->text, ">"))
			{
				config[CFG_DIFFICULTY]++;
				if(config[CFG_DIFFICULTY] > DIFF_IMPOSSIBLE)
					config[CFG_DIFFICULTY] = DIFF_EASY;
			}
			else
			{
				config[CFG_DIFFICULTY]--;
				if(config[CFG_DIFFICULTY] < DIFF_EASY)
					config[CFG_DIFFICULTY] = DIFF_IMPOSSIBLE;
			}
			break;
		}
		case OPTION_CONTROLSTYLE:
		{
			config[CFG_ABSOLUTE_CONTROL] = !config[CFG_ABSOLUTE_CONTROL];
			break;
		}
		case OPTION_MOUSEWHEEL:
		{
			config[CFG_MOUSE_WHEEL] = !config[CFG_MOUSE_WHEEL];
			break;
		}
		case OPTION_SOUNDVOLUME:
		{
			if(!strcmp(B->text, ">"))
			{
				if(config[CFG_SFX_VOLUME] < 250)
					config[CFG_SFX_VOLUME] += 25;
			}
			else
			{
				if(config[CFG_SFX_VOLUME] > 0)
					config[CFG_SFX_VOLUME] -= 25;
			}
			break;
		}
		case OPTION_MUSICVOLUME:
		{
			if(!strcmp(B->text, ">"))
			{
				if(config[CFG_MUSIC_VOLUME] < 250)
					config[CFG_MUSIC_VOLUME] += 25;
			}
			else
			{
				if(config[CFG_MUSIC_VOLUME] > 0)
					config[CFG_MUSIC_VOLUME] -= 25;
			}
			FSOUND_SetSFXMasterVolume( config[CFG_MUSIC_VOLUME]);
			break;
		}
		case OPTION_FULLSCREEN:
		{
			config[CFG_FULLSCREEN] = !config[CFG_FULLSCREEN];
			break;
		}
		case OPTION_BLENDING:
		{
			config[CFG_PARTICLE_BLENDING] = !config[CFG_PARTICLE_BLENDING];
			break;
		}
		case OPTION_MEMBRANES:
		{
			config[CFG_SHOW_MEMBRANES] = !config[CFG_SHOW_MEMBRANES];
			break;
		}
		case OPTION_LIGHTFLASHES:
		{
			config[CFG_LIGHT_FLASHES] = !config[CFG_LIGHT_FLASHES];
			break;
		}
		case OPTION_SHIPSELECT:
		{
			if(!strcmp(B->text, ">"))
			{
				player->playerdata.shiptype++;
				if(player->playerdata.shiptype > 5)
					player->playerdata.shiptype = 1;
				set_ship_type(player->playerdata.shiptype);
			}
			else
			{
				player->playerdata.shiptype--;
				if(player->playerdata.shiptype < 1)
					player->playerdata.shiptype = 5;
				set_ship_type(player->playerdata.shiptype);
			}
			break;
		}
		default:
			return;
	}
	play_sample(select_sound, config[CFG_SFX_VOLUME], 128, 1000, 0);
}


void update_option(GUI *O)
{
	if(O->action == OPTION_DIFFICULTY)
	{
		if(config[CFG_DIFFICULTY] == DIFF_EASY)			strcpy(O->text, "Easy");
		if(config[CFG_DIFFICULTY] == DIFF_NORMAL)		strcpy(O->text, "Normal");
		if(config[CFG_DIFFICULTY] == DIFF_HARD)			strcpy(O->text, "Hard");
		if(config[CFG_DIFFICULTY] == DIFF_IMPOSSIBLE)	strcpy(O->text, "Impossible");
	}
	else if(O->action == OPTION_MOUSEWHEEL)
	{
		if(config[CFG_MOUSE_WHEEL] == 0)	strcpy(O->text, "Disabled");
		if(config[CFG_MOUSE_WHEEL] == 1)	strcpy(O->text, "Enabled");
	}
	else if(O->action == OPTION_CONTROLSTYLE)
	{
		if(config[CFG_ABSOLUTE_CONTROL] == 0)	strcpy(O->text, "Relative");
		if(config[CFG_ABSOLUTE_CONTROL] == 1)	strcpy(O->text, "Absolute");
	}
	else if(O->action == OPTION_SOUNDVOLUME)
	{
		int vol = (float(config[CFG_SFX_VOLUME]) / 250.0f) * 100.0f;
		switch(vol)
		{
			case 0: strcpy(O->text, "0%"); break;
			case 10: strcpy(O->text, "10%"); break;
			case 20: strcpy(O->text, "20%"); break;
			case 30: strcpy(O->text, "30%"); break;
			case 40: strcpy(O->text, "40%"); break;
			case 50: strcpy(O->text, "50%"); break;
			case 60: strcpy(O->text, "60%"); break;
			case 70: strcpy(O->text, "70%"); break;
			case 80: strcpy(O->text, "80%"); break;
			case 90: strcpy(O->text, "90%"); break;
			case 100: strcpy(O->text, "100%"); break;
		}
	}
	else if(O->action == OPTION_MUSICVOLUME)
	{
		int vol = (float(config[CFG_MUSIC_VOLUME]) / 250.0f) * 100.0f;
		switch(vol)
		{
			case 0: strcpy(O->text, "0%"); break;
			case 10: strcpy(O->text, "10%"); break;
			case 20: strcpy(O->text, "20%"); break;
			case 30: strcpy(O->text, "30%"); break;
			case 40: strcpy(O->text, "40%"); break;
			case 50: strcpy(O->text, "50%"); break;
			case 60: strcpy(O->text, "60%"); break;
			case 70: strcpy(O->text, "70%"); break;
			case 80: strcpy(O->text, "80%"); break;
			case 90: strcpy(O->text, "90%"); break;
			case 100: strcpy(O->text, "100%"); break;
		}
	}
	else if(O->action == OPTION_FULLSCREEN)
	{
		if(config[CFG_FULLSCREEN])
			strcpy(O->text, "Fullscreen");
		else
			strcpy(O->text, "Windowed");
	}
	else if(O->action == OPTION_BLENDING)
	{
		if(config[CFG_PARTICLE_BLENDING])
			strcpy(O->text, "Alpha Blending");
		else
			strcpy(O->text, "Solid Fill");
	}
	else if(O->action == OPTION_MEMBRANES)
	{
		if(config[CFG_SHOW_MEMBRANES])
			strcpy(O->text, "Enabled");
		else
			strcpy(O->text, "Disabled");
	}
	else if(O->action == OPTION_LIGHTFLASHES)
	{
		if(config[CFG_LIGHT_FLASHES] == 0)	strcpy(O->text, "Disabled");
		if(config[CFG_LIGHT_FLASHES] == 1)	strcpy(O->text, "Enabled");
	}
}



void remove_self_and_next(GUI *G)
{
	if(!G)
		return;
	GUI *N = G->next;
	//delete G;
	G = NULL;
	if(N)
		remove_self_and_next(N);
}

void clear_gui()
{
	gui = NULL;
	return;
	GUI *G = gui;
	gui = NULL;
	info_label = NULL;
	remove_self_and_next(G);
	return;
	GUI *S = gui;
	GUI *tmp = gui;
	int its=0;

	info_label = NULL;

	while(S != NULL)
	{
		S = S->next;
		delete tmp;
		tmp = S;
		its++;
	}
	gui = NULL;
}





void default_config()
{
	config[CFG_ABSOLUTE_CONTROL]	= false;
	config[CFG_MOUSE_WHEEL]			= true;
	config[CFG_PARTICLE_BLENDING]	= true;
	config[CFG_SFX_VOLUME]			= 128;
	config[CFG_MUSIC_VOLUME]		= 128;
	config[CFG_FULLSCREEN]			= true;
	config[CFG_COLOUR_DEPTH]		= 16;
	config[CFG_SHOW_MEMBRANES]		= true;
	config[CFG_DIFFICULTY]			= DIFF_NORMAL;
	config[CFG_LIGHT_FLASHES]		= true;
}


int cfg_from_string(const char *val)
{
	if(!strcmp(val,"AbsoluteControl")) return CFG_ABSOLUTE_CONTROL;
	if(!strcmp(val,"UseMouseWheel")) return CFG_MOUSE_WHEEL;
	if(!strcmp(val,"SfxVolume")) return CFG_SFX_VOLUME;
	if(!strcmp(val,"MusicVolume")) return CFG_MUSIC_VOLUME;
	if(!strcmp(val,"Fullscreen")) return CFG_FULLSCREEN;
	if(!strcmp(val,"ColourDepth")) return CFG_COLOUR_DEPTH;
	if(!strcmp(val,"ParticleBlending")) return CFG_PARTICLE_BLENDING;
	if(!strcmp(val,"ShowMembranes")) return CFG_SHOW_MEMBRANES;
	if(!strcmp(val,"Difficulty")) return CFG_DIFFICULTY;
	if(!strcmp(val,"LightFlashes")) return CFG_LIGHT_FLASHES;

	return -1;
}

const char *string_from_cfg(int val)
{
	if(val == CFG_ABSOLUTE_CONTROL) return "AbsoluteControl";
	if(val == CFG_MOUSE_WHEEL) return "UseMouseWheel";
	if(val == CFG_SFX_VOLUME) return "SfxVolume";
	if(val == CFG_MUSIC_VOLUME) return "MusicVolume";
	if(val == CFG_FULLSCREEN) return "Fullscreen";
	if(val == CFG_COLOUR_DEPTH) return "ColourDepth";
	if(val == CFG_PARTICLE_BLENDING) return "ParticleBlending";
	if(val == CFG_SHOW_MEMBRANES) return "ShowMembranes";
	if(val == CFG_DIFFICULTY) return "Difficulty";
	if(val == CFG_LIGHT_FLASHES) return "LightFlashes";

	return "";
}

void itoa(int num, char * str)
{
	sprintf(str, "%d", num);
}

void output_cfg(FILE *file, int val)
{
	char out[256];
	char tmp[256];
	out[0] = 0;
	tmp[0] = 0;
	strcat(out, string_from_cfg(val));
	strcat(out, "=");
	itoa(config[val], tmp);
	strcat(out, tmp);
	strcat(out, "\n");
	fputs(out, file);
}


bool load_config()
{
	default_config();

	FILE *file = fopen("settings.cfg", "r");
	if(!file)
		return false;

	bool can_read = true;
	while(can_read)
	{
		char data[256];
		char *check = fgets(data, 256, file);

		if(check == NULL)
			can_read = false;
		else if(data[0] == '#')
			continue;
		else if(data[0] < 33)
			continue;
		else
		{
			char attr[256];
			char val[16];
			int nval=-1;
			for(int a=0; a < strlen(data); a++)
			{
				if(data[a] == '=')
				{
					int pos=0;
					for(int b=a+1; b < strlen(data); b++)
					{
						val[pos] = data[b];
						val[pos+1] = 0;
						pos++;
					}
					nval = atoi(val);
					break;
				}
				else if(data[a] > 32 && data[a] < 127)
				{
					attr[a] = data[a];
					attr[a+1] = 0;
				}
			}

			int cfg = cfg_from_string(attr);
			if(cfg != -1)
				config[cfg] = nval;
			if(cfg == CFG_COLOUR_DEPTH && nval != 16 && nval != 24 && nval != 32)
			{
				allegro_message("Erk, invalid colour depth. 16, 24, or 32, please. Setting to 16.");
				config[CFG_COLOUR_DEPTH] = 16;
				return false;
			}
		}
	}

	fclose(file);


	file = fopen("shipdata.cfg", "r");
	if(!file)
		return false;

	int s=0;
	int upto=1;
	fscanf(file, "Ships=%d", &s);
	if(s == 31415)
		upto = 5;
	else if(s == 92653)
		upto = 4;
	else if(s == 58979)
		upto = 3;
	else if(s == 32348)
		upto = 2;
	else
		upto = 1;
	
	fclose(file);

	for(int a=0; a < upto; a++)
		shipdata[a].owned = true;

	return true;
}


bool save_config()
{
	FILE *file = fopen("settings.cfg", "w");
	if(!file)
		return false;

	fputs("###########################################################################\n", file);
	fputs("# Configuration values.\n", file);
	fputs("# DO NOT CHANGE UNLESS YOU KNOW WHAT YOU'RE DOING!\n", file);
	fputs("###########################################################################\n", file);
	fputs("\n# Controls\n", file);
	output_cfg(file, CFG_DIFFICULTY);
	output_cfg(file, CFG_ABSOLUTE_CONTROL);
	output_cfg(file, CFG_MOUSE_WHEEL);

	fputs("\n# Audio\n", file);
	output_cfg(file, CFG_SFX_VOLUME);
	output_cfg(file, CFG_MUSIC_VOLUME);

	fputs("\n# Video\n", file);
	output_cfg(file, CFG_FULLSCREEN);
	output_cfg(file, CFG_COLOUR_DEPTH);
	output_cfg(file, CFG_PARTICLE_BLENDING);
	output_cfg(file, CFG_SHOW_MEMBRANES);
	output_cfg(file, CFG_LIGHT_FLASHES);
	
	fclose(file);

	
	file = fopen("shipdata.cfg", "w");
	if(!file)
		return false;

	if(shipdata[4].owned)
		fprintf(file, "Ships=%d", 31415);
	else if(shipdata[3].owned)
		fprintf(file, "Ships=%d", 92653);
	else if(shipdata[2].owned)
		fprintf(file, "Ships=%d", 58979);
	else if(shipdata[1].owned)
		fprintf(file, "Ships=%d", 32348);
	else
		fprintf(file, "Ships=%d", 46264);
	
	fclose(file);

	return true;
}
