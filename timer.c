
#include "timer.h"
#include <malloc.h>
#include <assert.h>
#include "jeq.h"

struct Timer {
	int handle;
	int sub;
	int evt;
	int value;
	struct Timer * next;
};

static struct {
	struct Timer * l;
	int h;
}g_timer;

int timer_set(int sub, int evt, int value)
{
	struct Timer * t = malloc(sizeof(struct Timer));
	t->handle = ++g_timer.h;
	t->sub = sub;
	t->evt = evt;
	t->value = value;
	t->next = g_timer.l;
	g_timer.l = t;
}

void timer_cancel(int h)
{
	struct Timer ** pp;
	pp = &g_timer.l;
	while (*pp != 0) {
		if ((*pp)->handle == h) {

			struct Timer * p = *pp;
			*pp = (*pp)->next;
			free(p);

		};
		if (*pp)pp = &((*pp)->next);
	}
}

void timer_tick()
{
	struct Timer ** pp;
	pp = &g_timer.l;
	while (*pp != 0) {
		//printf("v=%d ", (*pp)->value);
		if (!(--((*pp)->value))) {
			
			// The timer has expired
			jeq_send_now((*pp)->evt, 0, (*pp)->sub);
			
			struct Timer * p = *pp;
			*pp = (*pp)->next;
			free(p);
			
		};
		if(*pp)pp = &((*pp)->next);
	}
}
