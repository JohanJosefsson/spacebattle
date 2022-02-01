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
//	EVT_COLLISION,
	EVT_LEAVE,
	EVT_ADD_FIXED,
	COLSIG_PLAYER,
	COLSIG_FIXED,
	COLSIG_LASER,
	COLSIG_NOTHING,
	COLSIG_STONE,


};
/*
enum CollisionSignature {
	COLSIG_PLAYER,
	COLSIG_FIXED,
	COLSIG_LASER,
	COLSIG_NOTHING,
};
*/
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
	//int x;
	//int y;
//	enum CollisionSignature col_sig;
	int id;
};

struct LeaveData {
	int id;
};

// These are the static queue subscribers. Those exist during the entire exeution
enum Subs
{
	WORLD,
	NOF_SUBS, // used in queue initalization
};

