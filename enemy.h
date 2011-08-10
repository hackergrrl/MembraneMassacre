#ifndef _ENEMY_H_
#define _ENEMY_H_

#define MODE_IDLE	0
#define MODE_CHASE	1
#define MODE_FLEE	2
#define MODE_EAT	3
#define MODE_SHOOT	4

#define BMODE_SHOOT		0
#define BMODE_FBOMB		1
#define BMODE_SPAWN		2
#define BMODE_CHASE		3
#define BMODE_IDLE		4
#define BMODE_DIE		5

#define MAX_SPECIES		12

#define MAX_ENEMIES	100


SPRITE* create_enemy(SPRITE *parent, int x, int y, int genre);
void draw_enemy(SPRITE *S);
void apply_pressure(SPRITE *S, float amount, float width, int x, int y);
bool hit_enemy(SPRITE *S, int x, int y);
void apply_damage(SPRITE *S, int dam);
void update_glob(SPRITE *S);
void draw_glob(SPRITE *S);
void enemy_split(SPRITE *S);
void digest_circle(BITMAP *buf, int x, int y, int d);
void create_boss(int x, int y);
void create_globs(int num, int x, int y, float minang, float maxang, float minforce, float maxforce, int size, int r, int g, int b);
void enemy_change_radius(SPRITE *S, int newrad);


// Enemy species
#define SPECIES_MECHA		6
#define SPECIES_FREEZER		7
#define SPECIES_STALKER		8



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
	float thrust_mod;
	float top_speed;
	int idle_chance;
	int chase_chance;
	int flee_chance;
	int eat_chance;
	int shoot_chance;
	int max_generations;
	int min_level;
	int difficulty;
	int batch_size;
};


extern SPECIESDATA species[MAX_SPECIES];
extern int bmode_count;

#endif
