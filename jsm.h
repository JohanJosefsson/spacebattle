/* jsm.h */


/* Johans hierarchical state machines. Resembles functionality-wise Miro Sameks state machine framework as
described in his book Practical UML Statecharts in C/C++ */


/*
struct TopologyNode:
Used to describe the hierearchical state machines topology in the topology_p array in
state must be a non-negative number. The position in the array corresponds to the state
number.
*/
struct TopologyNode{
	int state;
    int superstate;
	int descend_substate;
};
struct MeType;
typedef int (*handlerfunc_t)(struct MeType * me, int ev);
typedef void(*onfunc_t)(struct MeType * me);

struct Statefuncs {
	// Must correspond to the position in the array:
	const int state_num;
	const char * name;
	// Returns 1 if the state is changed in any way, 0 otherwise: TODO, message consumed?
	handlerfunc_t statehandler;
	onfunc_t entryhandler;
	onfunc_t exithandler;
	onfunc_t inithandler;
};

struct StateChart {
	// To be filled out by the user:
	const struct TopologyNode * topology_p;
	const struct Statefuncs * state_funcs_p;
	// Used by the statemachine framework:
	int current_state;
	int next_state;
};

void jsm_init(struct StateChart * sc, struct MeType * user);
void jsm_dispatch(struct StateChart * sc, struct MeType * user, int evt);
int jsm_get_state(struct StateChart * sc);
int jsm_is_in_state(struct StateChart * sc, int state);

#define CHANGE(sc, nxt) \
do{(sc)->next_state = nxt;}while(0)

// Terminal pseudo state
#define TERMINAL (-2)
