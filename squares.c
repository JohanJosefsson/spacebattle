
#include "squares.h"

#include <assert.h>


// 480 x 270 pixel
// 15 x 8 squares
#define NSQW (15)
#define NSQH (9)

static const char pattern[] =
/*
|012345678901234|       */
"               " //0
" V             " //1
"   O        V  " //2
"    O          " //3
"  V           O" //4
"       O      O" //5
"       O  V    " //6
"  V   O        " //7
"      OO       " //8
;

struct List { int n; int iterator; struct SqCoord coord[NSQW*NSQH]; };

static char char_to_index_map[] = { 'V', 'O' };

struct List list_of_lists[sizeof(char_to_index_map) / sizeof(char)];


static int char_to_index(char c)
{
	int i = 0;
	while(1) {
		if(c == char_to_index_map[i])
		{
			return i;
		}
		i++;
		assert(i < sizeof(char_to_index_map) / sizeof(char));
	} 
}

void Sq_init(void)
{
	for (int i = 0; i < sizeof(char_to_index_map) / sizeof(char); i++)
	{
		for (int j = 0; j < NSQH*NSQW; j++) {
			if (pattern[j] == char_to_index_map[i]) {
				
				list_of_lists[i].coord[list_of_lists[i].n].x = (j % NSQW)*32;
				list_of_lists[i].coord[list_of_lists[i].n].y = (j / NSQW)*32;
				list_of_lists[i].n++;
			}
		}
	}
}


int Sq_getNextCoord(char c, int iter, struct SqCoord * p)
{
	int i = char_to_index(c);
	*p = list_of_lists[i].coord[iter];
	return (++iter%list_of_lists[i].n);
}
