#pragma once

// Configuration options
#define CFG_ABSOLUTE_CONTROL	0
#define CFG_MOUSE_WHEEL			1
#define CFG_PARTICLE_BLENDING	2
#define CFG_SFX_VOLUME			3
#define CFG_MUSIC_VOLUME		4
#define CFG_FULLSCREEN			5
#define CFG_COLOUR_DEPTH		6
#define CFG_SHOW_MEMBRANES		7
#define CFG_DIFFICULTY			8
#define CFG_LIGHT_FLASHES		9
#define MAX_CONFIG				10

#define GUI_WINDOW			1
#define GUI_BUTTON			2
#define GUI_OPTION			3
#define GUI_LABEL			4

#define ACTION_NEWGAME		1
#define ACTION_OPTIONS		2
#define ACTION_EXIT			3
#define ACTION_RESUME		4
#define OPTION_DIFFICULTY	5
#define OPTION_CONTROLSTYLE	6
#define OPTION_MOUSEWHEEL	7
#define OPTION_SOUNDVOLUME	8
#define OPTION_MUSICVOLUME	9
#define OPTION_FULLSCREEN	10
#define OPTION_BLENDING		11
#define OPTION_MEMBRANES	12
#define ACTION_RETURNMENU	13
#define OPTION_LIGHTFLASHES	14
#define ACTION_CREDITS		15
#define ACTION_SHIPSELECT_STORY		16
#define ACTION_SHIPSELECT_SURVIVAL	17
#define OPTION_SHIPSELECT	18
#define ACTION_RESTART		19


struct GUI
{
	bool disabled;
	int type;
	int action;
	int x, y;
	int width, height;
	char text[256];
	int col;

	GUI *next;
};


void draw_window(BITMAP *bmp, int x, int y, int w, int h, int col);
void draw_button(BITMAP *bmp, int x, int y, int w, int h, int col, const char *txt, bool selected, bool disabled);
void draw_guis(BITMAP *bmp, int col);
GUI *add_window(int x, int y, int w, int h);
GUI *add_button(int x, int y, int w, int h, const char *txt, int act);
GUI *add_label(int x, int y, const char *txt, int col=makecol(255,255,255));
GUI *add_option(int x, int y, int w, int h, const char *txt, int act);
void update_label(GUI *label, const char *txt);
void click_button(GUI *B);
void remove_gui(GUI *B);
void clear_gui();
void draw_outline(BITMAP *bmp, int x, int y, int w, int h, int col);

void default_config();
int cfg_from_string(const char *val);
bool load_config();
bool save_config();

extern int config[MAX_CONFIG];
extern GUI *gui;
extern GUI *info_label;
