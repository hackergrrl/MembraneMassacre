#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <allegro.h>
#include "sprites.h"

#define SHIP_FIGHTERX11		1
#define SHIP_CRIMSONWING	2
#define SHIP_INDIGOAVENGER	3
#define SHIP_JUGGERNAUT		4
#define SHIP_DOUBLEHEADER	5


struct SHIP
{
	bool owned;
	float thrust;
	float topspeed;
	float hullstrength;
	float ammoregen;
	int thrust_rating;
	int topspeed_rating;
	int hull_rating;
	int ammo_rating;
};



void update_player(SPRITE *S);
void set_ship_type(int s);
void damage_player(int dam);

extern int respawn_wait;
extern SPRITE *player;
extern float player_health;
extern int num_collisions;
extern SHIP shipdata[5];

#endif
