#ifndef _WEAPONS_H
#define _WEAPONS_H_

#include <allegro.h>

#define WEAPON_BLASTER      0
#define WEAPON_MACHINEGUNS  1
#define WEAPON_MISSILE      2
#define WEAPON_LASER        3
#define WEAPON_FREEZE_BOMB  4
#define MAX_WEAPONS         5

struct WEAPONDATA
{
    bool owned;
    int ammo, maxammo;
    int firerate, waitcount;
	int regenrate, regencount;
    int index;
};


void fire_weapon(int index);
void create_ice_root(int x, int y, float ang, int branches);

extern int curweapon;
extern WEAPONDATA weapons[MAX_WEAPONS];
extern int laser_delay;

#endif
