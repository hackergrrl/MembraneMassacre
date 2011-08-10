#include <allegro.h>
#include <math.h>
#include "main.h"
#include "sprites.h"
#include "enemy.h"
#include "map.h"
#include "utils.h"
#include "player.h"
#include "weapons.h"
#include "menu.h"

bool ehad_x_col, ehad_y_col;
SPRITE *curfoe=NULL;
int bmode_count=0;

void enemy_level_collision(SPRITE *S, int x, int y, int lx, int ly);
void create_globs(int num, int x, int y, float minang, float maxang, float minforce, float maxforce, int size, int r, int g, int b);
void update_boss(SPRITE *S);
void enemy_change_radius(SPRITE *S, int newrad);


SPECIESDATA species[MAX_SPECIES] = {
	{ 255, 255,255, 38, 40, 60, 200, 2500, 1.0f, 1.0f, 3.5f, 10, 25, 10, 35, 20, 3,   2, 20, 4 },   // Whites
	{ 255, 32, 16,  32, 25,100, 200, 2500, 0.3f, 1.2f, 3.7f, 0,  100,0,  0,  0,  2,   1, 10, 7 },   // Zombies
	{ 255, 160,64,  24, 8,  70, 280, 1800, 1.2f, 0.9f, 3.4f, 20, 25, 15, 35, 5,  5,   1, 8,  12  }, // Splitters
	{ 0,   200,200, 48, 45, 15, 200, 6000, 0.1f, 1.1f, 3.8f, 5,  5,  5,  80, 5,  3,   2, 20, 4 },   // Aquas
	{ 80,  200,32,  22, 25, 70, 90,  2700, 1.4f, 2.2f, 7.5f, 5,  10, 5,  10, 70, 0,   3, 30, 5 },   // Shooters
	{ 128, 0,128,   44, 45, 60, 200, 2500,-8.0f, 1.0f, 3.5f, 30, 20, 10, 20, 20, 2,   2, 20, 4 },   // Purples
	{ 128, 128,128, 50,200, 10, 200, 9999, 0.1f, 1.7f, 5.0f, 0, 100, 0,  0,  0,  0,   7, 100, 1 },  // Mecha
	{   0, 0, 255,  40, 100, 0,  200, 9999, 0.7f, 1.6f, 5.0f, 0, 100, 0,  0,  0, 0,   10, 100, 1 }, // Freezer
	{ 16,  16, 16,  35, 70,  0,  200, 9999, 0.0f, 2.8f, 6.5f, 0, 100, 0,  0,  0, 0,   7, 100, 1 },  // Stalker
	{ 255, 0, 0,    40, 35, 80, 200, 2350, 0.2f, 1.4f, 3.9f, 0,  100,0,  0,  0,  2,   5, 20,  7 },  // Super Zombies
	{ 255, 190,0,   32, 14, 50, 280, 1650, 1.1f, 1.1f, 3.6f, 20, 25, 15, 35, 5,  5,   4, 15,  10 }, // Super Splitters
	{ 32,  255,32,  28, 30, 50, 45,  2700, 1.5f, 2.5f, 8.0f, 5,  10, 5,  10, 70, 0,   6, 35,  5 },  // Super Shooters
};
/*
struct SPECIESDATA
{
	int r;
	int g;
	int b;
	int radius;
	int health;
	int eatrate;
	int firerate;
	int splitrate;
	float push_resistance;
	float thrust;
	float top_speed;
	int idle_chance;
	int chase_chance;
	int flee_chance;
	int eat_chance;
	int shoot_chance;
	int min_level;
	int difficulty;
};
*/



void draw_enemy(SPRITE *S)
{
	if(S->dead) return;

    int dx = S->x - mapx;
    int dy = S->y - mapy;
	float hq = ((float)S->enemydata.health / (float)S->enemydata.maxhealth);
	int rad = S->enemydata.radius;
    
	// Draw (transparent) membrane
	int *points = new int[rad*2];
	for(int b=0; b < rad; b++)
	{
		int a = b;
		if(a == rad-1) a--;
		if(a == 0) a++;
		float ang = ((float)b / (float)(rad)) * 360 * (3.14159f/180.0f);
		//float mem_thickness = 0.2f * ((float)S->enemydata.health / (float)S->enemydata.maxhealth) + 1.0f;
		points[b*2] = dx + cos(ang) * (S->enemydata.body[a].loc);
		points[b*2+1] = dy + sin(ang) * (S->enemydata.body[a].loc);
	}

	int r = S->enemydata.r * 0.7;
	int g = S->enemydata.g * 0.7;
	int b = S->enemydata.b * 0.7;
	if(S->enemydata.frozen > 0)
	{
		r = 0;
		g = 0;
		b = 200;
	}
	if(config[CFG_SHOW_MEMBRANES])// && S->enemydata.genre != SPECIES_MECHA_SMALL && S->enemydata.genre != SPECIES_MECHA_BIG)
	{
		set_trans_blender(r,g,b,128);
		drawing_mode(DRAW_MODE_TRANS, NULL, 0,0);
	}
	polygon(backbuffer, rad, points, makecol(r,g,b));
	drawing_mode(DRAW_MODE_SOLID, NULL, 0,0);

	delete[] points;


	// Draw cell body
	int max_size = 64;
	if(S->image == cellbody[0] || S->image == cellbody[1])
		max_size = 96;

	V3D_f **verts3d;
	verts3d = new V3D_f*[rad];
    for(int b=0; b < rad; b++)
    {
		int a = b;
		if(a == rad-1) a--;
		if(a == 0) a++;
        float ang = ((float)b / (float)(rad)) * 360 * (3.14159f/180.0f);
		verts3d[b] = new V3D_f();
		verts3d[b]->x = dx + cos(ang) * (S->enemydata.body[a].loc - S->enemydata.membrane_thickness*hq);
		verts3d[b]->y = dy + sin(ang) * (S->enemydata.body[a].loc - S->enemydata.membrane_thickness*hq);
		verts3d[b]->u = 64 + cos(ang) * minf(rad - S->enemydata.membrane_thickness, max_size);
		verts3d[b]->v = 64 + sin(ang) * minf(rad - S->enemydata.membrane_thickness, max_size);
		verts3d[b]->c = 128;  // Play around to change texture 'vividity'
		verts3d[b]->z = 0;
    }

	if( (S->enemydata.splitcount <= 30 && S->enemydata.splitcount > 0) || (S->enemydata.genre == SPECIES_FREEZER && S->lifetime <= 30) )
		set_trans_blender(S->enemydata.r/2,S->enemydata.g/2,S->enemydata.b/2,128);
	else
		set_trans_blender(S->enemydata.r,S->enemydata.g,S->enemydata.b,128);
	polygon3d_f(backbuffer, POLYTYPE_ATEX_LIT, S->image, rad, verts3d);

    for(int b=0; b < rad; b++)
		delete verts3d[b];
    delete[] verts3d;
}


void apply_pressure(SPRITE *S, float amount, float width, int x, int y)
{
	if(S->dead) return;
	if(S->enemydata.genre == SPECIES_MECHA)
		return;

	float dx = x - S->x;
	float dy = y - S->y;
	float ang = atan2(dy,dx) * (180.0f/3.14159f);
	if(ang < 0) ang += 360;
	if(ang > 359) ang -= 360;

	int angseg = ((float)ang / 360.0f) * (float)S->enemydata.radius;
	for(int a=-width; a <= width; a++)
	{
		int cang = angseg+a;
		if(cang < 0) cang += S->enemydata.radius;
		if(cang > S->enemydata.radius-1) cang -= S->enemydata.radius;
		S->enemydata.body[cang].vel -= amount * (1 - ((float)absf(a) / (float)width)*2);
		if(S->enemydata.body[cang].vel > S->topspeed)
			S->enemydata.body[cang].vel = S->topspeed;
		if(S->enemydata.body[cang].vel < -S->topspeed)
			S->enemydata.body[cang].vel = -S->topspeed;
	}

	ang -= 180;
	if(amount != 8.0f) // Missile hack
	{
		if(S->enemydata.radius <= 16) amount *= 1.2f;
		else if(S->enemydata.radius <= 24) amount *= 0.99f;
		else if(S->enemydata.radius <= 32) amount *= 0.9f;
		else if(S->enemydata.radius <= 40) amount *= 0.85f;
		else if(S->enemydata.radius <= 48) amount *= 0.8f;
		else amount *= 0.75f;
	}
	amount *= S->enemydata.push_resistance;
	if(amount < 0) amount = 0.1f;
	S->xv += cos( ang * (3.14159f/180.0f) ) * amount * 0.4f;
	S->yv += sin( ang * (3.14159f/180.0f) ) * amount * 0.4f;
	if(S->xv > S->topspeed) S->xv = S->topspeed;
	if(S->xv < -S->topspeed) S->xv = -S->topspeed;
	if(S->yv > S->topspeed) S->yv = S->topspeed;
	if(S->yv < -S->topspeed) S->yv = -S->topspeed;
}


SPRITE* create_enemy(SPRITE *parent, int x, int y, int genre)
{
	if(enemy_count >= MAX_ENEMIES)
		return NULL;

	SPECIESDATA sd = species[genre];
	int range=0;
    
	SPRITE *S = new SPRITE();
	S->enemydata.genre = genre;
    S->image = cellbody[random(3)];
	if(genre == SPECIES_MECHA)
	{
		S->image = cellbody[3];
		S->lifetime = 200;
	}
	if(genre == SPECIES_FREEZER)
		S->lifetime = 300;
    S->x = x;
    S->y = y;
    S->type = SPRITE_ENEMY;
    S->lifetime = -1;
    S->enemydata.r = sd.r;
    S->enemydata.g = sd.g;
    S->enemydata.b = sd.b;
	if(parent != NULL)
		S->enemydata.generations = parent->enemydata.generations-1;
	else
		S->enemydata.generations = sd.max_generations;
	range = sd.radius * 0.1;
    S->enemydata.radius = sd.radius + random(range)-(range/2);
	S->enemydata.membrane_thickness = S->enemydata.radius * 0.2f;
	S->xv = S->yv = 0;
	S->lastx = S->x;
	S->lasty = S->y;
	S->enemydata.tx = S->x;
	S->enemydata.ty = S->y;
	S->friction = 0.99f;
	S->thrust = 0.02f * sd.thrust_mod * diff_mod();
	S->topspeed = sd.top_speed * diff_mod();
	S->bounce = 0.4f;
	S->enemydata.atecount = 0;
	S->enemydata.eatrate = sd.eatrate;
	S->enemydata.eatcount = random(S->enemydata.eatrate);
	S->enemydata.splitrate = sd.splitrate;
	S->enemydata.splitcount = random(S->enemydata.splitrate);
	S->enemydata.firerate = sd.firerate;
	S->enemydata.firecount = random(S->enemydata.firerate);
	range = maxf(2.0f, sd.health * 0.1f);
	S->enemydata.maxhealth = sd.health + random(range)-(range/2);
	S->enemydata.health = S->enemydata.maxhealth * diff_mod();
	S->enemydata.frozen = 0;
	S->enemydata.push_resistance = sd.push_resistance;
	int rnum = random(100)+1;
	if(rnum < sd.idle_chance) S->enemydata.mode = MODE_IDLE;
	else if(rnum < sd.idle_chance+sd.chase_chance) S->enemydata.mode = MODE_CHASE;
	else if(rnum < sd.idle_chance+sd.chase_chance+sd.flee_chance) S->enemydata.mode = MODE_FLEE;
	else if(rnum < sd.idle_chance+sd.chase_chance+sd.flee_chance+sd.eat_chance) S->enemydata.mode = MODE_EAT;
	else S->enemydata.mode = MODE_SHOOT;

	// Hack
	if(S->enemydata.mode == MODE_FLEE)
		S->enemydata.mode = MODE_EAT;

    S->enemydata.body = new CELLDATA[S->enemydata.radius];
    for(int a=0; a < S->enemydata.radius; a++)
    {
        S->enemydata.body[a].loc = S->enemydata.radius;
		S->enemydata.body[a].vel = 0;
    }

	enemy_count++;
    add_sprite(S);

	destroy_terrain(S->x, S->y, S->enemydata.radius+3);
	return S;
}


void update_enemy(SPRITE *S)
{
	if(S == boss)
	{
		update_boss(S);
		return;
	}
	if(S->dead) return;
	if(S->x < 0 || S->x >= level->w || S->y < 0 || S->y >= level->h) { if(!S->dead) enemy_count--; S->dead = true; return; }

	S->enemydata.frozen = maxf(0, S->enemydata.frozen-1);
	S->enemydata.atecount = maxf(0, S->enemydata.atecount-1);
	S->enemydata.splitcount = maxf(0, S->enemydata.splitcount-1);
	if(S->enemydata.splitcount == 0)
		enemy_split(S);
	else if(S->enemydata.splitcount == 25)
		play_sound(stretch_sound, S->x, S->y);

	if(S->enemydata.frozen == 0)
	{
		// Cell body physics
		for(int a=0; a < S->enemydata.radius; a++)
		{
			CELLDATA *n1, *n2;
			if(a+1 > S->enemydata.radius-1)
				n1 = &S->enemydata.body[0];
			else
				n1 = &S->enemydata.body[a+1];
			if(a-1 < 0)
				n2 = &S->enemydata.body[S->enemydata.radius-1];
			else
				n2 = &S->enemydata.body[a-1];

			float dist = S->enemydata.radius - S->enemydata.body[a].loc;
			S->enemydata.body[a].vel += dist*0.01f;

			S->enemydata.body[a].vel *= 0.99f;
			n1->vel = (n1->vel * 0.5f) + (S->enemydata.body[a].vel * 0.5f);
			n2->vel = (n2->vel * 0.5f) + (S->enemydata.body[a].vel * 0.5f);

			S->enemydata.body[a].loc += S->enemydata.body[a].vel;
		}

		// Evade other cells (if touching)
		bool evaded=false;
		SPRITE *E = spritelist;
		while(E != NULL && E->type == SPRITE_ENEMY)
		{
			if(E == S)
			{
				E = E->next;
				continue;
			}

			float dx = S->x - E->x;
			float dy = S->y - E->y;
			float ang = atan2(dy,dx);
			if( sqrt(dx*dx + dy*dy) <= S->enemydata.radius + E->enemydata.radius )
			{
				evaded = true;
				float extra_push = 1.0f;
				if(E->enemydata.genre == SPECIES_STALKER)
					extra_push = 3.5f;
				S->xv += cos(ang) * S->thrust * extra_push;
				S->yv += sin(ang) * S->thrust * extra_push;
				apply_pressure(S, 0.15f, 4, E->x, E->y);
			}
			E = E->next;
		}
		
		int tmp = S->enemydata.mode;
		if(!player->playerdata.alive)
			S->enemydata.mode = MODE_EAT;
		if(evaded)
			S->enemydata.mode = MODE_IDLE;
		switch( S->enemydata.mode )
		{
			case MODE_CHASE:
			{
				// Chase player
				float dx = player->x - (S->x);
				float dy = player->y - (S->y);
				S->rot = atan2(dy,dx) * (180.0f/3.14159);
				S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
				S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;

				if(S->enemydata.genre == SPECIES_FREEZER && sqrt( (S->x-player->x)*(S->x-player->x) + (S->y-player->y)*(S->y-player->y) ) <= 500 )
				{
					S->lifetime--;
					if(S->lifetime <= 0)
					{
						fire_freeze_bomb(S);
						if(S->lifetime <= 0)
							S->lifetime = 600;
					}
				}
				if(S->enemydata.genre == SPECIES_MECHA)
				{
					float dist = sqrt( (S->x-player->x)*(S->x-player->x) + (S->y-player->y)*(S->y-player->y) );
					if(dist <= 600)
					{
						S->lifetime--;
						if(S->lifetime <= 0)
						{
							// Trace a (truncated) path to the player; ensure no walls nearly
							float px = (player->y+player->image->h/2);
							float py = (player->x+player->image->w/2);
							float ang = atan2( px - S->y, py - S->x ); 
							float dx = cos(ang)*3;
							float dy = sin(ang)*3;
							float rx = S->x;
							float ry = S->y;
							bool can_fire = true;
							for(int a=0; a < 40; a++)
							{
								rx += dx;
								ry += dy;
								if(getpixel(level,int(rx),int(ry)) != makecol(255,0,255))
								{
									S->lifetime = 30;
									can_fire = false;
									break;
								}
							}
							if(can_fire)
							{
								fire_missile(S);
								if(S->lifetime <= 0)
									S->lifetime = 70;
							}
						}
					}
				}

				break;
			}
			case MODE_FLEE:
			{
				// Run from player
				float dx = player->x + player->image->w/2 - (S->x);
				float dy = player->y + player->image->h/2 - (S->y);
				S->rot = atan2(dy,dx) * (180.0f/3.14159) - 180 + random(30)-15;
				S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
				S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
				break;
			}
			case MODE_EAT:
			{
				// Move towards target point
				float dx = S->enemydata.tx - (S->x);
				float dy = S->enemydata.ty - (S->y);
				S->rot = atan2(dy,dx) * (180.0f/3.14159);
				S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
				S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
				if( absf(dx) < 1 && absf(dy) < 1 )
				{
					int tries = 100;
					while( getpixel(level, S->enemydata.tx, S->enemydata.ty) == makecol(255,0,255) )
					{
						int rad = S->enemydata.radius;
						S->enemydata.tx = random(level->w-rad*2)+rad;
						S->enemydata.ty = random(level->h-rad*2)+rad;
						if(tries <= 0)
						{
							S->enemydata.mode = MODE_CHASE;
							break;
						}
						else
							tries--;
					}
				}
				break;
			}
			case MODE_SHOOT:
			{
				// Aim at player
				float dx = player->x + player->image->w/2 - (S->x);
				float dy = player->y + player->image->h/2 - (S->y);
				S->rot = atan2(dy,dx) * (180.0f/3.14159);
				if( sqrt(dx*dx + dy*dy) >= 400 )
				{
					S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
					S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
				}
				else
				{
					S->xv -= cos( S->rot * (3.14159/180.0f) ) * S->thrust*0.7f;
					S->yv -= sin( S->rot * (3.14159/180.0f) ) * S->thrust*0.7f;
					S->enemydata.firecount = maxf(0, S->enemydata.firecount-1);
					if(S->enemydata.firecount == 0)
						fire_enemy_blaster(S);
				}
				break;
			}
		}
		S->enemydata.mode = tmp;
	}

	// Movement physics
    ehad_x_col = false;
    ehad_y_col = false;
    S->mapcollision = false;
	float ratio = (360.0f / (float)S->enemydata.radius);
	for(int a=0; a < S->enemydata.radius; a++)
    {
		int ang = a * ratio;
		float cx = S->x + cos( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
        float cy = S->y + sin( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
		float lx = S->lastx + cos( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
        float ly = S->lasty + sin( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
        enemy_level_collision(S, (int)cx, (int)cy, (int)lx, (int)ly);
    }
    if(ehad_x_col)
    {        
        S->x = S->lastx - S->xv*2;
        S->xv = -S->xv*S->bounce;
    }
    if(ehad_y_col)
    {
        S->y = S->lasty - S->yv*2;
        S->yv = -S->yv*S->bounce;
    }

	if(S->enemydata.eatcount <= 0)
	{
		curfoe = S;
		do_circle(level, S->x, S->y, S->enemydata.radius+2, 0, digest_circle);
		int addrad = maxf(sqrt(S->xv*S->xv + S->yv*S->yv), 4);
		destroy_terrain(S->x,S->y,S->enemydata.radius+addrad);
		S->enemydata.eatcount = S->enemydata.eatrate;
	}
	S->enemydata.eatcount--;
    
    S->lastx = S->x;
    S->lasty = S->y;
    if(S->xv < -S->topspeed) S->xv = -S->topspeed;
    if(S->xv > S->topspeed) S->xv = S->topspeed;
    if(S->yv < -S->topspeed) S->yv = -S->topspeed;
    if(S->yv > S->topspeed) S->yv = S->topspeed;
    S->x += S->xv;
    S->y += S->yv;
    S->xv *= S->friction;
    S->yv *= S->friction;

	// Edge-fix/hack
	if(S->x == 0)
		S->x = 1;
	if(S->x == level->w-1)
		S->x = level->w-2;
	if(S->y == 0)
		S->y = 1;
	if(S->y == level->h-1)
		S->y = level->h-2;
}


bool hit_enemy(SPRITE *S, int x, int y)
{
	if(S->dead) return false;

	float dx = x - S->x;
	float dy = y - S->y;

	if(absf(dx) > S->enemydata.radius*2 || absf(dy) > S->enemydata.radius*2)
		return false;

	float ang = atan2(dy,dx) * (180.0f/3.14159f);
	if(ang < 0) ang += 360;
	if(ang > 359) ang -= 360;
	int angseg = (float)((float)ang / 360.0f) * S->enemydata.radius;
	float segx = S->x + cos( ang * (3.14159f/180.0f) ) * S->enemydata.body[angseg].loc;
	float segy = S->y + sin( ang * (3.14159f/180.0f) ) * S->enemydata.body[angseg].loc;
	dx = x - segx;
	dy = y - segy;

	return sqrt((float)(dx*dx + dy*dy)) <= 5;
}


void enemy_level_collision(SPRITE *S, int x, int y, int lx, int ly)
{
	if(!S)
		return;
/*
	if(x < 0 || lx < 0 || x >= level->w || lx >= level->w || y < 0 || ly < 0 || y >= level->h || ly >= level->h)
	{
		if(!S->dead)
			enemy_count--;
		S->dead = true;
		return;
	}
*/
    if( getpixel(level,x,ly) != makecol(255,0,255) )
    {
        ehad_x_col = true;
        S->mapcollision = true;
		if(getpixel(level,lx,ly) == makecol(255,0,255))
		{
			float vel = sqrt(S->xv*S->xv + S->yv*S->yv)*2;
			apply_pressure(S, vel, 5, x, ly);
		}
    }
    if( getpixel(level,lx,y) != makecol(255,0,255) )
    {
        ehad_y_col = true;
        S->mapcollision = true;
		if(getpixel(level,lx,ly) == makecol(255,0,255))
		{
			float vel = sqrt(S->xv*S->xv + S->yv*S->yv)*2;
			apply_pressure(S, vel, 5, lx, y);
		}
    }

	if(x <= 0) S->x += 3;
	if(x >= level->w-1) S->x -= 3;
	if(y <= 0) S->y += 3;
	if(y >= level->h-1) S->y -= 3;
}


void digest_circle(BITMAP *buf, int x, int y, int d)
{
	if(random(20) != 0 || getpixel(buf,x,y) == makecol(255,0,255))
		return;

	lvl++;

	if(d != 0 || curfoe == NULL)
		return;

	curfoe->enemydata.atecount = 10;

	spawn_particles(4, x, y, 1,
					curfoe->enemydata.r,curfoe->enemydata.g,curfoe->enemydata.b,255,
					curfoe->enemydata.r/2,curfoe->enemydata.g/2,curfoe->enemydata.b/2,0,
					0,359, 0.2f,0.3f, 0.997f, 32, true, BLEND_ALPHA);

	if(curfoe->enemydata.genre == SPECIES_FREEZER)
	{
		float ang = atan2(y-curfoe->y, x-curfoe->x);
		if(random(100) >= 50)
			ang += 3.14159f / 2.0f;
		else
			ang -= 3.14159f / 2.0f;
		create_ice_root(x + cos(ang)*50, y + sin(ang)*50, ang * (180.0f / 3.14159f), 3);
	}

	// Try making enemy grow from eating? (+regen)
	if(curfoe != boss)
	{
		if(random(17) == 1 && curfoe->enemydata.radius < species[curfoe->enemydata.genre].radius*1.3f)
			enemy_change_radius(curfoe, curfoe->enemydata.radius+1);
		if(random(12) == 1 && curfoe->enemydata.health < curfoe->enemydata.maxhealth*1.4f)
			curfoe->enemydata.health++;
	}
}


void apply_damage(SPRITE *S, int dam)
{
	if(!S)
		return;

	if(is_survival())
		dam *= 2;

	if(S == boss)
	{
		if(config[CFG_DIFFICULTY] == DIFF_EASY)
			dam = maxf(1, dam*1.3);
		if(config[CFG_DIFFICULTY] == DIFF_HARD)
			dam = maxf(1, dam*0.8);
		if(config[CFG_DIFFICULTY] == DIFF_IMPOSSIBLE)
			dam = maxf(1, dam*0.6);
	}

	S->enemydata.health -= dam;

	// Hackish; to allow the player to break out of the boss
	if(S == boss)
	{
		float dx = (player->x + player->image->w/2) - S->x;
		float dy = (player->y + player->image->h/2) - S->y;
		if( sqrt(dx*dx + dy*dy) <= S->enemydata.radius )
		{
			float ang = atan2(dy,dx);
			player->xv += cos(ang)*dam*0.2f;
			player->yv += sin(ang)*dam*0.2f;
		}
	}

	if(S->enemydata.health <= 0 && !S->dead && S != boss)
	{
		// Fancy death sequence
		create_globs(6, S->x, S->y, 0,359, 0.7f,0.75f, S->enemydata.radius/2, S->enemydata.r, S->enemydata.g, S->enemydata.b);
		spawn_bubbles(random(2)+4, S->x, S->y, 16, 4.5f, 12);
		spawn_particles(S->enemydata.radius*2, S->x, S->y, S->enemydata.radius,
						255,255,255,255,
						curfoe->enemydata.r,curfoe->enemydata.g,curfoe->enemydata.b,0,
						0,359, 0.5f,0.6f, 0.99f, 48, true, BLEND_ALPHA);
		S->dead = true;
		enemy_count--;

		/*
		weapons[1].ammo = min(weapons[1].maxammo, weapons[1].ammo+10);
		if(random(100) < 20)
			weapons[2].ammo = min(weapons[2].maxammo, weapons[2].ammo+1);
		weapons[3].ammo = min(weapons[3].maxammo, weapons[3].ammo+15);
		if(random(100) < 5)
			weapons[4].ammo = min(weapons[4].maxammo, weapons[4].ammo+1);
		*/

		if(S->enemydata.genre == SPECIES_MECHA)
			explosion(S->x,S->y, 64,2);
		if(S->enemydata.genre == SPECIES_FREEZER)
			fire_freeze_bomb(S);



		if(random(100) < 50)
			play_sound(pop1_sound, S->x, S->y);
		else
			play_sound(pop2_sound, S->x, S->y);

	}
}


void enemy_split(SPRITE *S)
{
	if(S->enemydata.generations <= 0)
	{
		return;
	}

	SPRITE *N = create_enemy(S, S->x+random(8)-4, S->y+random(8)-4, S->enemydata.genre);
	if(N != NULL)
	{
		if(random(100) < 50)
			play_sound(pop1_sound, S->x, S->y);
		else
			play_sound(pop2_sound, S->x, S->y);

		N->enemydata.splitcount = S->enemydata.splitrate;
		apply_pressure(S, 8, 10, N->x,S->y);
		apply_pressure(N, 8, 10, S->x,S->y);
		S->enemydata.splitcount = S->enemydata.splitrate + S->enemydata.splitrate*(enemy_count / 100);
		S->enemydata.health *= 0.75f;
		create_globs(4, S->x, S->y, 0,359, 0.7f,0.71f, 7, S->enemydata.r, S->enemydata.g, S->enemydata.b);
		spawn_bubbles(random(2)+4, S->x, S->y, 16, 4.5f, 12);

		//S->enemydata.radius *= 0.75f;  // Reduce overall mass
//		S->enemydata.radius = maxf(10,S->enemydata.radius);
//		N->enemydata.radius = S->enemydata.radius;
	}
	else
	{
		S->enemydata.splitcount = 25000;
	}

}



// GLOB stuff

void draw_glob(SPRITE *S)
{
    int dx = S->x - mapx;
    int dy = S->y - mapy;

	if(S->enemydata.radius * S->enemydata.scalar > 3)
	{
		int r = S->enemydata.r;
		int g = S->enemydata.g;
		int b = S->enemydata.b;
		if(config[CFG_PARTICLE_BLENDING])
		{
			drawing_mode(DRAW_MODE_TRANS, NULL, 0,0);
			set_trans_blender(r,g,b,64);
		}

		circlefill(backbuffer, dx, dy, S->enemydata.radius * S->enemydata.scalar - 0, makecol(r*0.7f,g*0.7f,b*0.7f));
		circlefill(backbuffer, dx, dy, S->enemydata.radius * S->enemydata.scalar - 1, makecol(r*0.8f,g*0.8f,b*0.8f));
		circlefill(backbuffer, dx, dy, S->enemydata.radius * S->enemydata.scalar - 2, makecol(r*0.9f,g*0.9f,b*0.9f));
		circlefill(backbuffer, dx, dy, S->enemydata.radius * S->enemydata.scalar - 3, makecol(r,g,b));

		if(config[CFG_PARTICLE_BLENDING])
			drawing_mode(DRAW_MODE_SOLID, NULL, 0,0);
	}
}


void update_glob(SPRITE *S)
{
	// Cell body physics
	for(int a=0; a < S->enemydata.radius; a++)
	{
		CELLDATA *n1, *n2;
		if(a+1 > S->enemydata.radius-1)
			n1 = &S->enemydata.body[0];
		else
			n1 = &S->enemydata.body[a+1];
		if(a-1 < 0)
			n2 = &S->enemydata.body[S->enemydata.radius-1];
		else
			n2 = &S->enemydata.body[a-1];

		float dist = S->enemydata.radius - S->enemydata.body[a].loc;
		S->enemydata.body[a].vel += dist*0.01f;

		S->enemydata.body[a].vel *= 0.999f;
		n1->vel = (n1->vel * 0.5f) + (S->enemydata.body[a].vel * 0.5f);
		n2->vel = (n2->vel * 0.5f) + (S->enemydata.body[a].vel * 0.5f);

		S->enemydata.body[a].loc += S->enemydata.body[a].vel;
	}

	S->x += S->xv;
	S->y += S->yv;

	if(getpixel(level,S->x,S->y) != makecol(255,0,255))
		S->friction = 0.06f;

	S->enemydata.scalar -= S->friction;
	if(S->enemydata.scalar <= 0)
		S->dead = true;
}


void create_globs(int num, int x, int y, float minang, float maxang, float minforce, float maxforce, int size, int r, int g, int b)
{
	// CULLING FTW!
	if( absf(mapx - x) >= SCREEN_W*1.5 || absf(mapy - y) >= SCREEN_H*1.5 )
		return;

	for(int a=0; a < num; a++)
	{
		SPRITE *S = new SPRITE();
		S->image = NULL;
		S->x = x + random(size) - (size/2);
		S->y = y + random(size) - (size/2);
		S->type = SPRITE_GLOB;
		S->friction = 0.003f;
		S->enemydata.r = r;
		S->enemydata.g = g;
		S->enemydata.b = b;
		S->enemydata.radius = size;
		S->rot = random(maxang-minang)+minang;
		float force = (random(maxforce*1000-minforce*1000) + minforce*1000) / 1000.0f;
		S->xv = cos( S->rot * (3.14159f/180.0f) ) * force;
		S->yv = sin( S->rot * (3.14159f/180.0f) ) * force;
		S->lastx = S->x;
		S->lasty = S->y;
		S->enemydata.scalar = 1;

		S->enemydata.body = new CELLDATA[S->enemydata.radius];
		for(int a=0; a < S->enemydata.radius; a++)
		{
			S->enemydata.body[a].loc = S->enemydata.radius;
			S->enemydata.body[a].vel = random(3)-1;
		}

		add_sprite(S);
	}
}



// FINAL BOSS!
void create_boss(int x, int y)
{
	SPRITE *S = new SPRITE();
	S->enemydata.genre = 10;
    S->image = cellbody[1];
    S->x = x;
    S->y = y;
    S->type = SPRITE_ENEMY;
    S->lifetime = 0;
    S->enemydata.r = 255;
    S->enemydata.g = 255;
    S->enemydata.b = 32;
    S->enemydata.radius = 120;
	S->enemydata.membrane_thickness = 20;
	S->xv = S->yv = 0;
	S->lastx = S->x;
	S->lasty = S->y;
	S->enemydata.tx = S->x;
	S->enemydata.ty = S->y;
	S->friction = 0.99f;
	S->thrust = 0.3f * diff_mod();
	S->topspeed = 1.8f * diff_mod();
	S->bounce = 0.2f;
	S->enemydata.eatrate = 1;
	S->enemydata.eatcount = 1;
	S->enemydata.splitrate = 1000000000;
	S->enemydata.splitcount = 1000000000;
	S->enemydata.firerate = 35;
	S->enemydata.firecount = 120;
	S->enemydata.maxhealth = 4000;
	S->enemydata.health = S->enemydata.maxhealth;
	S->enemydata.frozen = 0;
	S->enemydata.push_resistance = 0;
	S->enemydata.mode = BMODE_IDLE;
	bmode_count = 180;

    S->enemydata.body = new CELLDATA[S->enemydata.radius];
    for(int a=0; a < S->enemydata.radius; a++)
    {
        S->enemydata.body[a].loc = S->enemydata.radius;
		S->enemydata.body[a].vel = 0;
    }

    add_sprite(S);

	destroy_terrain(S->x, S->y, 500);

	enemy_count++;
	boss = S;
}


void update_boss(SPRITE *S)
{
	if(S->dead) return;

	S->enemydata.frozen = maxf(0.0f, S->enemydata.frozen-30.0f);
	if(S->enemydata.frozen > 0)
		return;

	// Cell body physics
	for(int a=0; a < S->enemydata.radius; a++)
	{
		CELLDATA *n1, *n2;
		if(a+1 > S->enemydata.radius-1)
			n1 = &S->enemydata.body[0];
		else
			n1 = &S->enemydata.body[a+1];
		if(a-1 < 0)
			n2 = &S->enemydata.body[S->enemydata.radius-1];
		else
			n2 = &S->enemydata.body[a-1];

		float dist = S->enemydata.radius - S->enemydata.body[a].loc;
		S->enemydata.body[a].vel += dist*0.01f;

		S->enemydata.body[a].vel *= 0.999f;
		n1->vel = (n1->vel * 0.5f) + (S->enemydata.body[a].vel * 0.5f);
		n2->vel = (n2->vel * 0.5f) + (S->enemydata.body[a].vel * 0.5f);

		S->enemydata.body[a].loc += S->enemydata.body[a].vel;
	}

	// Player swallowed by boss
	if( sqrt( (player->x-S->x)*(player->x-S->x) + (player->y-S->y)*(player->y-S->y) ) <= S->enemydata.radius )
	{
		if(random(100) < 20)
			damage_player(2);
	}


	if(player->playerdata.alive)
	{
		// Boss AI decision 'states'
		bmode_count = maxf(0.0f, bmode_count-1.0f);
		if(S->enemydata.health < 500 && S->enemydata.health > 0) bmode_count = 0;
		if(bmode_count == 0)
		{
			int hp = S->enemydata.health;
			int m = S->enemydata.mode;
			int r=0, g=0, b=0;

			if(hp <= 0)
			{
				if(S->enemydata.mode == BMODE_DIE)
				{
					enemy_count = 0;
					win_pause = 300;
					S->dead = true;
					for(int a=0; a < 6; a++)
					{
						create_globs(4, S->x+random(200)-80, S->y+random(200)-80, 0,359, 0.7f,0.71f, 15, 100+random(130),0,0);
						if(random(100) < 50)
							play_sound(pop1_sound, S->x, S->y);
						else
							play_sound(pop2_sound, S->x, S->y);
					}
					destroy_terrain(S->x, S->y, 100);
					spawn_particles(100, S->x, S->y, 2, 255,255,255,255, 255,32,32,0, 0,359, 2.1f,2.2f, 0.99f, 100, false, BLEND_ADD);
					spawn_particles(100, S->x, S->y, 2, 255,255,255,255, 32,255,32,0, 0,359, 2.4f,2.5f, 0.99f, 100, false, BLEND_ADD);
					spawn_particles(100, S->x, S->y, 2, 255,255,255,255, 32,32,255,0, 0,359, 2.7f,2.8f, 0.99f, 100, false, BLEND_ADD);
					return;
				}
				S->enemydata.mode = BMODE_DIE;
				r = 128;
				bmode_count = 7*60;
			}
			else if(hp < 500)
			{
				S->enemydata.mode = BMODE_CHASE;
				S->lifetime = -1;
				r = 128;
			}
			else if(hp < 1500)
			{
				if(m == BMODE_FBOMB)
				{
					S->enemydata.mode = BMODE_SHOOT;
					bmode_count = 300;
					b = 128;
				}
				else
				{
					b = 255;
					S->enemydata.mode = BMODE_FBOMB;
					bmode_count = 1000;
				}
			}
			else if(hp < 2500)
			{
				r = 200; g = 100;
				if(S->lifetime == 0)
				{
					S->enemydata.mode = BMODE_SPAWN;
					S->enemydata.splitcount = random(6);
					S->enemydata.splitrate = 0;
					bmode_count = 120;
					S->lifetime = -1;
				}
				else if(S->lifetime == -1)
				{
					S->enemydata.mode = BMODE_CHASE;
					S->lifetime = -2;
					bmode_count = 800;
				}
				else
				{
					S->lifetime = 0;
					S->enemydata.mode = BMODE_CHASE;
					bmode_count = 180;
				}
			}
			else if(hp < 3500)
			{
				r = 128; b = 64;
				S->enemydata.mode = BMODE_SHOOT;
				bmode_count = 120;
			}
			else
			{
				r = 255; g = 255;
				S->enemydata.mode = BMODE_IDLE;
				bmode_count = 120;
			}
			S->enemydata.r = r;
			S->enemydata.g = g;
			S->enemydata.b = b;
		}


		switch( S->enemydata.mode )
		{
			case BMODE_CHASE:
			{
				// Chase player (or run)
				float dx = (player->x+player->image->w/2) - (S->x);
				float dy = (player->y+player->image->h/2) - (S->y);
				if( sqrt(dx*dx + dy*dy) <= S->enemydata.radius )
					break;
				S->rot = atan2(dy,dx) * (180.0f/3.14159);
				if(S->lifetime == -2)
				{
					S->xv -= cos( S->rot * (3.14159/180.0f) ) * S->thrust * 0.6f;
					S->yv -= sin( S->rot * (3.14159/180.0f) ) * S->thrust * 0.6f;
				}
				else
				{
					S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
					S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
				}
				break;
			}
			case BMODE_FBOMB:
			{
				// Go near player
				float dx = player->x + player->image->w/2 - (S->x);
				float dy = player->y + player->image->h/2 - (S->y);
				S->rot = atan2(dy,dx) * (180.0f/3.14159);
				if( sqrt(dx*dx + dy*dy) >= 300 )
				{
					S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
					S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
				}
				else if(bmode_count > 120)
				{
					fire_freeze_bomb(S);
					bmode_count = 120;
				}
				break;
			}
			case BMODE_SPAWN:
			{
				int rate = species[S->enemydata.splitcount].health*2;
				if(bmode_count % (rate/3) == 0)
				{
					SPRITE *N = create_enemy(NULL, S->x, S->y, S->enemydata.splitcount);
					N->rot = S->enemydata.splitrate * rate;
					N->xv = cos( N->rot * (3.14159f/180.0f) ) * 10;
					N->yv = sin( N->rot * (3.14159f/180.0f) ) * 10;
					N->enemydata.mode = MODE_CHASE;
					N->enemydata.generations = 0;
					S->enemydata.splitrate++;

					if(random(100) < 50)
						play_sound(pop1_sound, S->x, S->y);
					else
						play_sound(pop2_sound, S->x, S->y);

					create_globs(4, S->x, S->y, 0,359, 0.7f,0.71f, 7, S->enemydata.r, S->enemydata.g, S->enemydata.b);
					spawn_bubbles(random(2)+4, S->x, S->y, 16, 4.5f, 16);
				}
				break;
			}
			case BMODE_SHOOT:
			{
				// Aim at player
				float dx = player->x + player->image->w/2 - (S->x);
				float dy = player->y + player->image->h/2 - (S->y);
				S->rot = atan2(dy,dx) * (180.0f/3.14159);
				if( sqrt(dx*dx + dy*dy) >= 400 )
				{
					S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
					S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
				}
				else
				{
					S->enemydata.firecount = maxf(0, S->enemydata.firecount-1);
					if(S->enemydata.firecount == 0)
					{
						for(int a=0; a < 4; a++)
						{
							SPRITE *B = new SPRITE();
							B->image = NULL;
							B->x = S->x;
							B->y = S->y;
							B->rot = S->rot + random(60)-30;
							B->xv = cos( B->rot * (3.14159f/180.0f) ) * 2;
							B->yv = sin( B->rot * (3.14159f/180.0f) ) * 2;
							B->type = SPRITE_ENEMY_BLASTER;
							B->enemydata.genre = -1;
							B->lifetime = 0;
							add_sprite(B);
						}
						S->enemydata.firecount = S->enemydata.firerate;
						play_sound(blaster_sound, S->x, S->y);
					}
				}
				break;
			}
			case BMODE_DIE:
			{
				if(bmode_count % 15 == 0)
				{
					create_globs(4, S->x+random(200)-100, S->y+random(200)-100, 0,359, 0.7f,0.71f, 15, 100+random(100),0,0);
					if(random(100) < 50)
						play_sound(pop1_sound, S->x, S->y);
					else
						play_sound(pop2_sound, S->x, S->y);
				}
				if(bmode_count % 20 == 0)
					explosion(S->x+random(200)-100, S->y+random(200)-100, random(48)+16, 4);
				break;
			}
		}
	}


	// Movement physics
    ehad_x_col = false;
    ehad_y_col = false;
    S->mapcollision = false;
	float ratio = (360.0f / (float)S->enemydata.radius);
	for(int a=0; a < S->enemydata.radius; a++)
    {
		int ang = a * ratio;
		float cx = S->x + cos( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
        float cy = S->y + sin( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
		float lx = S->lastx + cos( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
        float ly = S->lasty + sin( ang * (3.14159/180.0f) ) * S->enemydata.body[a].loc;
        enemy_level_collision(S, (int)cx, (int)cy, (int)lx, (int)ly);
    }
    if(ehad_x_col)
    {        
        S->x = S->lastx - S->xv*2;
        S->xv = -S->xv*S->bounce;
    }
    if(ehad_y_col)
    {
        S->y = S->lasty - S->yv*2;
        S->yv = -S->yv*S->bounce;
    }

	if(S->enemydata.eatcount <= 0)
	{
		curfoe = S;
		do_circle(level, S->x, S->y, S->enemydata.radius+4, 0, digest_circle);
		destroy_terrain(S->x,S->y,S->enemydata.radius+4);
		S->enemydata.eatcount = S->enemydata.eatrate;
	}
	S->enemydata.eatcount--;
    
    S->lastx = S->x;
    S->lasty = S->y;
    if(S->xv < -S->topspeed) S->xv = -S->topspeed;
    if(S->xv > S->topspeed) S->xv = S->topspeed;
    if(S->yv < -S->topspeed) S->yv = -S->topspeed;
    if(S->yv > S->topspeed) S->yv = S->topspeed;
    S->x += S->xv;
    S->y += S->yv;
    S->xv *= S->friction;
    S->yv *= S->friction;

	// Edge-fix/hack
	if(S->x == 0)
		S->x = 1;
	if(S->x == level->w-1)
		S->x = level->w-2;
	if(S->y == 0)
		S->y = 1;
	if(S->y == level->h-1)
		S->y = level->h-2;
}


void enemy_change_radius(SPRITE *S, int newrad)
{
//	CELLDATA *old = new CELLDATA[S->enemydata.radius];
//	memcpy(old, S->enemydata.body, sizeof(S->enemydata.body));

	delete[] S->enemydata.body;
    S->enemydata.body = new CELLDATA[newrad];
    for(int a=0; a < newrad; a++)
    {
		S->enemydata.body[a].loc = newrad;
		S->enemydata.body[a].vel = 0;
//		int index = ((float)a / (float)newrad) * (float)S->enemydata.radius;
//		S->enemydata.body[a].loc = old[index].loc;
//		S->enemydata.body[a].vel = old[index].vel;
    }
//	delete[] old;
	S->enemydata.radius = newrad;
}
