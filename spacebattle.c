#include "jpf.h"
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
};

enum spaceshipstate {flying, broken};
enum laserstate {active, inactive};
struct User {
	int sub;
	jpfusr_t usr;
        float x;
        float y;
	float vel_x;
	float vel_y;
	float angle;
        int w;
        int h;
        int inited;
        enum spaceshipstate state;
        int spid_spaceship;
        int spid_broken;
        struct {
            float x;
            float y;
            float angle;
            int tmo;
            enum laserstate state;
            int hidden;
            int spid;
        } laser;
};


void User_print(struct User * me)
{

#if 0
  printf(
        "x=%i\t"
        "y=%i\t"
	"vel_x=%f\t"
	"vel_y=%f\t"
	"angle=%f\n",
        (int)me->x,
 	(int)me->y,
 	me->vel_x,
 	me->vel_y,
 	me->angle);
#endif
}


// typedef void(*dispatch_f)(int event, void * data);

void on_dispatch(void * receiver, int ev, void * data)
{
	struct User * me = receiver;
	printf("on_dispatch e=%lu u=%d sub=%d\n", ev , me->usr, me->sub);
}

void User_init(struct User * me, jpfusr_t usr)
{
        me->usr = usr;
		me->x = usr%MAX_X;
        	me->y = usr%MAX_Y;
                me->vel_x = 0.0;
		me->vel_y = 0.0;
                me->angle = 0.0;
                me->h = MAX_Y;
		me->w = MAX_X;
                me->inited = 1;
                me->state = flying;
                me->laser.tmo = 0;
                me->laser.state = inactive;

	me->spid_spaceship = jpf_create_sprite("spaceship.png");
	me->spid_broken = jpf_create_sprite("broken.png");
	me->laser.spid = jpf_create_sprite("laser.png");
	me->sub =  jeq_subscribe(on_dispatch, me);

}

void User_deinit(struct User * me)
{
    me->inited = 0;
    jpf_release_sprite(me->spid_spaceship);
    jpf_release_sprite(me->spid_broken);
    jpf_release_sprite(me->laser.spid);
}

static void User_fire(struct User * me)
{
    if(broken == me->state)return;
    if(me->laser.tmo)return;
    me->laser.tmo = 20;
    me->laser.x = me->x;
    me->laser.y = me->y;
    me->laser.angle = me->angle;
    me->laser.state = active;
    me->laser.hidden = 0;
}

static void User_destroy(struct User * me)
{
  me->state = broken;
}

static void User_turn_left(struct User * me)
{
    me->angle -= 4.5;
}

static void User_turn_right(struct User * me)
{
    me->angle += 4.5;
}

static void User_accelerate(struct User * me)
{
    me->vel_x += cos(me->angle * PI/180.0) * 0.5;
    me->vel_y += sin(me->angle * PI/180.0) * 0.5;
}

static void User_decelerate(struct User * me)
{
    me->vel_x -= cos(me->angle * PI/180.0) * 0.2;
    me->vel_y -= sin(me->angle * PI/180.0) * 0.2;
}

static void User_prune(struct User * me)
{
  if(me->x < 0.0)me->x += me->w;
  if(me->x > me->w)me->x -= me->w;
  if(me->y < 0.0)me->y += me->h;
  if(me->y > me->h)me->y -= me->h;
}

static void User_move(struct User * me)
{
    switch(me->state) {
    case flying:
    me->x += me->vel_x;
    me->y += me->vel_y;
    User_prune(me);
//    me->x %= me->w;
//    me->y %= me->h;
    me->vel_x *= 0.95;
    me->vel_y *= 0.95;
    break;
    case broken:
    break;
    }
    User_print(me);
}


















void User_tick(struct User * me, jpfhandle_t h)//remove?
{
    if(me->laser.tmo)me->laser.tmo--;
    if(active == me->laser.state) {
        me->laser.x += 15*cos(me->laser.angle * PI/180.0);
        me->laser.y += 15*sin(me->laser.angle * PI/180.0);
        if(me->laser.x < 0 || me->laser.x > MAX_X || me->laser.y < 0 || me->laser.y > MAX_Y) {
            me->laser.state = inactive;
        }
    }
//  me->x += 1;
//  me->x %= MAX_X;
    if(broken == me->state)return;

//  KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
    int p = 0; 
    struct {char k[NROF_KEYEVT + 1];} keys = {{'.', '.', '.', '.', '.', '\0'}};

    if(is_key(me->usr, KEY_W)) {
      p += 1;
      User_accelerate(me);
      keys.k[KEY_W] = 'x';
    }
    if(is_key(me->usr, KEY_A)) {
      p += 1;
      User_turn_left(me);
      keys.k[KEY_A] = 'x';
    }
    if(is_key(me->usr, KEY_S)) {
      p += 1;
      User_decelerate(me);
      keys.k[KEY_S] = 'x';
    }
    if(is_key(me->usr, KEY_D)) {
      p += 1;
      User_turn_right(me);
      keys.k[KEY_D] = 'x';
    }
    if(is_key(me->usr, KEY_SPACE)) {
//      p += 1;
      User_fire(me);
      keys.k[KEY_SPACE] = 'x';
    }
	if (p)
		User_move(me);

	// void jeq_sendto(int evid, void * data, int dest);
	if (p)
		printf("%s", keys.k);

#if 0
  //  KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
	for (int k = 0; k < NROF_KEYEVT; k++) {
		if (keys.k[k] == 'x') {
			// void jeq_sendto(int evid, void * data, int dest);
			
		}
	}
#endif


	if (is_key(me->usr, KEY_W)) {
		jeq_sendto(EVT_W, 0, me->sub);
	}
	if (is_key(me->usr, KEY_A)) {
		jeq_sendto(EVT_A, 0, me->sub);
	}
	if (is_key(me->usr, KEY_S)) {
		jeq_sendto(EVT_S, 0, me->sub);
	}
	if (is_key(me->usr, KEY_D)) {
		jeq_sendto(EVT_D, 0, me->sub);
	}
	if (is_key(me->usr, KEY_SPACE)) {
		jeq_sendto(EVT_SPACE, 0, me->sub);
	}



	while (!jeq_dispatch());

}

int User_draw(struct User * me, jpfhandle_t h)
{
   if(active == me->laser.state) {
       jpf_draw_sprite(h, me->laser.spid, me->laser.x, me->laser.y, me->laser.angle);
    }

//{ "Id":102, "x":80, "y":120 }
    if(me->inited) {
        switch(me->state) {
        case flying:
        jpf_draw_sprite(h, me->spid_spaceship, me->x, me->y, me->angle);
        break;
        case broken:
        jpf_draw_sprite(h, me->spid_broken, me->x, me->y, me->angle);
        break;
        }
    }
    return me->inited;
}

int User_get_coordinates(struct User * me, float * x, float * y, float * lasx, float * lasy)
{
    *x = me->x;
    *y = me->y;
    *lasx = me->laser.x;
    *lasy = me->laser.y;
    return active == me->laser.state;
}
 


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
		User_init(p, usr);
	} else {
		printf("New user rejected!\n");
		return; // Ths will leak memory (pss)
	}
}

void jpf_on_remove_user(jpfusr_t usr)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(usr == g_app.users[i].usr) {
                        User_deinit(&g_app.users[i]);
//			g_app.users[i].usr = 0;
		}
	}
}

void jpf_on_tick(jpfhandle_t h)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(0 != g_app.users[i].usr) {
			User_tick(&g_app.users[i], h);
		}
	}
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
}


void jpf_on_draw(jpfhandle_t h)
{
	for(int i = 0; i < MAX_USR; i++) {
		if(0 != g_app.users[i].usr) {
			int r = User_draw(&g_app.users[i], h);
                        if(!r)g_app.users[i].usr= 0;
		}
	}
}










