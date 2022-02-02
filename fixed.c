
#include "spacebattle.h"
#include "squares.h"


struct {
	int sub;
	int spid_stone;
	int spid_bluestar;
	int spid_workshop;
	struct Bluestar
	{
		int cnt;
	}bluestar;
}g_fx;

static void on_dispatch(void * receiver, int ev, void * data)
{
	switch (ev)
	{
	case EVT_TICK:
		g_fx.bluestar.cnt++;
		if (g_fx.bluestar.cnt > 300)
		{
			g_fx.bluestar.cnt = 0;
			jeq_broadcast(EVT_BLUESTAR_RST, 0);
		}
		break;
	}
	//printf("fixed on_dispatch: %d\n", ev);
}



void fx_init()
{
	g_fx.sub = jeq_subscribe(on_dispatch, &g_fx);
	g_fx.spid_stone = jpf_create_sprite("stone.png");
	g_fx.spid_bluestar = jpf_create_sprite("bluestar.png");
	g_fx.spid_workshop = jpf_create_sprite("workshop.png");

	struct SqCoord coord = { 0,0 };
	int iter = 0;

	do {
		iter = Sq_getNextCoord('O', iter, &coord);
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_FIXED;
		p->id = g_fx.sub;
		p->x = coord.x;
		p->y = coord.y;
		//printf("SEND iter=%d x=%d y=%d\n", iter, p->x, p->y);
		jeq_sendto(EVT_ADD_FIXED, p, WORLD);
		
		//if (0 == iter)break;
	} while (iter);

	do {
		iter = Sq_getNextCoord('*', iter, &coord);
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_BLUESTAR;
		p->id = g_fx.sub;
		p->x = coord.x;
		p->y = coord.y;
		//printf("SEND iter=%d x=%d y=%d\n", iter, p->x, p->y);
		jeq_sendto(EVT_ADD_FIXED, p, WORLD);

		//if (0 == iter)break;
	} while (iter);
	do {
		iter = Sq_getNextCoord('X', iter, &coord);
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_WORKSHOP;
		p->id = g_fx.sub;
		p->x = coord.x;
		p->y = coord.y;
		//printf("SEND iter=%d x=%d y=%d\n", iter, p->x, p->y);
		jeq_sendto(EVT_ADD_FIXED, p, WORLD);

		//if (0 == iter)break;
	} while (iter);


}

void fx_draw(jpfhandle_t h)
{
	struct SqCoord coord = { 0,0 };
	int iter = 0;
	do {
		iter = Sq_getNextCoord('O', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_stone, coord.x, coord.y, 0);
	} while (iter);

	do {
		iter = Sq_getNextCoord('*', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_bluestar, coord.x, coord.y, 0);
	} while (iter);

	do {
		iter = Sq_getNextCoord('X', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_workshop, coord.x, coord.y, 0);
	} while (iter);



}
