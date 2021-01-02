#include "jpf.h"
#include "spaceship.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "jeq.h"

#define MAX_USR (10)


enum Event {
	EVT_W,
	EVT_A,
	EVT_S,
	EVT_D,
	EVT_SPACE,
	EVT_TICK,
};


struct User {
	jpfusr_t usr;
	struct Spaceship * s;
};


// one app, no vhost handling
static struct {
  struct User users[MAX_USR];
}g_app;

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


