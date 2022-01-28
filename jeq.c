#include "jeq.h"
#include <malloc.h>
#include <assert.h>

#define BC (-1) /* broadcast */

#define QS (100) /* Queue size */
#define NSUBS (30)
struct GEvQ {
	int head;
	int tail;
	struct{
		int evid;
		void * data;
		int dest;
	}q[QS];
}g_evq;

void jeq_sendto(int evid, void * data, int dest)
{
	/* the queue grows by the tail */
	g_evq.q[g_evq.tail].evid = evid;
	g_evq.q[g_evq.tail].data = data;
	g_evq.q[g_evq.tail].dest = dest;
	g_evq.tail = (g_evq.tail + 1)%QS;
	assert(g_evq.tail != g_evq.head);
}

void jeq_broadcast(int evid, void * data)
{
	jeq_sendto(evid, data, BC);
}

void jeq_send_now(int evid, void * data, int dest)
{
	/* add to the head */
	g_evq.head = (g_evq.head + QS - 1)%QS;
	assert(g_evq.tail != g_evq.head);

	g_evq.q[g_evq.head].evid = evid;
	g_evq.q[g_evq.head].data = data;
	g_evq.q[g_evq.head].dest = dest;
}


struct {
	int nextindex;
	int n_res; // Number of reserved
	struct Subr {
		dispatch_f dispatch;
		void * receiver;
		int alive;
	}subr[NSUBS];
}g_sublist;



void jeq_init(int n)
{
	assert(0 == g_sublist.nextindex);
	assert(0 == g_sublist.n_res);
	g_sublist.nextindex = n;
	g_sublist.n_res = n;
}



int jeq_subscribe(dispatch_f on_dispatch, void * data)
{
	int ret = g_sublist.nextindex;
	g_sublist.subr[g_sublist.nextindex].receiver = data;
	g_sublist.subr[g_sublist.nextindex].dispatch = on_dispatch;

	//g_sublist.nextindex++;
	int new_next = g_sublist.nextindex;
	int asscnt = 0;
	do {
		asscnt++;
		assert(asscnt <= NSUBS);
		new_next++;
		if (new_next == NSUBS) { new_next = g_sublist.n_res; }
	}while(0 != g_sublist.subr[new_next].alive);
	g_sublist.nextindex = new_next;

	assert(g_sublist.nextindex < NSUBS);
 	g_sublist.subr[ret].alive = 1;
	assert(ret < NSUBS);
	return ret;
}

void jeq_subscribe_res(int n, dispatch_f on_dispatch, void * data)
{
	assert(n < g_sublist.n_res);
	g_sublist.subr[n].alive = 1;
	g_sublist.subr[n].receiver = data;
	g_sublist.subr[n].dispatch = on_dispatch;
}

void jeq_unsub(int subid)
{
	g_sublist.subr[subid].alive = 0;
}

/* return nonzero if queue empty */
int jeq_dispatch(void)
{

	if (g_evq.head != g_evq.tail) {
		/* dispatch from the head */
		const int h = g_evq.head;
		if (BC == g_evq.q[h].dest) {
			for (int i = 0; i < NSUBS/*g_sublist.nextindex*/; i++) {
				if (g_sublist.subr[i].alive) {
					g_sublist.subr[i].dispatch(g_sublist.subr[i].receiver, g_evq.q[h].evid, g_evq.q[h].data);
				}
			}
		}
		else {
			if (g_sublist.subr[g_evq.q[h].dest].alive) {
				g_sublist.subr[g_evq.q[h].dest].dispatch(g_sublist.subr[g_evq.q[h].dest].receiver, g_evq.q[h].evid, g_evq.q[h].data);
			}
		}
		if(g_evq.q[h].data)free(g_evq.q[h].data);

		g_evq.head = (g_evq.head + 1) % QS;
		return 0;
	} else {
		
		return 1;
	}

}
