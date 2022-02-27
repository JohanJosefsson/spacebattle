#include "jpf.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "spaceship.h"
#include "spacebattle.h"
#include "jeq.h"
#include "jsm.h"
#include "squares.h"
#include "timer.h"

static void Spaceship_turn_left(struct Spaceship * me);
static void Spaceship_turn_right(struct Spaceship * me);
static void Spaceship_accelerate(struct Spaceship * me);
static void Spaceship_decelerate(struct Spaceship * me);
static void Spaceship_fire(struct Spaceship * me);
static void Spaceship_move(struct Spaceship * me);



/* JSM Statemachine

name = Spaceship
suffix = yes
top ->flying :h
  flying ->normal :h
    normal ->unprotected :he
      protected :he
      unprotected :he
    quarterbroken :he
    halfbroken :he
    threequarterbroken :he
  broken :he

*/

struct Spaceship;

enum SpaceshipState
{
top_s,
  flying_s,
    normal_s,
      protected_s,
      unprotected_s,
    quarterbroken_s,
    halfbroken_s,
    threequarterbroken_s,
  broken_s,
};

static const struct TopologyNode topology[] = {
  // id, super, descend
  {top_s, 0, flying_s},
    {flying_s, top_s, normal_s},
      {normal_s, flying_s, unprotected_s},
        {protected_s, normal_s},
        {unprotected_s, normal_s},
      {quarterbroken_s, flying_s},
      {halfbroken_s, flying_s},
      {threequarterbroken_s, flying_s},
    {broken_s, top_s}
};

// Fwd decl.
static int top_handler(struct Spaceship * me, int ev);
static int flying_handler(struct Spaceship * me, int ev);
static int normal_handler(struct Spaceship * me, int ev);
static void normal_on_entry(struct Spaceship * me);
static int protected_handler(struct Spaceship * me, int ev);
static void protected_on_entry(struct Spaceship * me);
static int unprotected_handler(struct Spaceship * me, int ev);
static void unprotected_on_entry(struct Spaceship * me);
static int quarterbroken_handler(struct Spaceship * me, int ev);
static void quarterbroken_on_entry(struct Spaceship * me);
static int halfbroken_handler(struct Spaceship * me, int ev);
static void halfbroken_on_entry(struct Spaceship * me);
static int threequarterbroken_handler(struct Spaceship * me, int ev);
static void threequarterbroken_on_entry(struct Spaceship * me);
static int broken_handler(struct Spaceship * me, int ev);
static void broken_on_entry(struct Spaceship * me);

static struct Statefuncs state_funcs[] = {
  // #, name, handler, entry, exit, init
  {top_s, "top", top_handler, 0,0,0,},
  {flying_s, "flying", flying_handler, 0,0,0,},
  {normal_s, "normal", normal_handler, normal_on_entry, 0,0,},
  {protected_s, "protected", protected_handler, protected_on_entry, 0,0,},
  {unprotected_s, "unprotected", unprotected_handler, unprotected_on_entry, 0,0,},
  {quarterbroken_s, "quarterbroken", quarterbroken_handler, quarterbroken_on_entry, 0,0,},
  {halfbroken_s, "halfbroken", halfbroken_handler, halfbroken_on_entry, 0,0,},
  {threequarterbroken_s, "threequarterbroken", threequarterbroken_handler, threequarterbroken_on_entry, 0,0,},
  {broken_s, "broken", broken_handler, broken_on_entry, 0,0,},
};

//.

/* Utilities */
static int sgn(int x)
{
	if (x < 0)return -1;
	if (x > 0)return 1;
	return 0;
}

static int absv(int x)
{
	return(x*sgn(x));
}


//enum spaceshipstate { flying, broken };
struct Spaceship {
	void * evtData_p;
    struct StateChart sc;



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
	float health;
	float angle;
	int w;
	int h;
	int inited;
//	enum spaceshipstate state;
	int spid_spaceship;
	int spid_broken;
	int spid_halfbroken;
	int spid_protected;
	int spid_quarterbroken;
	int spid_threequarterbroken;
	int	cur_spid;



	int cnt;
	struct Laser {
		int sub;
		int prohibit;
	} laser;
};






///
static int top_handler(struct Spaceship * me, int ev) {
	switch (ev) {
	case EVT_LASER_PROHIBIT_TMO:
		me->laser.prohibit = 0;
		return 1;
	}
	return 0;
}

static int flying_handler(struct Spaceship * me, int ev)
{
	//printf("%s %d\n", __FUNCTION__, ev);
	struct CollisionData * cd = 0;
	void * data = me->evtData_p;
	switch (ev) {
	case EVT_W:
		Spaceship_accelerate(me);
		return 1;
	case EVT_A:
		Spaceship_turn_left(me);
		return 1;
	case EVT_S:
		Spaceship_decelerate(me);
		return 1;
	case EVT_D:
		Spaceship_turn_right(me);
		return 1;
	case EVT_SPACE:
		Spaceship_fire(me);
		return 1;
	case EVT_TICK:
		Spaceship_tick(me, 0);
		Spaceship_move(me);
		return 1;
	case EVT_TREAD_DENIED:
		//printf("tread denied\n");
		;
		struct TreadRefuse * tr = data;
		me->x = tr->x;
		me->y = tr->y;
		me->vel_x = -1 * sgn(me->vel_x)*0.5;
		me->vel_y = -1 * sgn(me->vel_y)*0.5;
		return 1;

	case COLSIG_BLUESTAR:
		CHANGE(&(me->sc), protected_s);
		return 1;
	case COLSIG_WORKSHOP:
		if (me->vel_x < 1.0 && me->vel_y < 1.0) {
			CHANGE(&(me->sc), flying_s);
		}
		return 1;

	}
	return 0;
}

static int normal_handler(struct Spaceship * me, int ev) {
	struct CollisionData * cd = 0;
	void * data = me->evtData_p;
	switch (ev) {
	case COLSIG_LASER:
		cd = data;
		if (cd->id != me->laser.sub) {
			me->cnt++;
			if (me->cnt > 0)CHANGE(&(me->sc), quarterbroken_s);
		}
		return 1;
	}
	return 0;
}
static void normal_on_entry(struct Spaceship * me)
{
	me->cur_spid = me->spid_spaceship;
	me->cnt = 0;
	me->health = 1.0;
}
static int protected_handler(struct Spaceship * me, int ev) {
	switch (ev) {
	case COLSIG_LASER:
		return 1;
	case EVT_BLUESTAR_RST:
		CHANGE(&(me->sc), unprotected_s);
		return 1;
	}
	return 0;
}
static void protected_on_entry(struct Spaceship * me) {
	me->cur_spid = me->spid_protected;
}
static int unprotected_handler(struct Spaceship * me, int ev) { return 0; }
static void unprotected_on_entry(struct Spaceship * me) {
	me->cur_spid = me->spid_spaceship;
}
static int quarterbroken_handler(struct Spaceship * me, int ev) { 
	struct CollisionData * cd = 0;
	void * data = me->evtData_p;
	switch (ev) {
	case COLSIG_LASER:
		cd = data;
		if (cd->id != me->laser.sub) {
			me->cnt++;
			if(me->cnt > 5)CHANGE(&(me->sc), halfbroken_s);
		}
		return 1;
	}
	return 0;
}
static void quarterbroken_on_entry(struct Spaceship * me) {
	me->cur_spid = me->spid_quarterbroken;
	me->cnt = 0;
	me->health = 0.2;
}
static int halfbroken_handler(struct Spaceship * me, int ev) {
	struct CollisionData * cd = 0;
	void * data = me->evtData_p;
	switch (ev) {
	case COLSIG_LASER:
		cd = data;
		if (cd->id != me->laser.sub) {
			me->cnt++;
			if (me->cnt > 7)CHANGE(&(me->sc), threequarterbroken_s);
		}
		return 1;
	}
	return 0;
}
static void halfbroken_on_entry(struct Spaceship * me) {
	me->cur_spid = me->spid_halfbroken;
	me->cnt = 0;
	me->health = 0.1;
}
static int threequarterbroken_handler(struct Spaceship * me, int ev) {
	struct CollisionData * cd = 0;
	void * data = me->evtData_p;
	switch (ev) {
	case EVT_A:
		return 1;
	case COLSIG_LASER:
		cd = data;
		if (cd->id != me->laser.sub) {
			me->cnt++;
			if (me->cnt > 3)CHANGE(&(me->sc), broken_s);
		}
		return 1;
	}
	return 0;
}
static void threequarterbroken_on_entry(struct Spaceship * me) {
	me->cur_spid = me->spid_threequarterbroken;
	me->cnt = 0;
	me->health = 0.05;
}
static int broken_handler(struct Spaceship * me, int ev)
{
	struct CollisionData * cd = 0;
	void * data = me->evtData_p;
	switch (ev) {
	case EVT_A:
		me->angle -= 2.0;
		//Spaceship_turn_left(me);
		return 1;
	case EVT_D:
		me->angle += 1.0;
		return 1;
	case EVT_TICK:
		me->angle -= 0.4;
		Spaceship_tick(me, 0);
		Spaceship_move(me);
		return 1;
	}
	return 0;
}

static void broken_on_entry(struct Spaceship * me)
{
	//printf("%s sub=%d\n", __FUNCTION__, me->sub);
	me->vel_x = 0.0;
	me->vel_y = 0.0;
	me->cur_spid = me->spid_broken;
}








static void on_dispatch(void * receiver, int ev, void * data)
{
	struct Spaceship * me = receiver;
	me->evtData_p = data;
	jsm_dispatch(&me->sc, me, ev);
	me->evtData_p = 0;
}



static int g_nextSqCoord;

void Spaceship_init(struct Spaceship * me, jpfusr_t usr)
{
	struct Spaceship ss0 = { 0 };
	*me = ss0;
	struct StateChart sc0 = { 0 };
	me->sc = sc0;
	me->sc.topology_p = topology;
	me->sc.state_funcs_p = state_funcs;

	me->usr = usr;

	struct SqCoord coord;
	g_nextSqCoord = Sq_getNextCoord('V', g_nextSqCoord, &coord);
	me->x = coord.x;
	me->y = coord.y;



	me->vel_x = 0.0;
	me->vel_y = 0.0;
	me->angle = 0.0;
	me->h = MAX_Y;
	me->w = MAX_X;
	me->inited = 1;
	me->cnt = 0;
	me->health = 1.0;

	me->spid_spaceship = jpf_create_sprite("spaceship.png");
	me->spid_broken = jpf_create_sprite("broken.png");
	me->spid_halfbroken = jpf_create_sprite("halfbroken.png");
	me->spid_protected = jpf_create_sprite("protected.png");
	me->spid_quarterbroken = jpf_create_sprite("quarterbroken.png");
	me->spid_threequarterbroken = jpf_create_sprite("threequarterbroken.png");
	me->cur_spid = -1;

	me->sub = jeq_subscribe(on_dispatch, me);

}

void Spaceship_deinit(struct Spaceship * me)
{
	me->inited = 0;
	jpf_release_sprite(me->spid_spaceship);
	jpf_release_sprite(me->spid_broken);

	jeq_unsub(me->sub);
//	jeq_unsub(me->laser.sub);



	{
		struct LeaveData * p = malloc(sizeof(struct LeaveData));
		p->id = me->sub;
		jeq_sendto(EVT_LEAVE, p, WORLD);
	}
	{
		struct LeaveData * p = malloc(sizeof(struct LeaveData));
//		p->id = me->laser.sub;
		jeq_sendto(EVT_LEAVE, p, WORLD);
	}
}


static int laser2_fire(int x, int y, float angle, int ssid);

static void Spaceship_fire(struct Spaceship * me)
{
	/*
	if (me->laser.tmo)return;
	me->laser.tmo = 20;
	me->laser.x = me->x;
	me->laser.y = me->y;
	me->laser.angle = me->angle;
	me->laser.state = active;
	me->laser.hidden = 0;

	*/

	//printf("try fire %d\n", me->laser.prohibit);
	if (!me->laser.prohibit) {
		me->laser.sub = laser2_fire(me->x, me->y, me->angle, me->sub);
		me->laser.prohibit = 1;
		timer_set(me->sub, EVT_LASER_PROHIBIT_TMO, 20);

	}
}

static void Spaceship_turn_left(struct Spaceship * me)
{
	me->angle -= 4.5 * me->health;
}

static void Spaceship_turn_right(struct Spaceship * me)
{
	me->angle += 4.5 * me->health;
}

static void Spaceship_accelerate(struct Spaceship * me)
{
	me->vel_x += cos(me->angle * PI / 180.0) * 0.5;
	me->vel_y += sin(me->angle * PI / 180.0) * 0.5;
}

static void Spaceship_decelerate(struct Spaceship * me)
{
	me->vel_x -= cos(me->angle * PI / 180.0) * 0.2;
	me->vel_y -= sin(me->angle * PI / 180.0) * 0.2;
}

static void Spaceship_prune(struct Spaceship * me)
{
	if (me->x < 0.0)me->x += me->w;
	if (me->x > me->w)me->x -= me->w;
	if (me->y < 0.0)me->y += me->h;
	if (me->y > me->h)me->y -= me->h;
}

static void Spaceship_move(struct Spaceship * me)
{
		me->x += me->vel_x * me->health;
		me->y += me->vel_y * me->health;
		Spaceship_prune(me);
		me->vel_x *= 0.95;
		me->vel_y *= 0.95;
	//if ((abs(me->x - me->old.x) > 0.0001) || (abs(me->y - me->old.y) > 0.0001)) {
	if (absv(me->x) - absv(me->old.x) || absv(me->y) != absv(me->old.y)) {

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



/* JSM Statemachine

name = Laser2
qualified=yes
suffix = yes
top ->swooshing :ix
  swooshing :hex
  fizzling :he

*/

struct Laser2;

enum Laser2State
{
laser2_top_s,
  laser2_swooshing_s,
  laser2_fizzling_s,
};

static const struct TopologyNode laser2_topology[] = {
  // id, super, descend
  {laser2_top_s, 0, laser2_swooshing_s},
    {laser2_swooshing_s, laser2_top_s},
    {laser2_fizzling_s, laser2_top_s}
};

// Fwd decl.
static void laser2_top_on_init(struct Laser2 * me);
static void laser2_top_on_exit(struct Laser2 * me);
static int laser2_swooshing_handler(struct Laser2 * me, int ev);
static void laser2_swooshing_on_entry(struct Laser2 * me);
static void laser2_swooshing_on_exit(struct Laser2 * me);
static int laser2_fizzling_handler(struct Laser2 * me, int ev);
static void laser2_fizzling_on_entry(struct Laser2 * me);

static struct Statefuncs laser2_state_funcs[] = {
  // #, name, handler, entry, exit, init
  {laser2_top_s, "laser2_top", 0,0,laser2_top_on_exit, laser2_top_on_init},
  {laser2_swooshing_s, "laser2_swooshing", laser2_swooshing_handler, laser2_swooshing_on_entry, laser2_swooshing_on_exit, 0,},
  {laser2_fizzling_s, "laser2_fizzling", laser2_fizzling_handler, laser2_fizzling_on_entry, 0,0,},
};

//.

struct Laser2 {
	float x;
	float y;
	float angle;
	int sub;
	int ssid; // id of associated spaceship
	struct StateChart sc;
	void * evtData_p;
	struct Swooshing
	{
		int tmh;
	}swooshing;
};



static void laser2_on_dispatch(void * receiver, int ev, void * data)
{
	struct Laser2 * me = receiver;
	me->evtData_p = data;
	jsm_dispatch(&me->sc, me, ev);
	me->evtData_p = 0;
}


static int laser2_fire(int x, int y, float angle, int ssid)
{
	// Create and initialize a laser
	struct Laser2 * me = malloc(sizeof(struct Laser2));
	struct Laser2 l2 = {0};
	*me = l2;
	me->x = x;
	me->y = y;
	me->angle = angle;
	me->ssid = ssid;
	struct StateChart sc0 = { laser2_topology, laser2_state_funcs };
	me->sc = sc0;
	me->sub = jeq_subscribe(laser2_on_dispatch, me);
	jsm_init(&me->sc, me);
	
	return me->sub;
}


static void laser2_top_on_init(struct Laser2 * me)
{
	
}

static void laser2_top_on_exit(struct Laser2 * me)
{
	//me->sub = jeq_subscribe(laser2_on_dispatch, me);
	jeq_unsub(me->sub);
	free(me);
}


static int laser2_swooshing_handler(struct Laser2 * me, int ev) {
	switch (ev) {
	case EVT_TICK:
		//printf("tick (%d) ", me->sub);

		me->x += 15 * cos(me->angle * PI / 180.0);
		me->y += 15 * sin(me->angle * PI / 180.0);
		//if (me->x < 0 || me->x > MAX_X || me->y < 0 || me->y > MAX_Y) {
		//	me->state = inactive;
		//}
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_LASER;
		p->id = me->sub;
		p->x = me->x;////
		p->y = me->y;/////
		jeq_sendto(EVT_TREAD, p, WORLD);

		struct Draw * t = malloc(sizeof(struct Draw));
		t->id = me->sub;
		t->sprite = laser_sprite;
		t->x = me->x;////
		t->y = me->y;/////
		t->angle = me->angle;
		jeq_sendto(EVT_DRAW, t, DRAW_SUB);
/*
		me->swooshing.cnt++;
		if(me->swooshing.cnt > 20)CHANGE(&(me->sc), laser2_fizzling_s);
		*/
		return 1;
	case EVT_TMO:
		CHANGE(&(me->sc), laser2_fizzling_s);
		return 1;

	case COLSIG_FIXED:
		;
		struct LeaveData * l = malloc(sizeof(struct LeaveData));
		l->id = me->sub;
		jeq_sendto(EVT_LEAVE, l, WORLD);

		l = malloc(sizeof(struct LeaveData));
		l->id = me->sub;
		jeq_sendto(EVT_LEAVE, l, DRAW_SUB);
		CHANGE(&(me->sc), TERMINAL);
		return 1;
	}
	return 0;
}


static void laser2_swooshing_on_entry(struct Laser2 * me)
{
	me->swooshing.tmh = timer_set(me->sub, EVT_TMO, 10);
}

static void laser2_swooshing_on_exit(struct Laser2 * me)
{
	timer_cancel(me->swooshing.tmh);
}


static int laser2_fizzling_handler(struct Laser2 * me, int ev) {
	switch (ev) {
	case EVT_TMO:
		;
		struct LeaveData * p = malloc(sizeof(struct LeaveData));
		p->id = me->sub;
		jeq_sendto(EVT_LEAVE, p, WORLD);
		
		p = malloc(sizeof(struct LeaveData));
		p->id = me->sub;
		jeq_sendto(EVT_LEAVE, p, DRAW_SUB);
		CHANGE(&(me->sc), TERMINAL);
		return 1;
	case EVT_TICK:
		me->angle += 31;

		struct Draw * t = malloc(sizeof(struct Draw));
		t->id = me->sub;
		t->sprite = explosion_sprite;
		t->x = me->x;////
		t->y = me->y;/////
		t->angle = me->angle;
		jeq_sendto(EVT_DRAW, t, DRAW_SUB);
		return 1;


	}
	return 0;
}

static void laser2_fizzling_on_entry(struct Laser2 * me) {
	/*
	struct LeaveData * l = malloc(sizeof(struct LeaveData));
	l->id = me->sub;
	jeq_sendto(EVT_LEAVE, l, WORLD);
	*/

	struct TreadData * p = malloc(sizeof(struct TreadData));
	p->col_sig = COLSIG_LASER;
	p->id = me->sub;
	p->x = me->x;////
	p->y = me->y;/////
	jeq_sendto(EVT_TREAD, p, WORLD);

	struct Draw * t = malloc(sizeof(struct Draw));
	t->id = me->sub;
	t->sprite = explosion_sprite;
	t->x = me->x;////
	t->y = me->y;/////
	t->angle = me->angle;
	jeq_sendto(EVT_DRAW, t, DRAW_SUB);

	me->swooshing.tmh = timer_set(me->sub, EVT_TMO, 6);//name
};














// Keyboard handling
//
void Spaceship_tick(struct Spaceship * me, jpfhandle_t h)//remove?
{

	//  KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
	int p = 0;
	struct { char k[NROF_KEYEVT + 1]; } keys = { {'.', '.', '.', '.', '.', '\0'} };

	if (is_key(me->usr, KEY_W)) {
		p += 1;
		keys.k[KEY_W] = 'x';
	}
	if (is_key(me->usr, KEY_A)) {
		p += 1;
		keys.k[KEY_A] = 'x';
	}
	if (is_key(me->usr, KEY_S)) {
		p += 1;
		keys.k[KEY_S] = 'x';
	}
	if (is_key(me->usr, KEY_D)) {
		p += 1;
		keys.k[KEY_D] = 'x';
	}
	if (is_key(me->usr, KEY_SPACE)) {
		p += 1;
		keys.k[KEY_SPACE] = 'x';
		//ack_key(me->usr, KEY_SPACE);
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
		ack_key(me->usr, KEY_SPACE);
	}
}

int Spaceship_draw(struct Spaceship * me, jpfhandle_t h)
{

	//{ "Id":102, "x":80, "y":120 }
	if (me->inited) {

		jpf_draw_sprite(h, me->cur_spid, me->x, me->y, me->angle);
	}
	return me->inited;
}
/*
int Spaceship_get_coordinates(struct Spaceship * me, float * x, float * y, float * lasx, float * lasy)
{
	*x = me->x;
	*y = me->y;
	*lasx = me->laser.x;
	*lasy = me->laser.y;
	return active == me->laser.state;
}*/


struct Spaceship *  new_Spaceship(jpfusr_t usr)
{

	struct Spaceship * me = malloc(sizeof(struct Spaceship));
	Spaceship_init(me, usr);
	jsm_init(&me->sc, me);
	return me;
}


void free_Spaceship(struct Spaceship * me)
{
	Spaceship_deinit(me);
	free(me);
	return;
}

