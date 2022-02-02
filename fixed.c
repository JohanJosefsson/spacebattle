
#include "spacebattle.h"
#include "squares.h"


struct {
	int sub;
	int spid;
}g_fx;

static void on_dispatch(void * receiver, int ev, void * data)
{
	//printf("fixed on_dispatch: %d\n", ev);
}



void fx_init()
{
	g_fx.sub = jeq_subscribe(on_dispatch, &g_fx);
	g_fx.spid = jpf_create_sprite("stone.png");

	struct SqCoord coord = { 0,0 };
	int iter = 0;
	while (iter = Sq_getNextCoord('O', iter, &coord)) {
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_FIXED;
		p->id = g_fx.sub;
		p->x = coord.x;
		p->y = coord.y;
		jeq_sendto(EVT_ADD_FIXED, p, WORLD);
	}


}

void fx_draw(jpfhandle_t h)
{
	struct SqCoord coord = { 0,0 };
	int iter = 0;
	while (iter = Sq_getNextCoord('O', iter, &coord)) {
		jpf_draw_sprite(h, g_fx.spid, coord.x, coord.y, 0);
	}
}
