#include <allegro.h>
#include <math.h>
#include "map.h"
#include "main.h"
#include "player.h"
#include "weapons.h"
#include "enemy.h"
#include "utils.h"
#include "menu.h"



SHIP shipdata[5] = {
	{ true,  0.065f, 2.7f, 1.0f, 1.0f, 3, 4, 3, 3 },
	{ false,  0.07f,  5.0f, 1.6f, 1.0f, 4, 5, 1, 3 },
	{ false, 0.09f,  3.0f, 1.3f, 0.8f, 5, 4, 2, 2 },
	{ false, 0.04f,  2.2f, 0.5f, 1.4f, 2, 2, 5, 4 },
	{ false,  0.05f,  2.2f, 1.0f, 1.7f, 3, 2, 3, 5 },
	};



SPRITE *player;
bool rmb_pressed=false;
float player_health=100;
int num_collisions=0;
int last_mouse_z=0;
int respawn_wait=0;

void player_level_collision(int x, int y, float ang);
bool had_x_col, had_y_col;


void update_player(SPRITE *S)
{
	// Cap health and keep frozen-counter above/equal-to zero
	player_health = maxf(player_health,0);
	player->enemydata.frozen = maxf(0.0f, player->enemydata.frozen-1.0f);

	// Update ammo regeneration (on all weapons)
	for(int a=1; a < 5; a++)
	{
		if(weapons[a].waitcount <= 0)
			weapons[a].regencount--;
		if(weapons[a].regencount <= 0)
		{
			weapons[a].ammo = minf(weapons[a].maxammo, weapons[a].ammo+1);
			weapons[a].regencount = weapons[a].regenrate;
			if(is_survival())
				weapons[a].regencount /= 2;
		}
	}


	if(!player->playerdata.alive && game_state == STATE_GAME)
	{
		// Sound effect cues
		if(respawn_wait == 235)
			play_sound(respawn_sound[0], S->x, S->y);
		if(respawn_wait == 1)
			play_sound(respawn_sound[1], S->x, S->y);

		float dx = S->playerdata.rx - S->x;
		float dy = S->playerdata.ry - S->y;
		float ang = atan2(dy,dx);
		S->x += cos(ang)*3.5f;
		S->y += sin(ang)*3.5f;
		if( absf(dx) < 3 && absf(dy) < 3 )
		{
			S->x = S->playerdata.rx;
			S->y = S->playerdata.ry;
			respawn_wait--;
			if(respawn_wait <= 0)
			{
				int testx = S->x+S->image->w/2;
				int testy = S->y+S->image->h/2;
				spawn_particles(50, testx, testy, 40, 255,32,255,255, 32,200,32,32, 0,359, 0.1f,0.6f, 1, 150, true, BLEND_ADD);
				S->playerdata.alive = true;
				S->lastx = S->x;
				S->lasty = S->y;
				S->enemydata.frozen = 0;				

				for(int a=0; a < 5; a++) // Return 25% of ammo
				{
					weapons[a].ammo = minf(weapons[a].maxammo, weapons[a].ammo + weapons[a].maxammo*0.25);
				}

				player_health = 100;
				destroy_terrain(testx, testy, 96);
				spawn_lightflash(S->x+S->image->w/2,S->y+S->image->h/2, 10, 54, makecol(255,255,255), 1.0f);
				
				SPRITE *E = spritelist;
				while(E != NULL)
				{
					if(E->type == SPRITE_ENEMY)
					{
						if( sqrt( (testx-E->x)*(testx-E->x) + (testy-E->y)*(testy-E->y) ) <= 96 )
						{
							apply_damage(E, 1000);
						}
					}
					else
						break;
					E = E->next;
				}
			}
			else if(respawn_wait > 20)
			{
				// Spawn random 'creation particles'
				for(int a=0; a < random(3); a++)
				{
					float ang = (respawn_wait*5) % 360 + random(180)-90;
					float dist = random(380-64)+64;
					float x = (S->x + S->image->w/2) + cos(ang*(3.14159f/180.0f))*dist;
					float y = (S->y + S->image->h/2) + sin(ang*(3.14159f/180.0f))*dist;
					float life = respawn_wait * 0.7f;
					if(life < 20) life = 20;
					float spd = dist / life;
					int minang = minf(ang-179,ang-181);
					int maxang = maxf(ang-179,ang-181);
					int r = random(256);
					int g = random(256);
					int b = random(256);
					spawn_particles(1, x,y, 1, 255,255,255,128, 200,200,200,255, minang,maxang, spd,spd+0.01f, 1,life,false,BLEND_ALPHA);
				}
//				spawn_lightflash(S->x+S->image->w/2+random(12)-6,S->y+S->image->h/2+random(12)-6, 10, 32, makecol(255,255,255), 2.5f);
			}
		}
		return;
	}

    float dx = mouse_x - (S->x-mapx+S->image->w/2);
    float dy = mouse_y - (S->y-mapy+S->image->h/2);
    S->rot = atan2(dy,dx) * (180.0f/3.14159);
    
    if(!config[CFG_ABSOLUTE_CONTROL])
	{
		if(key[KEY_UP] || key[KEY_W])
		{
			S->xv += cos( S->rot * (3.14159/180.0f) ) * S->thrust;
			S->yv += sin( S->rot * (3.14159/180.0f) ) * S->thrust;
		}
		if(key[KEY_DOWN] || key[KEY_S])
		{
			S->xv -= cos( S->rot * (3.14159/180.0f) ) * S->thrust*0.75f;
			S->yv -= sin( S->rot * (3.14159/180.0f) ) * S->thrust*0.75f;
		}
		if(key[KEY_RIGHT] || key[KEY_D])
		{
			S->xv += cos( (S->rot+90) * (3.14159/180.0f) ) * S->thrust*0.6f;
			S->yv += sin( (S->rot+90) * (3.14159/180.0f) ) * S->thrust*0.6f;
		}
		if(key[KEY_LEFT] || key[KEY_A])
		{
			S->xv += cos( (S->rot-90) * (3.14159/180.0f) ) * S->thrust*0.6f;
			S->yv += sin( (S->rot-90) * (3.14159/180.0f) ) * S->thrust*0.6f;
		}
	}
	else
	{
		if(key[KEY_UP] || key[KEY_W])
		{
			S->xv += cos( -90 * (3.14159/180.0f) ) * S->thrust * 0.85f;
			S->yv += sin( -90 * (3.14159/180.0f) ) * S->thrust * 0.85f;
		}
		if(key[KEY_DOWN] || key[KEY_S])
		{
			S->xv += cos( 90 * (3.14159/180.0f) ) * S->thrust * 0.85f;
			S->yv += sin( 90 * (3.14159/180.0f) ) * S->thrust * 0.85f;
		}
		if(key[KEY_RIGHT] || key[KEY_D])
		{
			S->xv += cos( 0 * (3.14159/180.0f) ) * S->thrust * 0.85f;
			S->yv += sin( 0 * (3.14159/180.0f) ) * S->thrust * 0.85f;
		}
		if(key[KEY_LEFT] || key[KEY_A])
		{
			S->xv += cos( 180 * (3.14159/180.0f) ) * S->thrust * 0.85f;
			S->yv += sin( 180 * (3.14159/180.0f) ) * S->thrust * 0.85f;
		}
	}
    
    weapons[curweapon].waitcount = maxf(0, weapons[curweapon].waitcount-1);
    if((mouse_b & 1) && weapons[curweapon].waitcount == 0 && (weapons[curweapon].ammo > 0 || weapons[curweapon].ammo == -1))
    {
		if(player->enemydata.frozen == 0)
		{
			weapons[curweapon].waitcount = weapons[curweapon].firerate;
			if( (player->playerdata.shiptype == SHIP_JUGGERNAUT || player->playerdata.shiptype == SHIP_DOUBLEHEADER)
				&& curweapon == WEAPON_MACHINEGUNS)
			{
				weapons[curweapon].waitcount *= 0.3f;
				if(random(100) < 50)
					weapons[curweapon].ammo--;
			}
			else if(weapons[curweapon].ammo != -1)
				weapons[curweapon].ammo--;

			bool fired = false;
			if(player->playerdata.shiptype == SHIP_DOUBLEHEADER)
			{
				if(curweapon == WEAPON_BLASTER || curweapon == WEAPON_MISSILE)
				{
					float ox = player->x;
					float oy = player->y;
					player->x = ox + cos( (player->rot + 90) * (3.14159f / 180.0f) ) * 8;
					player->y = oy + sin( (player->rot + 90) * (3.14159f / 180.0f) ) * 8;
					fire_weapon(curweapon);
					player->x = ox - cos( (player->rot + 90) * (3.14159f / 180.0f) ) * 8;
					player->y = oy - sin( (player->rot + 90) * (3.14159f / 180.0f) ) * 8;
					fire_weapon(curweapon);
					player->x = ox;
					player->y = oy;
					fired = true;
				}
			}
			if(!fired)
				fire_weapon(curweapon);
		}
    }
    
    if(!(mouse_b & 2) && rmb_pressed)
        rmb_pressed = false;
    if((mouse_b & 2) && !rmb_pressed)
    {
        rmb_pressed = true;
        do
        {
            curweapon++;
            if(curweapon > 4)
                curweapon = 0;
        } while( !weapons[curweapon].owned );
    }

	if(config[CFG_MOUSE_WHEEL])
	{
		int mz_delta = last_mouse_z - mouse_z;
		if(mz_delta < 0)
		{
			do
			{
				curweapon++;
				if(curweapon > 4)
					curweapon = 0;
			} while( !weapons[curweapon].owned );
		}
		else if(mz_delta > 0)
		{
			do
			{
				curweapon--;
				if(curweapon < 0)
					curweapon = 4;
			} while( !weapons[curweapon].owned );
		}
		last_mouse_z = mouse_z;
	}

	if(key[KEY_1]) curweapon = 0;
    if(key[KEY_2]) curweapon = 1;
    if(key[KEY_3]) curweapon = 2;
    if(key[KEY_4]) curweapon = 3;
    if(key[KEY_5]) curweapon = 4;
    
    // Cap speed based on direction
	float rot = atan2(S->yv, S->xv);
	if(S->xv < 0 && S->xv < S->topspeed*cos(rot)) S->xv = S->topspeed*cos(rot);
    if(S->xv > 0 && S->xv > S->topspeed*cos(rot)) S->xv = S->topspeed*cos(rot);
	if(S->yv < 0 && S->yv < S->topspeed*sin(rot)) S->yv = S->topspeed*sin(rot);
    if(S->yv > 0 && S->yv > S->topspeed*sin(rot)) S->yv = S->topspeed*sin(rot);
    
    had_x_col = false;
    had_y_col = false;
    S->mapcollision = false;
	num_collisions = 0;
    for(int a=0; a < 360; a += 2)
    {
		float ang = a * (3.14159/180.0f);
        float cx = (S->x + S->image->w/2) + cos( ang ) * 8;
        float cy = (S->y + S->image->h/2) + sin( ang ) * 8;
        player_level_collision((int)cx, (int)cy, ang);
    }
    //do_circle(level, S->x+S->image->w/2, S->y+S->image->h/2, 8, 0, player_level_collision);
	if(num_collisions > 15)
	{
		if(had_x_col)
		{        
			S->x = S->lastx;
			S->xv = -S->xv*player->bounce;
		}
		if(had_y_col)
		{
			S->y = S->lasty;
			S->yv = -S->yv*S->bounce;
		}
	}
	else
		destroy_terrain((S->x + S->image->w/2), (S->y + S->image->h/2), 8);
    
	// Frozen?
	if(player->enemydata.frozen > 0)
	{
		player->xv = 0;
		player->yv = 0;
	}

    S->lastx = S->x;
    S->lasty = S->y;
    S->x += S->xv;
    S->y += S->yv;
    S->xv *= S->friction;
    S->yv *= S->friction;
    
    // Thruster particles
	int tr=1,tg=1,tb=1;
	switch(player->playerdata.shiptype)
	{
		case 1: tr = 32; tg = 32; tb = 128; break;
		case 2: tr = 128; tg = 32; tb = 32; break;
		case 3: tr = 128; tg = 32; tb = 128; break;
		case 4: tr = 128; tg = 85; tb = 32; break;
		case 5: tr = 32; tg = 128; tb = 128; break;
	}


    float ang = (S->rot - 90) * (3.14159f/180.0f);
    float sx = (S->x + S->image->w/2) + cos(ang) * 6;
    float sy = (S->y + S->image->h/2) + sin(ang) * 6;
    float ang2 = (S->rot - 180) * (3.14159f/180.0f);
    sx += cos(ang2) * 8;
    sy += sin(ang2) * 8;
    spawn_particles(2, sx,sy, 3, tr,tg,tb,255, tr,tg,tb,32,
                    S->rot-5-180,S->rot+5-180, 0.3f,0.6f, 0.998f, 24, false, BLEND_ADD);
                    
    ang = (S->rot + 90) * (3.14159f/180.0f);
    sx = (S->x + S->image->w/2) + cos(ang) * 6;
    sy = (S->y + S->image->h/2) + sin(ang) * 6;
    ang2 = (S->rot - 180) * (3.14159f/180.0f);
    sx += cos(ang2) * 8;
    sy += sin(ang2) * 8;
    spawn_particles(2, sx,sy, 3, tr,tg,tb,255, tr,tg,tb,32,
                    S->rot-5-180,S->rot+5-180, 0.3f,0.6f, 0.998f, 24, false, BLEND_ADD);

	// Check if hit enemy
	float testx = S->x + S->image->w / 2;
	float testy = S->y + S->image->h / 2;
	SPRITE *E = spritelist;
	while(E != NULL)
	{
		if(E->type == SPRITE_ENEMY)
		{
			if(E == boss && E->enemydata.mode == BMODE_DIE)
			{
				E = E->next;
				continue;
			}

			if( hit_enemy(E, testx, testy) )
			{
				float speed = sqrt(S->xv*S->xv + S->yv*S->yv) + sqrt(E->xv*E->xv + E->yv*E->yv);
				screen_shake = speed * 10;
				damage_player(speed);
				S->xv = -E->xv*2;
				S->yv = -E->yv*2;
				apply_pressure(E, 8, 10, testx,testy);
				spawn_particles(10, testx, testy, 10, 255,32,32,255, 255,255,32,32, 0,359, 0.4f,0.6f, 1, 32, true, BLEND_ADD);
				spawn_lightflash(testx,testy, 2, 18, makecol(255,128,0), 0.5f);
				break;
			}
		}
		else
			break;
		E = E->next;
	}

	// Death check
	if(player_health <= 0 && player->playerdata.alive)
	{
		// Upon-death values
		player->playerdata.lives--;
		player_health = 0;
		player->enemydata.frozen = 0;
		player->xv = player->yv = 0;
		player->playerdata.alive = false;
		player->playerdata.rx = player->x;
		player->playerdata.ry = player->y;
		respawn_wait = 240;

		// Decide on good (and semi-distant) respawn location
		float dx = player->playerdata.rx - player->x;
		float dy = player->playerdata.ry - player->y;
		while( sqrt( dx*dx + dy*dy ) <= 500 )
		{
			player->playerdata.rx = random(level->w-64)+32;
			player->playerdata.ry = random(level->h-64)+32;
			dx = player->playerdata.rx - player->x;
			dy = player->playerdata.ry - player->y;
		}

		// Visual effects
		explosion(player->x+player->image->w/2,player->y+player->image->h/2, 48, 3);
		spawn_particles(50, testx, testy, 10, 255,32,32,255, 255,255,32,32, 0,359, 0.1f,0.6f, 1, 64, true, BLEND_ADD);

		perform_fade(4, makecol(0,255,255));
	}
}


void player_level_collision(int x, int y, float ang)
{
    int cx = ((int)player->lastx + player->image->w/2) + cos(ang)*7;
    int cy = ((int)player->lasty + player->image->h/2) + sin(ang)*7;
    
	bool col=false;

    if( getpixel(level,x,cy) != makecol(255,0,255) )
    {
        had_x_col = true;
        player->mapcollision = true;
		col = true;
    }
    if( getpixel(level,cx,y) != makecol(255,0,255) )
    {
        had_y_col = true;
        player->mapcollision = true;
		col = true;
    }

	if(col)
		num_collisions++;
}


void set_ship_type(int s)
{
	player->playerdata.shiptype = s;
	player->thrust = shipdata[s-1].thrust;
	player->topspeed = shipdata[s-1].topspeed;
    player->image = ships[s-1];
}


void damage_player(int dam)
{
	player_health -= dam * shipdata[player->playerdata.shiptype-1].hullstrength;
}
