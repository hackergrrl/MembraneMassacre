#ifndef _MAP_H_
#define _MAP_H_

#include <allegro.h>

extern BITMAP *level_bg;
extern BITMAP *level_fg;
extern BITMAP *level;
extern BITMAP *level_shadow;
extern int mapx;
extern int mapy;
extern unsigned char bg_r, bg_g, bg_b;

void generate_map(int m);
void generate_crappy_map();
void draw_map();
void apply_texture_to_map(BITMAP *tex);
BITMAP* generate_bg(BITMAP *img, unsigned char r, unsigned char g, unsigned char b);
void destroy_terrain(int x, int y, int radius);


#endif
