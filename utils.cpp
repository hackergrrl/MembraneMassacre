#include "main.h"
#include "utils.h"


int random(int m)
{
	if(m == 0)
	{
		//allegro_message("Error: Div by zero (random function).");
		//exit(1);
		return 0;
	}
    return rand()%m;
}

/*
int max(int a, int b)
{
	return (a < b) ? b : a;
}


int min(int a, int b)
{
	return (a > b) ? b : a;
}
*/