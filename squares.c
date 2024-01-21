
#include "jpfappdefines.h"
#include "squares.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>

struct List { int n; int iterator; struct SqCoord coord[NSQW*NSQH]; };

// Global variables
static const char * g_char_to_index_map_p;
static int nchar;
struct List * g_list_of_lists;

static int char_to_index(char c)
{
	int i = 0;
	while(1) {
		if(c == g_char_to_index_map_p[i])
		{
			return i;
		}
		i++;
		assert(i < nchar);
	} 
}

void Sq_init(const char* pattern, const char* char_to_index_map, int n)
{
	g_char_to_index_map_p = char_to_index_map;
	nchar = n;
	g_list_of_lists = malloc(sizeof(struct List) * nchar);

	for (int i = 0; i < nchar; i++)
	{
		struct List l0 = { 0 };
		g_list_of_lists[i] = l0;
		for (int j = 0; j < NSQH*NSQW; j++) {
			if (pattern[j] == g_char_to_index_map_p[i]) {
				
				g_list_of_lists[i].coord[g_list_of_lists[i].n].x = (j % NSQW)*32;
				g_list_of_lists[i].coord[g_list_of_lists[i].n].y = (j / NSQW)*32;
				g_list_of_lists[i].n++;
			}
		}
	}
}

int Sq_getNextCoord(char c, int iter, struct SqCoord * p)
{
	int i = char_to_index(c);
	*p = g_list_of_lists[i].coord[iter];
	int ret = (iter + 1) % g_list_of_lists[i].n;
	return ret;
}
