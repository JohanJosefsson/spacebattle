#include "app.h"
#include "jeq.h"
#include "world.h"

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>


#define NW (50)
struct GWorld {
	int x;
	struct TreadData * pd[NW];
	const int * solid_col_sigs;
}g_world;


// Returns 1 if both collision signatures are in the solid list
static int is_solid(int s1, int s2) {
	int i = 0;
	int r1 = 0;
	int r2 = 0;
	while (g_world.solid_col_sigs[i] != -1) {
		int s = g_world.solid_col_sigs[i];
		r1 = r1 || (s1 == s);
		r2 = r2 || (s2 == s);
		if (r1 && r2)
			return 1;
		i++;
	}
	return 0;
}


static add_fixed(struct TreadData * p)
{
	for (int i = 0; i < NW; i++) {
		if (!g_world.pd[i]) {
			p->id = FIXED_SUB;
			printf("fixed %d %d\n", p->id, p->col_sig);
			struct TreadData * td = malloc(sizeof(struct TreadData));
			*td = *p;
			g_world.pd[i] = td;
			break;
		}
	}
}

static void tread(struct TreadData * p)
{
	int i0 = -1; // The users current index
	int cnt = 0; // logging
	for (int i = 0; i < NW; i++) {
		if (g_world.pd[i] && p->id == g_world.pd[i]->id) {
			i0 = i;
		}
		if (g_world.pd[i])cnt++;
	}
	//printf("World %d objects\n", cnt);

	int in = -1; // Index New
	int solid_collision = 0; // collision between two solids is detected

	// Loop through all indices
	for (int i = 0; i < NW; i++) {
		if (g_world.pd[i] && p->id == g_world.pd[i]->id) continue;
		// Skip if the index has no user (data)
		if (!g_world.pd[i]) {
			if (-1 == in) {
				in = i; // remember index for use if user is new
			}
			continue;
		}

		// Check for collision
		int adx = abs(p->x - g_world.pd[i]->x);
		int ady = abs(p->y - g_world.pd[i]->y);
		//printf(" { %d %d } ", adx, ady);
		if (adx <= 16 && ady <= 16) {
			//printf("COLLISION!\n");
			if (is_solid(g_world.pd[i]->col_sig, p->col_sig)) {
				solid_collision = 1;

				//assert(-1 != i0); // New object collides => crash
				if (-1 == i0)return; // N.b! This can make som funy behavior TODO
				                     // it seems to work but I dunno why


				struct TreadRefuse * tr = malloc(sizeof(struct TreadRefuse));
				tr->col_sig = g_world.pd[i]->col_sig;
				tr->x = g_world.pd[i0]->x;
				tr->y = g_world.pd[i0]->y;
				jeq_sendto(EVT_TREAD_DENIED, tr, g_world.pd[i0]->id);////
			} else {
				struct CollisionData * cd1 = malloc(sizeof(struct CollisionData));
				//cd1->col_sig = g_world.pd[i]->col_sig;
				cd1->id = g_world.pd[i]->id;
				jeq_sendto(g_world.pd[i]->col_sig, cd1, p->id);
				struct CollisionData * cd2 = malloc(sizeof(struct CollisionData));
				//cd2->col_sig = p->col_sig;
				cd2->id = p->id;
				jeq_sendto(p->col_sig, cd2, g_world.pd[i]->id);
			}

		}
	}

	// Assert that we have place for a new one
	assert(!(i0 == -1 && in == -1));

	if (-1 == i0) {
		struct TreadData * pp = malloc(sizeof(struct TreadData));
		*pp = *p;
		g_world.pd[in] = pp;
		//printf("adding  %d at index %d\n", p->id, in);
	}
	else {
		if (!solid_collision) {
			struct TreadData * pp = malloc(sizeof(struct TreadData));
			*pp = *p;
			free(g_world.pd[i0]);
			g_world.pd[i0] = pp;
			//printf("i0=%d\n", i0);
		}
		else {
			//printf("no move\n");
		}
	}
}


static void leave(struct LeaveData * p)
{
	//printf("leave() %d ", p->id);
	int i0 = -1; // The users current index
	for (int i = 0; i < NW; i++) {
		if (g_world.pd[i] && p->id == g_world.pd[i]->id) {
			//printf(" -> %d", g_world.pd[i]->id);
			free(g_world.pd[i]);
			g_world.pd[i] = 0;
		}
	}
	//printf(" .\n");

}





static void on_dispatch(void * receiver, int ev, void * data)
{
	switch (ev) {
	case EVT_TREAD:
		//if (((struct TreadData *)data)->col_sig == COLSIG_FIXED)printf("FIXED");
		tread((struct TreadData *)data);
		break;
	case EVT_LEAVE:
		leave((struct LeaveData *)data);
		break;
	case EVT_ADD_FIXED:
		add_fixed((struct TreadData *)data);
		break;
	}
}
void World_init(const int * solid_col_sigs)
{
	g_world.solid_col_sigs = solid_col_sigs;
	jeq_subscribe_res(WORLD, on_dispatch, &g_world);
}

