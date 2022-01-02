#include "jpf.h"
#include "spaceship.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "jeq.h"

#define MAX_USR (10)

/*
enum Event {
	EVT_W,
	EVT_A,
	EVT_S,
	EVT_D,
	EVT_SPACE,
	EVT_TICK,
};
*/

struct User {
	jpfusr_t usr;
	struct Spaceship * s;
};


// one app, no vhost handling
static struct {
  struct User users[MAX_USR];
}g_app;

////////////////////////////////////////////////////////////

#define NW (30)
struct GWorld{
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
		r1 = r1 || (s1 = s);
		r2 = r2 || (s2 = s);
		if(r1 && r2)return 1;
		i++;
	}
	return 0;
}

static void tread(struct TreadData * p)
{
	int i0 = -1; // The users current index
	for (int i = 0; i < NW; i++) {
		if (g_world.pd[i] && p->id == g_world.pd[i]->id) {
			i0 = i;
		}
	}


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
			if (is_solid(g_world.pd[i]->col_sig, g_world.pd[i0]->col_sig)) {
				solid_collision = 1;


				struct TreadRefuse * tr = malloc(sizeof(struct TreadRefuse));
				tr->col_sig = g_world.pd[i]->col_sig;
				tr->x = g_world.pd[i0]->x;
				tr->y = g_world.pd[i0]->y;
				jeq_sendto(EVT_TREAD_DENIED, tr, g_world.pd[i0]->id);////
			}
			 
		}
	}

	// Assert that we have place for a new one
	assert(!(i0 == -1 && in == -1));

	if (-1 == i0) {
		struct TreadData * pp = malloc(sizeof(struct TreadData));
		*pp = *p;
		g_world.pd[in] = pp;
		printf("adding  %d at index %d\n", p->id, in);
	} else {
		if (!solid_collision) {
			struct TreadData * pp = malloc(sizeof(struct TreadData));
			*pp = *p;
			free(g_world.pd[i0]);
			g_world.pd[i0] = pp;
		}
		else {
			printf("no move\n");
		}
	}




	//printf("%d %d %d\n", p->id, p->x, p->y);
}

//typedef void(*dispatch_f)(void * receiver, int event, void * data);
//static void on_dispatch(void * receiver, int ev, void * data)
static void on_dispatch(void * receiver, int ev, void * data)
{
	switch (ev) {
	case EVT_TREAD:
		tread((struct TreadData *)data);
		break;
	}
}
static void World_init(const int * solid_col_sigs)
{
	g_world.solid_col_sigs = solid_col_sigs;
	jeq_subscribe_res(WORLD, on_dispatch, &g_world);
}







////////////////////////////////////////////////////////////

static const int solid_col_sigs[] = {
	COLSIG_PLAYER,
	COLSIG_FIXED,
	-1
};

void jpf_init()
{
	jeq_init(NOF_SUBS);
	World_init(solid_col_sigs);
}

void jpf_on_new_user(jpfusr_t usr)
{
        struct User * p = 0;
	for(int i = 0; i < MAX_USR; i++) {
		if(0 == g_app.users[i].usr) {
                        p = &g_app.users[i];
		        break;
		}
	}
        if(p) {
			p->usr = usr;
			p->s = new_Spaceship(usr);
		//User_init(p, usr);
	} else {
		printf("New user rejected!\n");
		return; // Ths will leak memory (pss)
	}
}

void jpf_on_remove_user(jpfusr_t usr)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(usr == g_app.users[i].usr) {
                        //User_deinit(&g_app.users[i]);
				free_Spaceship(g_app.users[i].s);
				g_app.users[i].usr = 0;
		}
	}
}

void jpf_on_tick(jpfhandle_t h)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(0 != g_app.users[i].usr) {
//			User_tick(&g_app.users[i], h);
			Spaceship_tick(g_app.users[i].s, h);
		}
	}

	jeq_broadcast(EVT_TICK, 0);
	while (!jeq_dispatch());
#if 0
        // See if laser hits
        for(int i = 0; i <  MAX_USR; i++) {
                float x0, y0; // dummy
                float x, y, lasx, lasy;
                int a;
		if(0 == g_app.users[i].usr || !User_get_coordinates(&g_app.users[i], &x0, &x0, &lasx, &lasy)) {
                    continue;
                } else {
                for(int j = 0; j <  MAX_USR; j++) {
                    if(i == j)continue;
	            if(0 != g_app.users[j].usr) {
                        User_get_coordinates(&g_app.users[j], &x, &y, &x0, &y0);
                    } else {
                        continue;
                    }
                    //if((x - lasx)*(x - lasx) + (y - lasy)*(y - lasy) < 16.0*16.0) {
                    float d2 = (x - lasx)*(x - lasx) + (y - lasy)*(y - lasy);
                    if(d2 < 16.0*16.0) {
                        User_destroy(&g_app.users[j]);
                    }
                }
            }
        }
#endif
}


void jpf_on_draw(jpfhandle_t h)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(0 != g_app.users[i].usr) {
//			int r = User_draw(&g_app.users[i], h);
			int r = Spaceship_draw(g_app.users[i].s, h);
			if(!r)g_app.users[i].usr= 0;
		}
	}
}


