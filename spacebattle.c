#include "jpf.h"
#include "spaceship.h"
#include "app.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "jeq.h"
#include "world.h"
#include "squares.h"
#include "fixed.h"
#include "draw.h"
#include "timer.h"

#define MAX_USR (5)

struct User {
	jpfusr_t usr;
	struct Spaceship * s;
};


// one app, no vhost handling
static struct {
  struct User users[MAX_USR];
}g_app;

static const int solid_col_sigs[] = {
	COLSIG_PLAYER,
	COLSIG_FIXED,
	COLSIG_STONE,
	-1
};

void jpf_init()
{
  // Make it possible to call several times
  static int is_inited = 0;
  if(is_inited) {
    return;
  }

  jeq_init(NOF_SUBS);
  Sq_init();
  fx_init();
  draw_init();

  is_inited = 1;

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
	} else {
		//printf("New user rejected!\n");
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

void jpf_on_tick(void)
{
void * h = 0;
	timer_tick();
	for(int i = 0; i < MAX_USR; i++) {
		if(0 != g_app.users[i].usr) {
			Spaceship_tick(g_app.users[i].s, h);
		}
	}
	jeq_broadcast(EVT_TICK, 0);
	while (!jeq_dispatch());
}


void jpf_on_draw(jpfhandle_t h)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(0 != g_app.users[i].usr) {
			int r = Spaceship_draw(g_app.users[i].s, h);
			if(!r)g_app.users[i].usr= 0;
		}
	}
	fx_draw(h);
	draw_draw(h);
}


