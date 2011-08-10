#ifndef _SPRITES_H_
#define _SPRITES_H_

#include <allegro.h>

#define SPRITE_PLAYER			1
#define SPRITE_PARTICLE			2
#define SPRITE_BULLET			3
#define SPRITE_BLASTER			4
#define SPRITE_MISSILE			5
#define SPRITE_ICE				6
#define SPRITE_ICE_ROOT			7
#define SPRITE_ENEMY			8
#define SPRITE_GLOB				9
#define SPRITE_ENEMY_BLASTER	10
#define SPRITE_BEAM				11
#define SPRITE_EXPLOSION		12
#define SPRITE_SMOKE			13
#define SPRITE_LIGHT_FLASH		14
#define SPRITE_BUBBLE			15
#define SPRITE_SCATTER_MISSILE	16

#define BLEND_NORMAL		0
#define BLEND_ALPHA			1
#define BLEND_ADD			2


struct BEAMDATA
{
	int x1, y1, x2, y2;
	int lifetime;
};

struct PLAYERDATA
{
	float rx, ry;
	bool alive;
	int shiptype;
	float ramming_power;
	int lives;
};

struct PARTICLEDATA
{
    int start_r,start_g,start_b,start_a,  end_r,end_g,end_b,end_a;
	int blendmode;
    int maxlifetime;
};


struct CELLDATA
{
	float loc;
	float vel;
};

struct ENEMYDATA
{
    int r;
	int g;
	int b;
	float push_resistance;
    int radius;
	int health, maxhealth;
	int eatrate, eatcount;
	int splitrate, splitcount;
	int firerate, firecount;
	int membrane_thickness;
	int mode;
	int frozen;
	float scalar;  // For globs
	int tx, ty;  // target x/y
	int genre;
	bool isparticle;
	int generations;
	int atecount; // for minimap appearance
    CELLDATA *body;
};


struct SPRITE
{
    SPRITE() { dead = false; type = 0; }
    BITMAP *image;
    float x, y, lastx,lasty;
    float xv, yv;
    float rot;
    float friction, thrust, topspeed, bounce;
    unsigned char type;
    PLAYERDATA playerdata;
    PARTICLEDATA particledata;
    ENEMYDATA enemydata;
	BEAMDATA beamdata;
    bool mapcollision;
    int lifetime;
    bool dead;
	int col;
        
    SPRITE *next;
};

void add_sprite(SPRITE *S);
void add_particle(SPRITE *S);
bool remove_sprite(SPRITE *S, SPRITE *cur);
void draw_sprite(SPRITE *S);
void draw_sprites();
void update_sprite(SPRITE *S);
void update_bullet(SPRITE *S);
void update_blaster(SPRITE *S);
void update_missile(SPRITE *S);
void update_freeze_particle(SPRITE *S);
void update_ice_root(SPRITE *S);
void update_enemy(SPRITE *S);
void update_enemy_blaster(SPRITE *S);
void update_particle(SPRITE *S);
void update_sprites();
void draw_particle(SPRITE *S);
void draw_beam(SPRITE *S);
void spawn_particles(int num, float x, float y, int rand_radius, int sr, int sg, int sb, int sa,
					 int er, int eg, int eb, int ea, int minang, int maxang, float minspeed,
					 float maxspeed, float friction, int lifetime, bool sprites, int bmode);
void fire_enemy_blaster(SPRITE *E);
void fire_freeze_bomb(SPRITE *E);
void fire_missile(SPRITE *E);
void fire_scatter_missiles(SPRITE *E);
void explosion(float x, float y, int size, int duration);
void spawn_lightflash(int x, int y, int rad, int maxrad, int col, float speed);
void spawn_bubbles(int num, int x, int y, int distrib, float force, int size);
void pop_bubble(SPRITE *S, float force);
bool hit_bubble(SPRITE *S, int x, int y);

extern SPRITE *spritelist;
extern SPRITE *particlelist;
extern int spf;
extern SPRITE *boss;

#endif
