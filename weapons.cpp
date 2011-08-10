#include <math.h>
#include "main.h"
#include "weapons.h"
#include "map.h"
#include "sprites.h"
#include "player.h"
#include "utils.h"
#include "enemy.h"

int curweapon = 0;
int laser_delay=0;
SPRITE *curbeam=NULL;


WEAPONDATA weapons[MAX_WEAPONS] = {
    { true, -1,   -1,   25,  0, 0,0,    WEAPON_BLASTER },
    { false, 1000, 1000, 3,  0, 30,0,   WEAPON_MACHINEGUNS },
    { false, 25,   25,   60, 0, 800,0,  WEAPON_MISSILE },
    { false, 2000, 2000, 0,  0, 15,0,   WEAPON_LASER },
    { false, 5,    5,    120,0, 3600,0, WEAPON_FREEZE_BOMB },
    };


void update_bullet(SPRITE *S)
{
	bool expire=false;

    S->x += S->xv;
    S->y += S->yv;
    
    if( getpixel(level, S->x,S->y) != makecol(255,0,255) )
    {
		expire = true;
		do_circle(level, S->x, S->y, 3, 1, digest_circle);
    }

	SPRITE *E = spritelist;
	while(E != NULL)
	{
		if(E->type == SPRITE_ENEMY)
		{
			if( hit_enemy(E, S->x, S->y) )
			{
				if(E->enemydata.genre == SPECIES_MECHA)
				{
					float ang = atan2(S->y-E->y, S->x-E->x);
					float speed = sqrt(S->xv*S->xv + S->yv*S->yv);
					S->xv = cos(ang) * speed;
					S->yv = sin(ang) * speed;
					spawn_particles(6, S->x,S->y, 2, 255,255,16,255, 255,16,16,0, 0,360, 0.2f,0.3f, 0.99f, 16, false, BLEND_ADD);
					break;
				}
				else
				{
					expire = true;
					apply_pressure(E, 0.5f, 5, S->x,S->y);
					if(random(100) < 50)
						apply_damage(E, 1);
					else
						apply_damage(E, 2);
				}
				break;
			}
		}
		else
			break;
		E = E->next;
	}

	if(expire)
	{
        spawn_particles(6, S->x,S->y, 2, 255,255,16,255, 255,16,16,0, 0,360, 0.2f,0.3f, 0.99f, 16, false, BLEND_ADD);
        destroy_terrain(S->x,S->y, 3);
        S->lifetime = -1;
	}
}

void update_blaster(SPRITE *S)
{
	bool expire=false;

    S->x += S->xv;
    S->y += S->yv;
    
    spawn_particles(3, S->x,S->y, 3, 0,255,255,160, 0,255,32,16, 0,360, 0.5f,0.6f, 0.99f, 8, false, BLEND_ADD);

    if( getpixel(level, S->x,S->y) != makecol(255,0,255) )
    {
		expire = true;
		do_circle(level, S->x, S->y, 15, 1, digest_circle);
    }

	SPRITE *E = spritelist;
	while(E != NULL)
	{
		if(E->type == SPRITE_ENEMY)
		{
			if( hit_enemy(E, S->x, S->y) )
			{
				expire = true;
				apply_pressure(E, 3.5f, 8, S->x,S->y);
				apply_damage(E, 10);
				break;
			}
		}
		else
			break;
		E = E->next;
	}

	if(expire)
	{
        spawn_particles(25, S->x,S->y, 15, 255,255,255,255, 32,200,32,64, 0,360, 0.2f,0.25f, 0.99f, 24, false, BLEND_ADD);
        destroy_terrain(S->x,S->y, 15);
        S->lifetime = -1;
		play_sound(boom_sound, S->x, S->y);
	}
}

void update_missile(SPRITE *S)
{
	bool expire=false;

    S->x += S->xv;
    S->y += S->yv;

    S->enemydata.frozen--;
	if(S->enemydata.frozen < 0)
		S->enemydata.frozen = 0;
	if(S->enemydata.frozen == 1)
		expire = true;
	if(S->enemydata.frozen == 0)
	{
		float sx = (S->x+S->image->w/2) + cos( (S->rot-180) * (3.14159/180.0) ) * 8;
		float sy = (S->y+S->image->h/2) + sin( (S->rot-180) * (3.14159/180.0) ) * 8;
		spawn_particles(5, sx,sy, 3, 255,255,16,255, 128,128,128,64, 0,360, 0.5f,0.6f, 0.99f, 6, false, BLEND_ADD);

		if(S->type == SPRITE_SCATTER_MISSILE)
		{
			S->xv = cos( (S->rot + 30*sin((gcount+S->topspeed)/10.0f) ) * (3.14159f / 180.0f) ) * 4.25f;
			S->yv = sin( (S->rot + 30*sin((gcount+S->topspeed)/10.0f) ) * (3.14159f / 180.0f) ) * 4.25f;
			if(gcount % 3 == 0)
				spawn_particles(4, sx,sy, 5, 140,140,140,255, 140,140,140,64, 0,360, 0.2f,0.3f, 0.99f, 48, false, BLEND_ALPHA);
		}
		else if(gcount % 5 == 0)
			spawn_particles(10, sx,sy, 5, 140,140,140,255, 140,140,140,64, 0,360, 0.2f,0.3f, 0.99f, 48, false, BLEND_ALPHA);
	    
		if( getpixel(level, S->x+S->image->w/2,S->y+S->image->h/2) != makecol(255,0,255) )
		{
			expire = true;
			do_circle(level, S->x, S->y, 50, 1, digest_circle);
		}
	}

	SPRITE *E = spritelist;
	while(E != NULL)
	{
		if(E->type == SPRITE_ENEMY && E->enemydata.genre == SPECIES_STALKER && S->enemydata.frozen == 0)
		{
			float dx = (S->x+S->image->w/2) - (E->x);
			float dy = (S->y+S->image->h/2) - (E->y);
			float dist = sqrt( dx*dx + dy*dy );
			if( hit_enemy(E, S->x, S->y) )
			{
				spawn_bubbles(15, S->x, S->y, 10, 10.0f, 5);
				if(S->type == SPRITE_MISSILE)
				{
					S->xv = -S->xv*2;
					S->yv = -S->yv*2;
					S->rot -= 180;
					S->enemydata.genre = -1;
				}
				else
					S->dead = true;
				break;
			}
			else if( dist <= 300 )
			{
				apply_pressure(E, 0.5f, 5.0f, dx+E->x, dy+E->y);
				E->xv *= 0.8f;
				E->yv *= 0.8f;
			}
		}
		else if(E->type == SPRITE_ENEMY && (S->enemydata.genre != -1 || E->enemydata.genre != SPECIES_MECHA))
		{
			if( hit_enemy(E, S->x, S->y) )
			{
				expire = true;
				if(S->type == SPRITE_MISSILE)
				{
					apply_pressure(E, 8.0f, 15, S->x,S->y);
					apply_damage(E, 100);
				}
				else
				{
					apply_pressure(E, 4.0f, 10, S->x,S->y);
					apply_damage(E, 30);
				}
				break;
			}
		}
		else if(E->type == SPRITE_MISSILE && E != S)
		{
			float dx = (S->x+S->image->w/2) - (E->x+E->image->w/2);
			float dy = (S->y+S->image->h/2) - (E->y+E->image->h/2);
			if( sqrt( dx*dx + dy*dy ) <= 13 )
			{
				expire = true;
				explosion(E->x,E->y,24,1);
				E->lifetime = -1;
				break;
			}
		}
		E = E->next;
	}
	if(S->enemydata.genre == -1)
	{
		float dx = (S->x+S->image->w/2) - (player->x+player->image->w/2);
		float dy = (S->y+S->image->h/2) - (player->y+player->image->h/2);
		if( sqrt( dx*dx + dy*dy ) <= 15 )
		{
			expire = true;
			damage_player(10);
		}
	}

	if(expire)
	{
		if(S->type == SPRITE_MISSILE)
		{
			explosion(S->x+S->image->w/2, S->y+S->image->h/2, 40, 2);

			SPRITE *E = spritelist;
			while(E != NULL)
			{
				if(E->type == SPRITE_ENEMY)
				{
					float dx = S->x - E->x;
					float dy = S->y - E->y;
					float dist = sqrt(dx*dx + dy*dy);
					if( dist <= 50 )
					{
						float per = dist / 50.0f;
						apply_pressure(E, 4.0f*per, 10*per, S->x,S->y);
						apply_damage(E, 25);
						break;
					}
				}
				else
					break;
				E = E->next;
			}		
		}
		else
			explosion(S->x+S->image->w/2, S->y+S->image->h/2, 20, 2);
        S->lifetime = -1;

	}
}


void fire_laser()
{
	int laser_col = makecol(255,32,32);
	if(player->playerdata.shiptype == SHIP_INDIGOAVENGER)
		laser_col = makecol(128,32,255);

    float x = player->x + player->image->w/2;
    float y = player->y + player->image->h/2;
    float ang = player->rot * (3.14159f/180.0f);
	
	spawn_lightflash(x, y, 1, 12, laser_col, 0.8f);

    
    bool hitsomething=false;
    while(!hitsomething)
    {
        x += cos(ang)*2;
        y += sin(ang)*2;
        
        if( getpixel(level, (int)x, (int)y) != makecol(255,0,255) )
        {
            hitsomething = true;
			do_circle(level, x, y, 3, 1, digest_circle);
        }

		SPRITE *E = spritelist;
		while(E != NULL)
		{
			if(E->type == SPRITE_ENEMY)
			{
				if( hit_enemy(E, x, y) )
				{
					hitsomething = true;
					apply_pressure(E, 0.3f, 10, x,y);
					if(player->playerdata.shiptype == SHIP_INDIGOAVENGER)
					{
						E->xv = 0;
						E->yv = 0;
						int rad = minf(E->enemydata.radius*1.5, 60);
						spawn_lightflash(E->x, E->y, 1, rad, laser_col, 7.0f);
					}
					
					if(E->enemydata.genre == SPECIES_STALKER)
					{
						if(random(3) == 2)
						{
						    spawn_particles(10, E->x,E->y, E->enemydata.radius, 255,32,32,255, 200,200,32,255, 0,360, 0.4f,0.6f, 0.99f, 48, false, BLEND_ADD);
							E->enemydata.r = minf(255, E->enemydata.r+1);
							E->enemydata.g = maxf(0, E->enemydata.g-1);
							E->enemydata.b = maxf(0, E->enemydata.b-1);
							apply_damage(E,1);
						}
						E->xv *= 0.9f;
						E->yv *= 0.9f;
					}
					else if(random(2) == 0)
					{
						if(player->playerdata.shiptype == SHIP_INDIGOAVENGER)
							apply_damage(E, 3);
						else
							apply_damage(E, 2);
					}
					break;
				}
			}
			else
				break;
			E = E->next;
		}
    }

	if(player->playerdata.shiptype == SHIP_INDIGOAVENGER)
		destroy_terrain((int)x, (int)y, 4);
	else
		destroy_terrain((int)x, (int)y, 3);
    spawn_particles(3, (int)x,(int)y, 3, 255,32,32,255, 200,200,32,255, 0,360, 0.2f,0.3f, 0.99f, 48, false, BLEND_ADD);
	spawn_lightflash(x, y, 1, 12, laser_col, 0.8f);
    
    
	SPRITE *S = new SPRITE();
	S->type = SPRITE_BEAM;
	S->beamdata.x1 = player->x + player->image->w/2;
	S->beamdata.y1 = player->y + player->image->h/2;
	S->beamdata.x2 = x;
	S->beamdata.y2 = y;
	S->x = player->x;
	S->y = player->y;
	S->lifetime = 16;
	curbeam = S;  // Hackish, since we can thus only draw 1 laser at a time
	add_sprite(S);
	
	
	if(laser_delay > 0) return;
	play_sound(laser_sound, player->x, player->y);
    laser_delay = 30;  // 0.5 seconds, for the sound effect
}


void draw_beam(SPRITE *S)
{
	if(S != curbeam)
		return;
	int width= 15.0f * ((float)S->lifetime / 16.0f);
	S->beamdata.x1 = player->x + player->image->w/2;
	S->beamdata.y1 = player->y + player->image->h/2;

	int laser_col = makecol(200,0,0);
	if(player->playerdata.shiptype == SHIP_INDIGOAVENGER)
		laser_col = makecol(100,0,200);

	thickline(backbuffer, S->beamdata.x1-mapx,S->beamdata.y1-mapy,
			S->beamdata.x2-mapx,S->beamdata.y2-mapy, width,
			getr(laser_col), getg(laser_col), getb(laser_col), true);
}


void fire_freeze_bomb(SPRITE *E)
{
	int cx = E==player ? E->x + E->image->w/2 : E->x;
	int cy = E==player ? E->y + E->image->h/2 : E->y;

	if(E == player && player->playerdata.shiptype == SHIP_CRIMSONWING)
		spawn_lightflash(cx, cy, 4, 64, makecol(255,32,32), 2.25f);
	else
		spawn_lightflash(cx, cy, 4, 64, makecol(32,128,255), 2.25f);

    for(int a=0; a < 360; a += 2)
    {
        SPRITE *S = new SPRITE();
        S->image = NULL;
		S->x = cx;
		S->y = cy;
        S->rot = a;
        S->xv = cos( a * (3.14159f/180.0f) ) * 4;
        S->yv = sin( a * (3.14159f/180.0f) ) * 4;
        S->type = SPRITE_ICE;
        S->lifetime = 200;
		if(E != player)
			S->enemydata.genre = -1;
		else
		{
			if(player->playerdata.shiptype == SHIP_CRIMSONWING)
				S->rot = 1;
			S->enemydata.genre = 0;
		}
        add_sprite(S);
    }
	if(E == player)
	{
		if(player->playerdata.shiptype == SHIP_CRIMSONWING)
			spawn_particles(15, cx,cy, 8, 16,16,255,255, 200,8,8,32, 0,360, 0.5f,0.6f, 0.99f, 64, true, BLEND_ALPHA);
		else
			spawn_particles(15, cx,cy, 8, 16,16,255,255, 200,8,8,32, 0,360, 0.5f,0.6f, 0.99f, 64, true, BLEND_ALPHA);
	}
    play_sound(fbomb_sound, E->x, E->y);
}


void update_freeze_particle(SPRITE *S)
{
	bool expire = false;
    S->x += S->xv;
    S->y += S->yv;
    
    int mcol = getpixel(level, S->x,S->y);
    if( mcol != makecol(255,0,255) )
    {
		expire = true;
        
		if(random(100) > 70)
		{
			if(S->rot != 1)
				create_ice_root(S->x, S->y, S->rot, 3);
		}
    }
	else
		S->lifetime--;

	if(S->lifetime == 0)
		expire = true;


	SPRITE *E = spritelist;
	while(E != NULL)
	{
		if(E->type == SPRITE_ENEMY && S->enemydata.genre != -1)
		{
			if( hit_enemy(E, S->x, S->y) )
			{
				S->lifetime = -1;
				E->xv *= 0.7f;
				E->yv *= 0.7f;

				if(S->rot == 1)
				{
					apply_damage(E, 3);
					expire = true;
					break;
				}

				spawn_particles(4, S->x-S->xv*2,S->y-S->yv*2, 2, 16,16,255,255, 8,8,200,32, 0,360, 0.01f,0.05f, 0.99f, 48, false, BLEND_ALPHA);
				if(E->enemydata.genre == SPECIES_FREEZER)
				{
					if(random(4) == 1 && E->enemydata.radius < species[E->enemydata.genre].radius*2.0f)
						enemy_change_radius(E, E->enemydata.radius+1);
				}
				else if(E->enemydata.genre == SPECIES_MECHA)
				{
					E->enemydata.r = maxf(0, E->enemydata.r-2);
					E->enemydata.g = maxf(0, E->enemydata.g-2);
					E->enemydata.b = minf(255, E->enemydata.b+2);
					E->xv *= 0;
					E->yv *= 0;
					E->enemydata.frozen = 30;
				}
				else
				{
					apply_pressure(E, 0.5f, 5, S->x,S->y);
					apply_damage(E, 1);
					E->enemydata.frozen += 100;
					E->enemydata.splitcount = E->enemydata.splitrate*10;
				}
				break;
			}
		}
		else if(E->type == SPRITE_MISSILE && S->rot != 1)
		{
			float dx = (S->x) - (E->x+E->image->w/2);
			float dy = (S->y) - (E->y+E->image->h/2);
			if( sqrt( dx*dx + dy*dy ) <= 6 )
			{
				E->xv = 0;
				E->yv = 0;
				E->enemydata.frozen = 60;
				S->lifetime = -1;
				break;
			}
		}
		E = E->next;
	}
	if(S->enemydata.genre == -1)
	{
		float dx = S->x - (player->x + player->image->w/2);
		float dy = S->y - (player->y + player->image->h/2);
		if( sqrt( dx*dx + dy*dy ) <= 10 )
		{
			spawn_particles(4, S->x-S->xv*2,S->y-S->yv*2, 2, 16,16,255,255, 8,8,200,32, 0,360, 0.01f,0.05f, 0.99f, 48, false, BLEND_ALPHA);
			player->enemydata.frozen += 50;
			player->xv *= 0.4f;
			player->xv *= 0.4f;
			S->lifetime = -1;
			damage_player(1);
		}
	}

    if(expire)
	{
        int col;
		if(S->rot != 1 && random(100) < 30)
		{
			spawn_particles(4, S->x-S->xv*2,S->y-S->yv*2, 2, 16,16,255,255, 8,8,200,32, 0,360, 0.01f,0.05f, 0.99f, 48, false, BLEND_ALPHA);
			if( random(100) < 50 )
				col = makecol(0,0,128);
			else
				col = makecol(0,200,200);
		}
		else if(S->rot == 1 && random(100) < 40)
		{
			spawn_particles(4, S->x-S->xv*2,S->y-S->yv*2, 2, 16,16,255,255, 200,8,8,32, 0,360, 0.01f,0.05f, 0.99f, 48, false, BLEND_ALPHA);
			if( random(100) < 50 )
				col = makecol(128,0,128);
			else
				col = makecol(200,200,0);
			explosion(S->x, S->y, 13, 1);
			destroy_terrain(S->x,S->y,16);
		}

        S->lifetime = -1;
        
        if(random(100) > 90)
        {
            if(random(100) < 50)
				play_sound(freeze1_sound, S->x, S->y);
            else
				play_sound(freeze2_sound, S->x, S->y);
        }
	}
}


void fire_enemy_blaster(SPRITE *E)
{
    SPRITE *S = new SPRITE();
    S->image = NULL;
    S->x = E->x;
    S->y = E->y;
    S->rot = E->rot;
    S->xv = cos( S->rot * (3.14159f/180.0f) ) * 2;
    S->yv = sin( S->rot * (3.14159f/180.0f) ) * 2;
    S->type = SPRITE_ENEMY_BLASTER;
    S->lifetime = 0;
    add_sprite(S);

	E->enemydata.firecount = E->enemydata.firerate;

	play_sound(blaster_sound, E->x, E->y);
}

void update_enemy_blaster(SPRITE *S)
{
	bool expire=false;

    S->x += S->xv;
    S->y += S->yv;
    
    if(S->enemydata.genre == -1)
		spawn_particles(1, S->x,S->y, 3, 16,255,128,128, 255,190,0,64, 0,360, 0.5f,0.6f, 0.99f, 8, false, BLEND_ADD);
	else
		spawn_particles(1, S->x,S->y, 3, 16,255,255,255, 16,16,255,64, 0,360, 0.5f,0.6f, 0.99f, 8, false, BLEND_ADD);

    if( getpixel(level, S->x,S->y) != makecol(255,0,255) )
    {
		expire = true;
		do_circle(level, S->x, S->y, 15, 1, digest_circle);
    }

	float dx = S->x - (player->x + player->image->w / 2);
	float dy = S->y - (player->y + player->image->h / 2);
	if( sqrt(dx*dx + dy*dy) <= 10 )
	{
		float speed = sqrt(S->xv*S->xv + S->yv*S->yv) + sqrt(player->xv*player->xv + player->yv*player->yv);
		damage_player(speed*4.0f);
		player->xv = S->xv;
		player->yv = S->yv;
		spawn_particles(10, S->x, S->y, 10, 255,16,16,255, 255,255,16,32, 0,359, 0.4f,0.6f, 1, 32, true, BLEND_ADD);
		expire = true;
	}

	if(expire)
	{
		if(S->enemydata.genre == -1)  // Boss blaster
		{
			explosion(S->x,S->y,16,1);
		}
		spawn_particles(25, S->x, S->y, 10, 255,16,16,255, 255,255,16,32, 0,359, 0.4f,0.6f, 1, 32, true, BLEND_ADD);
        destroy_terrain(S->x,S->y, 15);
        S->lifetime = -1;
		play_sound(boom_sound, S->x, S->y);
	}
}



void fire_weapon(int index)
{
    WEAPONDATA *wep = &weapons[index];
    SPRITE *S;
    
    switch(index)
    {
        case WEAPON_MACHINEGUNS:
			if(player->playerdata.shiptype != SHIP_JUGGERNAUT)
			{
				S = new SPRITE();
				S->image = NULL;
				S->x = (player->x + player->image->w/2) + cos( (player->rot+90) * (3.14159f/180.0f) ) * 6;
				S->y = (player->y + player->image->h/2) + sin( (player->rot+90) * (3.14159f/180.0f) ) * 6;
				spawn_particles(4, S->x,S->y, 1, 255,255,16,255, 255,16,16,64, player->rot-15,player->rot+15, 0.5f,0.6f, 0.99f, 8, true, BLEND_ADD);
				S->rot = (random(6)-2) + player->rot;
				S->xv = cos( S->rot * (3.14159f/180.0f) ) * 2;
				S->yv = sin( S->rot * (3.14159f/180.0f) ) * 2;
				S->type = SPRITE_BULLET;
				S->lifetime = 0;
				add_sprite(S);
				spawn_lightflash(S->x, S->y, 2, 6, makecol(255,255,64), 0.4f);

				S = new SPRITE();
				S->image = NULL;
				S->x = (player->x + player->image->w/2) + cos( (player->rot-90) * (3.14159f/180.0f) ) * 6;
				S->y = (player->y + player->image->h/2) + sin( (player->rot-90) * (3.14159f/180.0f) ) * 6;
				spawn_particles(4, S->x,S->y, 1, 255,255,16,255, 255,16,16,64, player->rot-15,player->rot+15, 0.5f,0.6f, 0.99f, 8, true, BLEND_ADD);
				S->rot = (random(6)-2) + player->rot;
				S->xv = cos( S->rot * (3.14159f/180.0f) ) * 2;
				S->yv = sin( S->rot * (3.14159f/180.0f) ) * 2;
				S->type = SPRITE_BULLET;
				S->lifetime = 0;
				add_sprite(S);
				spawn_lightflash(S->x, S->y, 2, 6, makecol(255,255,64), 0.4f);
			}
			else
			{
				S = new SPRITE();
				S->image = NULL;
				S->x = (player->x + player->image->w/2);
				S->y = (player->y + player->image->h/2);
				spawn_particles(4, S->x,S->y, 1, 255,255,16,255, 255,16,16,64, player->rot-15,player->rot+15, 0.5f,0.6f, 0.99f, 8, true, BLEND_ADD);
				S->rot = (random(9)-4) + player->rot;
				S->xv = cos( S->rot * (3.14159f/180.0f) ) * 2;
				S->yv = sin( S->rot * (3.14159f/180.0f) ) * 2;
				S->type = SPRITE_BULLET;
				S->lifetime = 0;
				add_sprite(S);
				spawn_lightflash(S->x, S->y, 2, 6, makecol(255,255,64), 0.4f);
			}
            
			play_sound(mg_sound, S->x, S->y);
            break;
            
        case WEAPON_BLASTER:
		{
            S = new SPRITE();
            S->image = NULL;
            S->x = (player->x + player->image->w/2);
            S->y = (player->y + player->image->h/2);
            spawn_particles(6, S->x,S->y, 3, 0,255,255,255, 0,255,0,32, 0,360, 0.5f,0.6f, 0.99f, 8, true, BLEND_ALPHA);
            S->rot = player->rot;
            S->xv = cos( S->rot * (3.14159f/180.0f) ) * 2;
            S->yv = sin( S->rot * (3.14159f/180.0f) ) * 2;
            S->type = SPRITE_BLASTER;
            S->lifetime = 0;
            add_sprite(S);

            play_sound(blaster_sound, S->x, S->y);
			spawn_lightflash(S->x, S->y, 2, 8, makecol(64,255,64), 0.45f);
            break;
		}
        case WEAPON_MISSILE:
			if(player->playerdata.shiptype == SHIP_JUGGERNAUT)
				fire_scatter_missiles(player);
			else
				fire_missile(player);
            break;

        case WEAPON_LASER:
			fire_laser();
            break;
        
        case WEAPON_FREEZE_BOMB:
            fire_freeze_bomb(player);
            break;
	}
}


void fire_missile(SPRITE *E)
{
	int cx = E==player ? E->x + E->image->w/2 : E->x;
	int cy = E==player ? E->y + E->image->h/2 : E->y;

    SPRITE *S = new SPRITE();
    S->image = missileimage;
    S->x = cx - S->image->w/2;
    S->y = cy - S->image->h/2;
    spawn_particles(6, S->x,S->y, 3, 255,255,0,255, 255,0,0,32, 0,360, 0.5f,0.6f, 0.99f, 8, true, BLEND_ALPHA);
	float angle = E->rot * (3.14159f/180.0f);
	if(E != player)
	{
		float dist = sqrt( (E->x-player->x)*(E->x-player->x) + (E->y-player->y)*(E->y-player->y) );
		float px = (player->y+player->image->h/2) + player->xv*(dist/45.0f);
		float py = (player->x+player->image->w/2) + player->yv*(dist/45.0f);
		float ang = atan2( px - E->y, py - E->x );
		angle = ang;
	}
	S->xv = cos( angle ) * (E == player ? 4 : 4.5f);
    S->yv = sin( angle ) * (E == player ? 4 : 4.5f);
	S->rot = angle * (180.0f / 3.14159f);
    S->type = SPRITE_MISSILE;
    S->lifetime = 0;
	S->enemydata.frozen = 0;
	if(E != player)
		S->enemydata.genre = -1;
	else
		S->enemydata.genre = 0;
    add_sprite(S);

    play_sound(rocket_sound, S->x, S->y);
}



void create_ice_root(int x, int y, float ang, int branches)
{
    SPRITE *S = new SPRITE();
    S->image = NULL;
    S->x = x;
    S->y = y;
    S->rot = ang;
    S->type = SPRITE_ICE_ROOT;
    S->lifetime = branches;
    add_sprite(S);
}


void update_ice_root(SPRITE *S)
{
    //S->rot += random(20)-10;
    S->x += cos( S->rot * (3.14159f/180.0f) );
    S->y += sin( S->rot * (3.14159f/180.0f) );
    
    if( getpixel(level, S->x,S->y) == makecol(255,0,255) )
    {
        S->lifetime = 0;
        return;
    }
    
    if( random(10) == 0 )
    {
        int ang;
        if(random(100) < 50)
            ang = S->rot - 55 + random(20)-10;
        else
            ang = S->rot + 55 + random(20)-10;
        create_ice_root(S->x, S->y, ang, S->lifetime-1);
        S->lifetime--;
    }
    
	int col = makecol(0,140,230);
	putpixel_alpha(level, S->x, S->y, col, 170);
	float nx = S->x + cos( (S->rot+90) * (3.14159f/180.0f) );
	float ny = S->y + sin( (S->rot+90) * (3.14159f/180.0f) );
	if( getpixel(level, nx,ny) != makecol(255,0,255) )
		putpixel_alpha(level, nx,ny, col, 70);
	nx = S->x + cos( (S->rot-90) * (3.14159f/180.0f) );
	ny = S->y + sin( (S->rot-90) * (3.14159f/180.0f) );
	if( getpixel(level, nx,ny) != makecol(255,0,255) )
		putpixel_alpha(level, nx,ny, col, 70);
}


void fire_scatter_missiles(SPRITE *E)
{
	int cx = E==player ? E->x + E->image->w/2 : E->x;
	int cy = E==player ? E->y + E->image->h/2 : E->y;

    for(int a=0; a < 5; a++)
	{
		SPRITE *S = new SPRITE();
		S->image = missileimage2;
		S->x = cx - S->image->w/2;
		S->y = cy - S->image->h/2;
		if(E == player)
		{
			float offset = random(16)-8;
			S->x += cos( player->rot * (3.14159f / 180.0f) ) * offset;
			S->y += sin( player->rot * (3.14159f / 180.0f) ) * offset;
		}
		spawn_particles(6, S->x,S->y, 3, 255,255,0,255, 255,0,0,32, 0,360, 0.5f,0.6f, 0.99f, 8, true, BLEND_ALPHA);
		float angle = E->rot * (3.14159f/180.0f);
		if(E != player)
		{
			float dist = sqrt( (E->x-player->x)*(E->x-player->x) + (E->y-player->y)*(E->y-player->y) );
			float px = (player->y+player->image->h/2) + player->xv*(dist/45.0f);
			float py = (player->x+player->image->w/2) + player->yv*(dist/45.0f);
			float ang = atan2( px - E->y, py - E->x );
			angle = ang;
		}
		S->xv = cos( angle ) * (E == player ? 4 : 4.5f);
		S->yv = sin( angle ) * (E == player ? 4 : 4.5f);
		S->rot = (random(30) - 15) + angle * (180.0f / 3.14159f);
		S->type = SPRITE_SCATTER_MISSILE;
		S->lifetime = 0;
		S->enemydata.frozen = 0;
		S->topspeed = random(50000);
		if(E != player)
			S->enemydata.genre = -1;
		else
			S->enemydata.genre = 0;
		add_sprite(S);
	}

    play_sound(rocket_sound, E->x, E->y);
}
