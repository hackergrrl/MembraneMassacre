#include <math.h>
#include "map.h"
#include "main.h"
#include "utils.h"
#include "sprites.h"

BITMAP *level_bg=0;
BITMAP *level_fg=0;
BITMAP *level=0;
BITMAP *level_shadow=0;
int mapx=0, mapy=0;
unsigned char bg_r, bg_g, bg_b;

const int shadow_dist = 7;



void draw_map()
{    
	blit(level_bg, backbuffer, (int)(mapx*0.6f)%level_bg->w/2,(int)(mapy*0.6f)%level_bg->h/2, 0,0, SCREEN_W,SCREEN_H);

	// Draw some nice dropshadows on the level :)
	masked_blit(level_shadow, backbuffer, mapx-shadow_dist,mapy-shadow_dist, 0,0, SCREEN_W,SCREEN_H);

	masked_blit(level, backbuffer, mapx,mapy, 0,0, SCREEN_W,SCREEN_H);


	/*
	int tile_size=24;
	for(int y=0; y < level->h/tile_size; y++)
	{
		for(int x=0; x < level->w/tile_size; x++)
		{
			int c1 = getpixel(level,x*tile_size+(tile_size/2),y*tile_size+(tile_size/2)) == makecol(255,0,255);
			int c2 = getpixel(level,x*tile_size,y*tile_size) == makecol(255,0,255);
			int c3 = getpixel(level,x*tile_size+(tile_size),y*tile_size) == makecol(255,0,255);
			int c4 = getpixel(level,x*tile_size+(tile_size),y*tile_size+(tile_size)) == makecol(255,0,255);
			int c5 = getpixel(level,x*tile_size,y*tile_size+(tile_size)) == makecol(255,0,255);
			int sum = c1+c2+c3+c4+c5;
			if(sum >= 3)
			{
//				rect(backbuffer, x*tile_size-mapx,y*tile_size-mapy, x*tile_size+tile_size-mapx,y*tile_size+tile_size-mapy, makecol(0,255,0));
			}
			else
			{
				rect(backbuffer, x*tile_size-mapx,y*tile_size-mapy, x*tile_size+tile_size-mapx,y*tile_size+tile_size-mapy, makecol(0,255,0));
//				rectfill(backbuffer, x*tile_size-mapx,y*tile_size-mapy, x*tile_size+tile_size-mapx,y*tile_size+tile_size-mapy, makecol(0,255,0));
			}
		}
	}
	*/
}


void generate_map(int m)
{
    switch(m)
    {
        case 0:
            generate_crappy_map();
            break;
    }
}


void generate_crappy_map()
{
	clear_bitmap(level_shadow);

    for(int b=0; b < random(30)+70; b++)
    {
        int x = random(level->w);
        int y = random(level->h);
        int ang = random(360);
        int base = random(40)+20;
        int fluc = base / 2;
        int angmod = random(10)-4;        
        for(int a=0; a < random(60)+10; a++)
        {
            int rad = base+random(fluc)-(fluc/2);
            destroy_terrain(x,y, rad);
            ang += angmod;
            float power = rad*0.75f;
            x += cos( ang * (3.14159/180.0f) ) * power;
            y += sin( ang * (3.14159/180.0f) ) * power;
        }
    }
}


void apply_texture_to_map(BITMAP *tex)
{
	int tw = tex->w;
	int th = tex->h;
    int nw = (level->w / tw)+1;
    int nh = (level->h / th)+1;
	    
    // Copy texture across map
	for(int y=0; y < nh; y++)
    {
        for(int x=0; x < nw; x++)
        {
            if(x % 2 == 1 && y % 2 == 1)
                draw_sprite_vh_flip(level, tex, x*tex->w,y*tex->h);
            else if(x % 2 == 1)
                draw_sprite_h_flip(level, tex, x*tex->w,y*tex->h);
            else if(y % 2 == 1)
                draw_sprite_v_flip(level, tex, x*tex->w,y*tex->h);
            else
                draw_sprite(level, tex, x*tex->w,y*tex->h);
        }
    }
}


BITMAP* generate_bg(BITMAP *img, unsigned char r, unsigned char g, unsigned char b)
{
    bg_r = r;
    bg_g = g;
    bg_b = b;
    
    BITMAP *bg2 = create_bitmap(img->w*2, img->h*2);
    BITMAP *bg = create_bitmap(img->w*2, img->h*2);
    draw_sprite(bg, img, 0,0);
    draw_sprite_h_flip(bg, img, img->w,0);
    draw_sprite_v_flip(bg, img, 0,img->h);
    draw_sprite_vh_flip(bg, img, img->w,img->h);
    
    set_trans_blender(r,g,b,255);
    draw_lit_sprite(bg2, bg, 0,0, 96);

	destroy_bitmap(bg);
    
    return bg2;
}



void destroy_terrain(int x, int y, int radius)
{
	circlefill(level,x,y,radius,makecol(255,0,255));
	circlefill(level_shadow,x,y,radius,makecol(255,0,255));
}