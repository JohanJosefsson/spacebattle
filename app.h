
// Global definitions and data structures

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
	EVT_LEAVE,
	EVT_ADD_FIXED,
	COLSIG_PLAYER,
	COLSIG_FIXED,
	COLSIG_LASER,
	COLSIG_NOTHING,
	COLSIG_STONE,
	COLSIG_BLUESTAR,
	COLSIG_PINKSTAR,
	COLSIG_WORKSHOP,
	EVT_BLUESTAR_RST,
	EVT_PINKSTAR_RST,
	EVT_DRAW,
	EVT_TIMER,
	EVT_LASER_PROHIBIT_TMO,
	EVT_TMO,
	EVT_CNT_UPDATED,

};
struct TreadData {
	int id;
	int x;
	int y;
	int col_sig;
};
struct TreadRefuse {
	int col_sig;
	int x;
	int y;
};
struct CollisionData {
	int id;
};

struct LeaveData {
	int id;
};

struct Draw {
  int id;
  int sprite;
  float x;
  float y;
  float angle;
};

// These are the static queue subscribers. Those exist during the entire exeution
enum Subs
{
	WORLD,
	FIXED_SUB, // Special subscriber (id in world) for fixed things
	DRAW_SUB,
	NOF_SUBS, // used in queue initalization
};


enum Sprites {
	laser_sprite,
	explosion_sprite,
	NOF_SPRITES,
};

