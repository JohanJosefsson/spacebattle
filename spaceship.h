#include "jpf.h"



enum Event {
	EVT_W,
	EVT_A,
	EVT_S,
	EVT_D,
	EVT_SPACE,
	EVT_TICK,

	EVT_TREAD,
	EVT_TREAD_DENIED,
	EVT_COLLISION,
};

enum CollisionSignature {
	COLSIG_PLAYER,
	COLSIG_FIXED,
	COLSIG_LASER
};

struct TreadData {
	int id;
	int x;
	int y;
	enum CollisionSignature col_sig;
};
struct TreadRefuse {
	enum CollisionSignature col_sig;
	int x;
	int y;
};
struct CollisionData {
	//int x;
	//int y;
	enum CollisionSignature col_sig;
	int id;
};



enum Subs
{
	WORLD,
	NOF_SUBS,
};

struct Spaceship;


void Spaceship_tick(struct Spaceship * me, jpfhandle_t h);//remove?
int Spaceship_draw(struct Spaceship * me, jpfhandle_t h);

struct Spaceship *  new_Spaceship(jpfusr_t usr);
void free_Spaceship(struct Spaceship * me);
