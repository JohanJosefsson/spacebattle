#include "jsm.h"
#include <stdio.h>

//enum State;

static int isSuperStateOf(struct StateChart * sc, int possibleSuperState, int subState)
{
	int i = subState;
	while ( i != 0) {
		i = sc->topology_p[i].superstate;
		if (i == possibleSuperState) return 1;
	} 
	return 0;
}

int jsm_get_state(struct StateChart * sc) {
	return sc->current_state;
}
int jsm_is_in_state(struct StateChart * sc, int state) {
	if (state == sc->current_state) { return 1; }
	return(isSuperStateOf(sc, state, sc->current_state));
}

#define HIDE
#ifdef HIDE
#define PRINTF(...)
#else
#define PRINTF printf
#endif

#define getStateName(sc, s) (sc->state_funcs_p[s].name)

static void descend(struct StateChart * sc, struct MeType * user);

static void descendTo(struct StateChart * sc, struct MeType * user, int tgt, int already_inside)
{
	PRINTF("descendTo(%d)\n", tgt);
	int lastentered = sc->current_state;
	while (tgt != lastentered)
	{
		int next = tgt;
		while (sc->topology_p[next].superstate != lastentered) {
			next = sc->topology_p[next].superstate;
		}
		PRINTF("ENTRY action of state: %s (%d)\n", getStateName(sc, next), next);
		if (!already_inside) {
			if (sc->state_funcs_p[next].entryhandler)sc->state_funcs_p[next].entryhandler(user);
		}
		already_inside = 0;
		lastentered = next;
	}
	sc->current_state = tgt;
	if (sc->topology_p[sc->current_state].descend_substate)descend(sc, user);
}

static void descend(struct StateChart * sc, struct MeType * user)
{
	PRINTF("descend()\n");
	int fromstate = sc->current_state;
	int tostate = sc->topology_p[fromstate].descend_substate;
	if (tostate) {
		if (sc->state_funcs_p[fromstate].inithandler)sc->state_funcs_p[fromstate].inithandler(user);

	}
	descendTo(sc, user, tostate, 0);
}

void jsm_init(struct StateChart * sc, struct MeType * user)
{
//	sc->next_state = -1;
//	if (sc->initdescendhandler)sc->initdescendhandler(user);
	sc->current_state = 0;
	descend(sc, user);
}
void jsm_dispatch(struct StateChart * sc, struct MeType * user, int evt)
{
	int stateFuncIdToCall;
	int stateFuncRet;

	//PRINTF("***\nHandle signal:  %d\n", evt);
	//PRINTF("state before processing = %s\n", sc->state_funcs_p[sc->current_state].name);
	sc->next_state = -1;

	int triggerstate = -1;
	// call the state function
	int inroot = 0;
	stateFuncIdToCall = sc->current_state;
	do {
		stateFuncRet = 0;
		if (sc->state_funcs_p[stateFuncIdToCall].statehandler) {
			stateFuncRet = sc->state_funcs_p[stateFuncIdToCall].statehandler(user, evt);
			triggerstate = stateFuncIdToCall;
		}
		inroot = stateFuncIdToCall == 0;
		stateFuncIdToCall = sc->topology_p[stateFuncIdToCall].superstate;
	} while (!stateFuncRet && !inroot);

	if (sc->next_state != -1)
	{
		//TODO internal tranistions (out-in) does not work...I think
		int s, tgt;
		int is_internal_trans = isSuperStateOf(sc, sc->next_state, sc->current_state) && (sc->next_state != triggerstate);
		tgt = sc->next_state;

		//
		int straightinline = 0;

		s = sc->current_state;
		//exit all hierarchical states

		while (!isSuperStateOf(sc, s, tgt) || isSuperStateOf(sc, triggerstate, s)) {

			// call state handler with exit signal
			PRINTF("EXIT action of state: %s (%d)\n", getStateName(sc, s), s);
			if (!(is_internal_trans && s == tgt)) {
				if (sc->state_funcs_p[s].exithandler)sc->state_funcs_p[s].exithandler(user);
			}
			s = sc->topology_p[s].superstate;
			sc->current_state = s;
		};
		// enter all hierarcical states
		int desctate = sc->next_state;
		do {
			descendTo(sc, user, desctate, is_internal_trans);
			desctate = sc->topology_p[sc->current_state].descend_substate;
		} while (desctate);
	}
	//PRINTF("state after processing = %s\n", sc->state_funcs_p[sc->current_state].name);
}


