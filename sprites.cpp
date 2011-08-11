#include <math.h>
#include "sprites.h"
#include "map.h"
#include "main.h"
#include "player.h"
#include "utils.h"
#include "enemy.h"
#include "menu.h"

SPRITE *lightlist=NULL;
SPRITE *spritelist=NULL;
SPRITE *particlelist=NULL;
int spf=0;
SPRITE *boss=NULL;



void add_sprite(SPRITE *S)
{
    if(spritelist == NULL)
    {
        spritelist = S;
        S->next = NULL;
        return;
    }
    
    // Add enemies to the front, for optimization
	if(S->type == SPRITE_ENEMY)
	{
		SPRITE *E = spritelist;
		S->next = E;
		spritelist = S;
		return;
	}
	
	SPRITE *E = spritelist;
    for(; E->next != NULL; E = E->next)
    {
    }
    E->next = S;
    S->next = NULL;
}
void add_particle(SPRITE *S)
{
    if(particlelist == NULL)
    {
        particlelist = S;
        S->next = NULL;
        return;
    }

	SPRITE *E = particlelist;
	particlelist = S;
	S->next = E;
}
void add_light(SPRITE *S)
{
    if(lightlist == NULL)
    {
        lightlist = S;
        S->next = NULL;
        return;
    }

	SPRITE *E = lightlist;
	lightlist = S;
	S->next = E;
}


bool remove_sprite(SPRITE *S, SPRITE *cur)
{
    if(cur == NULL || S == NULL)
        return false;

	if(cur == S && !cur->next)
	{
		if(cur == spritelist) { spritelist = NULL; }
		if(cur == particlelist) { particlelist = NULL; }
		if(cur == lightlist) { lightlist = NULL; }
		delete cur;
		return true;
	}
  
    SPRITE *E = cur;
    while(E->next != NULL)
    {
        if(E->next == S)
        {
			E->next = S->next;
            delete S;
            S = NULL;
            return true;
        }
        E = E->next;
    }

    return false;
}


void draw_sprite(SPRITE *S)
{
    if(S == NULL || S->type == 0)
        return;
        
	if( absf(mapx - (int)S->x) < SCREEN_W*1.5 && absf(mapy - (int)S->y) < SCREEN_H*1.5 )
    {
        int dx = (int)S->x-mapx;
        int dy = (int)S->y-mapy;
        
        switch(S->type)
        {
            case SPRITE_PLAYER:
				if(!S->playerdata.alive) break;
            case SPRITE_MISSILE:
                rotate_sprite(backbuffer, S->image, dx,dy, itofix((int)(S->rot*(256.0f/360.0f))));
                break;
            case SPRITE_SCATTER_MISSILE:
                rotate_sprite(backbuffer, S->image, dx,dy, itofix((int)(S->rot*(256.0f/360.0f))));
                break;
            case SPRITE_PARTICLE:
                draw_particle(S);
                break;
            case SPRITE_BULLET:
                rectfill(backbuffer, dx-1,dy-1,dx+1,dy+1, makecol(random(100)+100,random(100)+100,random(100)+100));
                break;
            case SPRITE_ICE:
                int col;
				if(S->rot != 1)
				{
					if( random(100) < 50 )
						col = makecol(32,32,random(128)+128);
					else
						col = makecol(32,random(55)+200,255);
				}
				else
				{
					if( random(100) < 50 )
						col = makecol(random(128)+128,32,32);
					else
						col = makecol(255,random(55)+200,32);
				}
				for(int x=-1; x < 2; x++)
				{
					for(int y=-1; y < 2; y++)
						putpixel_alpha(backbuffer, dx+x,dy+y, col, 190);
				}

                break;
            case SPRITE_ENEMY:
                draw_enemy(S);
                break;
			case SPRITE_GLOB:
				draw_glob(S);
				break;
			case SPRITE_BEAM:
				draw_beam(S);
				break;
			case SPRITE_EXPLOSION:
			{
				if(S->lifetime > 20)
					break;
				int frame = (20-S->lifetime) / 4;
				for(int y=0; y < 24; y++)
				{
					for(int x=0; x < 24; x++)
					{
						int col = getpixel(explosion_trail, frame*24+x,y);
						if(col != makecol(255,0,255))
							putpixel_add(backbuffer, dx+x, dy+y, col, 255);
					}
				}
				break;
			}
			case SPRITE_SMOKE:
			{
				if(S->lifetime > 30)
					break;
				int frame = (30-S->lifetime) / 5;
				for(int y=0; y < 24; y++)
				{
					for(int x=0; x < 24; x++)
					{
						int col = getpixel(smoke_trail, frame*24+x,y);
						if(col != makecol(255,0,255))
							putpixel_alpha(backbuffer, dx+x, dy+y, col, 160);
					}
				}
				break;
			}
			case SPRITE_LIGHT_FLASH:
			{
				// HACK: For light flashes, 'xv/yv' is the current/max radius
				int radius = S->xv;
				float life = 1 - float(S->xv / S->yv);

				// HACK: Last light insists on sticking around for one extra draw-cycle, so need to cull it out :(
				if(S->xv >= S->yv-S->thrust)
					break;

				drawing_mode(DRAW_MODE_TRANS, NULL, 0,0);
				set_add_blender(255,255,255,96*life);
				
				circlefill(backbuffer, dx, dy, radius*1.5, S->col);
				circlefill(backbuffer, dx, dy, radius*1.25, S->col);
				circlefill(backbuffer, dx, dy, radius*1.0, S->col);

				drawing_mode(DRAW_MODE_SOLID, NULL, 0,0);

				break;
			}
			case SPRITE_BUBBLE:
			{
				int size = (int)S->rot*2;

				drawing_mode(DRAW_MODE_TRANS, NULL, 0,0);
				set_trans_blender(255,255,255,32);
				circlefill(backbuffer, dx+size/2, dy+size/2, S->rot, makecol(255,255,255));
				drawing_mode(DRAW_MODE_SOLID, NULL, 0,0);

				stretch_sprite(backbuffer, bubbleimage, dx, dy, size,size);
				circle(backbuffer, dx+size/2, dy+size/2, S->rot, makecol(0,0,0));
				break;
			}
        }
    }
}


void draw_sprites()
{
    for(SPRITE *S = particlelist; S != NULL; S = S->next)
    {
        draw_sprite(S);
    }
    for(SPRITE *S = spritelist; S != NULL; S = S->next)
    {
        draw_sprite(S);
    }

	draw_sprite(player);

	for(SPRITE *S = lightlist; S != NULL; S = S->next)
    {
		draw_sprite(S);
    }
}


void update_sprites()
{
    SPRITE *S = particlelist;
    while(S != NULL)
    {
        update_sprite(S);
        if(S->dead)
        {
            SPRITE *E = S->next;
            remove_sprite(S, particlelist);
            S = E;
            continue;
        }
        S = S->next;
    }

    S = spritelist;
    while(S != NULL)
    {
        update_sprite(S);
        if(S->dead)
        {
            SPRITE *E = S->next;
            remove_sprite(S, spritelist);
            S = E;
            continue;
        }
        S = S->next;
    }

    S = lightlist;
    while(S != NULL)
    {
        update_sprite(S);
        if(S->dead)
        {
            SPRITE *E = S->next;
            remove_sprite(S, lightlist);
            S = E;
            continue;
        }
        S = S->next;
    }
}


void update_sprite(SPRITE *S)
{
    if(S == NULL)
        return;
                
    spf++;
    switch(S->type)
    {
        case SPRITE_PLAYER:
            update_player(S);
            break;
        case SPRITE_PARTICLE:
            update_particle(S);
            if(S->lifetime <= 0)
                S->dead = true;
            break;
        case SPRITE_BULLET:
            for(int a=0; a < 6; a++)
            {
                update_bullet(S);
                if(S->lifetime == -1)
                {
                    S->dead = true;
                    return;
                }
            }
            break;
        case SPRITE_BLASTER:
            for(int a=0; a < 3; a++)
            {
                update_blaster(S);
                if(S->lifetime == -1)
                {
                    S->dead = true;
                    return;
                }
            }
            break;
        case SPRITE_MISSILE:
		case SPRITE_SCATTER_MISSILE:
            for(int a=0; a < 1; a++)
            {
                update_missile(S);
                if(S->lifetime == -1)
                {
                    S->dead = true;
                    return;
                }
            }
            break;
        case SPRITE_ICE:
            update_freeze_particle(S);
            if(S->lifetime <= 0)
               S->dead = true;
            break;
        case SPRITE_ICE_ROOT:
            update_ice_root(S);
            if(S->lifetime <= 0)
                S->dead = true;
            break;
        case SPRITE_ENEMY:
			update_enemy(S);
            break;
        case SPRITE_GLOB:
			update_glob(S);
            break;
        case SPRITE_ENEMY_BLASTER:
            for(int a=0; a < 2; a++)
            {
                update_enemy_blaster(S);
                if(S->lifetime == -1)
                {
                    S->dead = true;
                    return;
                }
            }
            break;
		case SPRITE_BEAM:
		{
			S->lifetime--;
			if(S->lifetime <= 0)
				S->dead = true;
			break;
		}
		case SPRITE_EXPLOSION:
		{
			S->lifetime--;
			if(S->lifetime == 20)
			{
				destroy_terrain(S->x,S->y, 16);
			}
			if(S->lifetime <= 0)
				S->dead = true;
			break;
		}
		case SPRITE_SMOKE:
		{
			S->lifetime--;
			if(S->lifetime <= 0)
				S->dead = true;
			break;
		}
		case SPRITE_LIGHT_FLASH:
		{
			// HACK: xv/yv = current/max radius, thrust = speed of increase
			S->xv += S->thrust;
			if(S->xv >= S->yv)
				S->dead = true;
			break;
		}
		case SPRITE_BUBBLE:
		{
			float ang = (gcount % 360 + S->thrust) * (3.14159f / 180.0f);
			S->xv += sin(ang)*0.01f;
			S->xv *= 0.99f;
			S->yv *= 0.99f;
			S->x += S->xv;
			S->y += S->yv;
			S->yv -= 0.02f;
			if(S->yv <= -2.5f)
				S->yv = -2.5f;

			int tx = S->x + S->rot;
			int ty = S->y + S->rot;

			S->lifetime--;
			if(S->lifetime <= 0)
			{
				S->dead = true;
				if(S->rot >= 8)
					spawn_bubbles(S->rot, tx,ty, 3, 5.5f, S->rot*0.45f);
			}
			/*
			if(getpixel(level,tx,ty) != makecol(255,0,255))
			{
				if( sqrt(S->xv*S->xv + S->yv*S->yv) >= 1.5f )
				{
					pop_bubble(S, 5.5f);
				}
				else
				{
					if(getpixel(level,tx-S->xv,ty) != makecol(255,0,255))
					{
						S->x -= S->xv*2;
						S->xv = -S->xv*0.8f;
					}
					if(getpixel(level,tx,ty-S->yv) != makecol(255,0,255))
					{
						S->y -= S->yv*2;
						S->yv = -S->yv*0.8f;
					}
				}
			}*/
			break;
		}
    }    
}


void draw_particle(SPRITE *S)
{
    int dx = (int)S->x-mapx;
    int dy = (int)S->y-mapy;
    int r = (S->particledata.start_r * ((float)S->lifetime / (float)S->particledata.maxlifetime)) +
            (S->particledata.end_r * ((float)(S->particledata.maxlifetime-S->lifetime) / (float)S->particledata.maxlifetime));
    int g = (S->particledata.start_g * ((float)S->lifetime / (float)S->particledata.maxlifetime)) +
            (S->particledata.end_g * ((float)(S->particledata.maxlifetime-S->lifetime) / (float)S->particledata.maxlifetime));
    int b = (S->particledata.start_b * ((float)S->lifetime / (float)S->particledata.maxlifetime)) +
            (S->particledata.end_b * ((float)(S->particledata.maxlifetime-S->lifetime) / (float)S->particledata.maxlifetime));
    int a = (S->particledata.start_a * ((float)S->lifetime / (float)S->particledata.maxlifetime)) +
            (S->particledata.end_a * ((float)(S->particledata.maxlifetime-S->lifetime) / (float)S->particledata.maxlifetime));
    
    // Allow particle blending to be disabled (for low-end computers)
	if(!config[CFG_PARTICLE_BLENDING])
		S->particledata.blendmode = BLEND_NORMAL;

	for(int x=-1; x < 2; x++)
	{
		for(int y=-1; y < 2; y++)
		{
			if(S->particledata.blendmode == BLEND_NORMAL)
				putpixel(backbuffer, dx+x,dy+y, makecol(r,g,b));
			else if(S->particledata.blendmode == BLEND_ALPHA)
				putpixel_alpha(backbuffer, dx+x,dy+y, makecol(r,g,b), a);
			else if(S->particledata.blendmode == BLEND_ADD)
				putpixel_add(backbuffer, dx+x,dy+y, makecol(r,g,b), a);
		}
	}
}


void spawn_particles(int num, float x, float y, int rand_radius, int sr, int sg, int sb, int sa,
					 int er, int eg, int eb, int ea, int minang, int maxang, float minspeed,
					 float maxspeed, float friction, int lifetime, bool sprites, int bmode)
{
	// CULLING FTW!
	if( absf(mapx - x) >= SCREEN_W*1.5 || absf(mapy - y) >= SCREEN_H*1.5 )
		return;

    for(int a=0; a < num; a++)
    {
        SPRITE *P = new SPRITE();
        P->type = SPRITE_PARTICLE;
        
        float ang1 = random(360) * (3.14159f/180.0f);
        int rad = random(rand_radius);
        P->x = x + (cos(ang1) * rad);
        P->y = y + (sin(ang1) * rad);
        
        P->particledata.blendmode = bmode;
		P->particledata.start_r = sr;
        P->particledata.start_g = sg;
        P->particledata.start_b = sb;
        P->particledata.start_a = sa;
        P->particledata.end_r = er;
        P->particledata.end_g = eg;
        P->particledata.end_b = eb;
        P->particledata.end_a = ea;
        
        float ang2 = (float)(random(maxang-minang)+minang) * (3.14159f/180.0f);
        int minsp = minspeed * 10000;
        int maxsp = maxspeed * 10000;
        float speed = (float)(random(maxsp-minsp)+minsp) / 10000.0f;
        P->xv = cos(ang2) * speed;
        P->yv = sin(ang2) * speed;
        
        P->friction = friction;
        P->lifetime = lifetime;
        P->particledata.maxlifetime = lifetime;
        
        if(!sprites)
            add_particle(P);
        else
            add_sprite(P);
    }
}

void spawn_particles(int num, float x, float y, int rand_radius, int sr, int sg, int sb,
					 int er, int eg, int eb, int minang, int maxang, float minspeed,
					 float maxspeed, float friction, int lifetime, bool sprites)
{
	spawn_particles(num, x, y, rand_radius, sr,sg,sb,255, er,eg,eb,255, minang,maxang, minspeed,maxspeed, friction, lifetime, sprites, BLEND_NORMAL);
}

void update_particle(SPRITE *S)
{
    S->x += S->xv;
    S->y += S->yv;
    S->xv *= S->friction;
    S->yv *= S->friction;
    
    //if( getpixel(level, S->x,S->y) != makecol(255,0,255) )
      //S->lifetime = 0;

    S->lifetime--;
}


void explosion(float x, float y, int size, int duration)
{
	play_sound(boom_sound, x, y);

	int num = (size / 3) * duration;

	spawn_lightflash(x,y, 5, size*1.5, makecol(255,128,0), float(size*1.35) / float(duration*15));

	// Generate particles (dormant+active)
	for(int a=0; a < num; a++)
	{
		float timeframe = random(100) / 100.0f;
		float dist = size * timeframe;
		float angle = random(360) * (3.14159f / 180.0f);

		SPRITE *E = new SPRITE();
		E->x = x + cos(angle) * dist - 12;
		E->y = y + sin(angle) * dist - 12;
		E->type = SPRITE_EXPLOSION;
		E->lifetime = 20 + (duration-1)*10 * timeframe;
		add_sprite(E);

		
		timeframe = random(100) / 100.0f;
		dist = size * timeframe;
		angle = random(360) * (3.14159f / 180.0f);

		SPRITE *S = new SPRITE();
		S->x = x + cos(angle) * dist - 12;
		S->y = y + sin(angle) * dist - 12;
		S->type = SPRITE_SMOKE;
		S->lifetime = 30 + (duration-1)*10 * timeframe;
		add_sprite(S);
	}
	int bub_num = maxf(0, size / 14);
	spawn_bubbles(bub_num, x, y, size/duration, size*4/(duration), 16);

	// Calculate screen-shake effect
	float dx = (player->x + player->image->w/2) - x;
	float dy = (player->y + player->image->h/2) - y;
	float dist = sqrt(dx*dx + dy*dy);
	if(dist <= size*1.5f)
	{
		float force = absf(48-dist);
		if(size > 16)
			screen_shake = force*2;

		damage_player(screen_shake * 0.15f);

		float ang = atan2(dy,dx);
		player->xv += cos(ang)*force;
		player->yv += sin(ang)*force;
	}

	// Apply push-back to nearby (unfortunate) foes
	SPRITE *E = spritelist;
	while(E != NULL)
	{
		if(E->type == SPRITE_ENEMY)
		{
			dx = E->x - x;
			dy = E->y - y;
			dist = sqrt(dx*dx + dy*dy);
			float maxdist = size*5;
			if( dist <= maxdist )
			{
				float amt = 1 - (dist / maxdist);  // Perctentage of impact
				apply_pressure(E, 0.2f*amt, 9*amt, x,y);
				
				float force = 64*amt;
				float ang = atan2(dy,dx);
				E->xv += cos(ang)*force;
				E->yv += sin(ang)*force;
				apply_damage(E, force * 0.3f);
			}
		}
		else
			break;
		E = E->next;
	}
}


void spawn_lightflash(int x, int y, int rad, int maxrad, int col, float speed)
{
	if(!config[CFG_LIGHT_FLASHES])
		return;

	// CULLING FTW!
	if( absf(mapx - x) >= SCREEN_W*1.5 || absf(mapy - y) >= SCREEN_H*1.5 )
		return;

    SPRITE *P = new SPRITE();
    P->type = SPRITE_LIGHT_FLASH;    
    P->x = x;
    P->y = y;
	P->col = col;
	P->xv = rad;
	P->yv = maxrad;
	P->thrust = speed;

    add_light(P);
}



void spawn_bubbles(int num, int x, int y, int distrib, float force, int size)
{
	// CULLING FTW!
	if( absf(mapx - x) >= SCREEN_W*1.5 || absf(mapy - y) >= SCREEN_H*1.5 )
		return;

	for(int a=0; a < num; a++)
	{
		SPRITE *B = new SPRITE();
		B->type = SPRITE_BUBBLE;

		float ang = (float)random(360) * (3.14159f / 180.0f);
		float dist = random(distrib);
		B->x = x + cos(ang)*dist;
		B->y = y + sin(ang)*dist;

		ang = (float)random(360) * (3.14159f / 180.0f);
		dist = (float)random(force*10000) / 10000.0f + 0.2f;
		B->xv = cos(ang)*dist;
		B->yv = sin(ang)*dist;

		B->rot = random(size/2)+size/2;
		B->thrust = random(32000);
		B->lifetime = 440 + random(60);

		add_sprite(B);
	}
	//create_globs(1, x,y, 0,359, 0.1f,0.2f, size*2.2f, 255,255,255);

	if(random(100) < 50)
		play_sound(pop1_sound, x, y);
	else
		play_sound(pop2_sound, x, y);
}


void pop_bubble(SPRITE *S, float force)
{
	if(!S || S->type != SPRITE_BUBBLE)
		return;

	S->dead = true;
	if(S->rot >= 8)
	{
		spawn_bubbles(S->rot, S->x+S->rot,S->y+S->rot, S->rot, force, S->rot*0.65f);
	}
}


bool hit_bubble(SPRITE *S, int x, int y)
{
	float dx = x - S->x;
	float dy = y - S->y;

	return sqrt(dx*dx+dy*dy) <= S->rot;
}
