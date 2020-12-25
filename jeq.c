#include "jeq.h"

#include <assert.h>

#define BC (-1) /* broadcast */

#define QS (100) /* Queue size */
#define NSUBS (100)
struct {
	int head;
	int tail;
	struct{
		int evid;
		void * data;
		int dest;
	}q[100];
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
	dispatch_f dispatch[NSUBS];
	void * receiver[NSUBS];
}g_sublist;

int jeq_subscribe(dispatch_f on_dispatch, void * data)
{
	int ret = g_sublist.nextindex;
	g_sublist.receiver[g_sublist.nextindex] = data;
	g_sublist.dispatch[g_sublist.nextindex++] = on_dispatch;
	assert(g_sublist.nextindex < NSUBS);
	return ret;
}

void jeq_unsub(int subid)
{
	assert(0); // Not yet implemented
}

/* return nonzero if queue empty */
int jeq_dispatch(void)
{

	if (g_evq.head != g_evq.tail) {
		/* dispatch from the head */
		int h = g_evq.head;
		if (BC == g_evq.q[h].dest) {
			for (int i = 0; i < g_sublist.nextindex; i++) {
				g_sublist.dispatch[i](g_sublist.receiver[i], g_evq.q[h].evid, g_evq.q[h].data);
			}
		}
		else {
			g_sublist.dispatch[g_evq.q[h].dest](g_sublist.receiver[g_evq.q[h].dest], g_evq.q[h].evid, g_evq.q[h].data);
		}

		g_evq.head = (g_evq.head + 1) % QS;
		return 0;
	} else {
		
		return 1;
	}

}
