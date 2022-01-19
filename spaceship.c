#include "jpf.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "spaceship.h"
#include "jeq.h"



static void Spaceship_turn_left(struct Spaceship * me);
static void Spaceship_turn_right(struct Spaceship * me);
static void Spaceship_accelerate(struct Spaceship * me);
static void Spaceship_decelerate(struct Spaceship * me);
static void Spaceship_fire(struct Spaceship * me);
static void Spaceship_move(struct Spaceship * me);

////
void Spaceship_tick_2(struct Spaceship * me, jpfhandle_t h);



/* Utility */
static int sgn(int x)
{
	if (x < 0)return -1;
	if (x > 0)return 1;
	return 0;
}

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
enum spaceshipstate {flying, broken};
enum laserstate {active, inactive};
struct Spaceship {
	int sub;
	jpfusr_t usr;
        float x;
        float y;
		struct {
			float x;
			float y;
		}old;
	float vel_x;
	float vel_y;
	float angle;
        int w;
        int h;
        int inited;
        enum spaceshipstate state;
        int spid_spaceship;
        int spid_broken;
        struct Laser {
			int sub;
            float x;
            float y;
            float angle;
            int tmo;
            enum laserstate state;
            int hidden;
            int spid;
        } laser;
};


void Spaceship_print(struct Spaceship * me)
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

static void on_dispatch(void * receiver, int ev, void * data)
{
	struct Spaceship * me = receiver;
	//printf("on_dispatch e=%lu u=%d sub=%d\n", ev , me->usr, me->sub);

	switch(ev) {
	case EVT_W:
		Spaceship_accelerate(me);
		break;
	case EVT_A:
		Spaceship_turn_left(me);
		break;
	case EVT_S:
		Spaceship_decelerate(me);
		break;
	case EVT_D:
		Spaceship_turn_right(me);
		break;
	case EVT_SPACE:
		Spaceship_fire(me);
		break;
	case EVT_TICK:
		Spaceship_tick_2(me, 0);
		Spaceship_move(me);
		break;
	case EVT_TREAD_DENIED:
		printf("tread denied\n");
		struct TreadRefuse * tr = data;
		me->x = tr->x;
		me->y = tr->y;
		me->vel_x = -1*sgn(me->vel_x)*0.5;
		me->vel_y = -1*sgn(me->vel_y)*0.5;
		break;
	case EVT_COLLISION:
		struct CollisionData * cd = data;
		printf("SPACESHIP collision %d\n", cd->id);
		if (cd->id != me->laser.sub) {
			me->state = broken;
		} else {
			printf("self!\n");
		}
		break;
	}
}


static void on_dispatch_laser(void * receiver, int ev, void * data)
{
	struct Laser * me = receiver;
	//printf("on_dispatch e=%lu u=%d sub=%d\n", ev , me->usr, me->sub);

	switch (ev) {
	case EVT_TICK:
//		Spaceship_tick_2(me, 0);
//		Spaceship_move(me);
		break;

	case EVT_COLLISION:
		struct CollisionData * cd = data;
		printf("laser_ collision %d\n", cd->id);
		break;

	}
}




void Spaceship_init(struct Spaceship * me, jpfusr_t usr)
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
	me->laser.sub = jeq_subscribe(on_dispatch_laser, &me->laser);

}

void Spaceship_deinit(struct Spaceship * me)
{
    me->inited = 0;
    jpf_release_sprite(me->spid_spaceship);
    jpf_release_sprite(me->spid_broken);
    jpf_release_sprite(me->laser.spid);
}

static void Spaceship_fire(struct Spaceship * me)
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

static void Spaceship_destroy(struct Spaceship * me)
{
  me->state = broken;
}

static void Spaceship_turn_left(struct Spaceship * me)
{
    me->angle -= 4.5;
}

static void Spaceship_turn_right(struct Spaceship * me)
{
    me->angle += 4.5;
}

static void Spaceship_accelerate(struct Spaceship * me)
{
    me->vel_x += cos(me->angle * PI/180.0) * 0.5;
    me->vel_y += sin(me->angle * PI/180.0) * 0.5;
}

static void Spaceship_decelerate(struct Spaceship * me)
{
    me->vel_x -= cos(me->angle * PI/180.0) * 0.2;
    me->vel_y -= sin(me->angle * PI/180.0) * 0.2;
}

static void Spaceship_prune(struct Spaceship * me)
{
  if(me->x < 0.0)me->x += me->w;
  if(me->x > me->w)me->x -= me->w;
  if(me->y < 0.0)me->y += me->h;
  if(me->y > me->h)me->y -= me->h;
}

static void Spaceship_move(struct Spaceship * me)
{
    switch(me->state) {
    case flying:
    me->x += me->vel_x;
    me->y += me->vel_y;
    Spaceship_prune(me);
//    me->x %= me->w;
//    me->y %= me->h;
    me->vel_x *= 0.95;
    me->vel_y *= 0.95;
    break;
    case broken:
    break;
    }
    Spaceship_print(me);
	//if ((abs(me->x - me->old.x) > 0.0001) || (abs(me->y - me->old.y) > 0.0001)) {
	if (abs(me->x) - abs(me->old.x) || abs(me->y) != abs(me->old.y)) {

		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_PLAYER;
		p->id = me->sub;
		p->x = me->x;
		p->y = me->y;
		jeq_sendto(EVT_TREAD, p, WORLD);
		me->old.x = me->x;
		me->old.y = me->y;
		//printf("%f %f %f %f\n", me->x, me->old.x, me->y, me->old.y);


	}
}



void Spaceship_tick(struct Spaceship * me, jpfhandle_t h)//remove?
{}

void Spaceship_tick_2(struct Spaceship * me, jpfhandle_t h)//remove?
{
    if(me->laser.tmo)me->laser.tmo--;
    if(active == me->laser.state) {
        me->laser.x += 15*cos(me->laser.angle * PI/180.0);
        me->laser.y += 15*sin(me->laser.angle * PI/180.0);
        if(me->laser.x < 0 || me->laser.x > MAX_X || me->laser.y < 0 || me->laser.y > MAX_Y) {
            me->laser.state = inactive;
        }


		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_LASER;
		p->id = me->laser.sub;
		p->x = me->laser.x;////
		p->y = me->laser.y;/////
		jeq_sendto(EVT_TREAD, p, WORLD);

    }
//  me->x += 1;
//  me->x %= MAX_X;
    if(broken == me->state)return;

//  KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
    int p = 0; 
    struct {char k[NROF_KEYEVT + 1];} keys = {{'.', '.', '.', '.', '.', '\0'}};

	if(is_key(me->usr, KEY_W)) {
      p += 1;
      keys.k[KEY_W] = 'x';
    }
    if(is_key(me->usr, KEY_A)) {
      p += 1;
      keys.k[KEY_A] = 'x';
    }
    if(is_key(me->usr, KEY_S)) {
      p += 1;
      keys.k[KEY_S] = 'x';
    }
    if(is_key(me->usr, KEY_D)) {
      p += 1;
      keys.k[KEY_D] = 'x';
    }
    if(is_key(me->usr, KEY_SPACE)) {
      p += 1;
      keys.k[KEY_SPACE] = 'x';
    }
	if (p) {
		//printf("%s", keys.k);
	}



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
#if 0
	jeq_sendto(EVT_TICK, 0, me->sub);

	while (!jeq_dispatch());
#endif
}

int Spaceship_draw(struct Spaceship * me, jpfhandle_t h)
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

int Spaceship_get_coordinates(struct Spaceship * me, float * x, float * y, float * lasx, float * lasy)
{
    *x = me->x;
    *y = me->y;
    *lasx = me->laser.x;
    *lasy = me->laser.y;
    return active == me->laser.state;
}
 

struct Spaceship *  new_Spaceship(jpfusr_t usr)
{

	struct Spaceship * me = malloc(sizeof(struct Spaceship));
	Spaceship_init(me, usr);

	return me;
}


void free_Spaceship(struct Spaceship * me)
{
	Spaceship_deinit(me);
	free(me);
	return;
}


