
#include "spacebattle.h"
#include "squares.h"


struct {
	int sub;
	int spid_stone;
	int spid_bluestar;
	int spid_pinkstar;
	int spid_workshop;
	int spids_comet[3];
	struct Bluestar
	{
		int cnt;
		int rot;
	}bluestar;
	struct Pinkstar
	{
		int cnt;
		int rot;
	}pinkstar;
	struct Comet
	{
		int x;
		int y;
	}comet;
}g_fx;

static void on_dispatch(void * receiver, int ev, void * data)
{
	switch (ev)
	{
	case EVT_TICK:

		// Bluestar
		g_fx.bluestar.rot += 1.2;
		g_fx.bluestar.cnt++;
		if (g_fx.bluestar.cnt > 300)
		{
			g_fx.bluestar.cnt = 0;
			jeq_broadcast(EVT_BLUESTAR_RST, 0);
		}

		// Pinkstar
		g_fx.pinkstar.rot += 2.2;
		g_fx.pinkstar.cnt++;
		if (g_fx.pinkstar.cnt > 350)
		{
			g_fx.pinkstar.cnt = 0;
			jeq_broadcast(EVT_PINKSTAR_RST, 0);
		}

		// Comet
		g_fx.comet.y += 7;
		g_fx.comet.y %= MAX_Y;

		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_LASER;
		p->id = g_fx.sub;
		p->x = g_fx.comet.x;////
		p->y = g_fx.comet.y;/////
		jeq_sendto(EVT_TREAD, p, WORLD);


		break;
	}
	//printf("fixed on_dispatch: %d\n", ev);
}



void fx_init()
{
	g_fx.sub = jeq_subscribe(on_dispatch, &g_fx);/// fix?
	g_fx.spid_stone = jpf_create_sprite("stone.png");
	g_fx.spid_bluestar = jpf_create_sprite("bluestar.png");
	g_fx.spid_pinkstar = jpf_create_sprite("pinkstar.png");
	g_fx.spid_workshop = jpf_create_sprite("workshop.png");

	g_fx.spids_comet[0] = jpf_create_sprite("comet1.png");
	g_fx.spids_comet[1] = jpf_create_sprite("comet2.png");
	g_fx.spids_comet[2] = jpf_create_sprite("comet3.png");

	struct SqCoord coord = { 0,0 };
	int iter = 0;

	g_fx.comet.x = (9 * 32);
	g_fx.comet.y = 0;

	do {
		iter = Sq_getNextCoord('O', iter, &coord);
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_FIXED;
		p->id = FIXED_SUB;
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
		p->id = FIXED_SUB;
		p->x = coord.x;
		p->y = coord.y;
		//printf("SEND iter=%d x=%d y=%d\n", iter, p->x, p->y);
		jeq_sendto(EVT_ADD_FIXED, p, WORLD);

		//if (0 == iter)break;
	} while (iter);

	do {
		iter = Sq_getNextCoord('+', iter, &coord);
		struct TreadData * p = malloc(sizeof(struct TreadData));
		p->col_sig = COLSIG_PINKSTAR;
		p->id = FIXED_SUB;
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
		p->id = FIXED_SUB;
		p->x = coord.x;
		p->y = coord.y;
		//printf("SEND iter=%d x=%d y=%d\n", iter, p->x, p->y);
		jeq_sendto(EVT_ADD_FIXED, p, WORLD);

		//if (0 == iter)break;
	} while (iter);


}

void fx_draw(jpfhandle_t h)
{
	jpf_draw_sprite(h, g_fx.spids_comet[(g_fx.comet.y)%3], g_fx.comet.x, g_fx.comet.y, 270);

	struct SqCoord coord = { 0,0 };
	int iter = 0;
	do {
		iter = Sq_getNextCoord('O', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_stone, coord.x, coord.y, 0);
	} while (iter);

	do {
		iter = Sq_getNextCoord('*', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_bluestar, coord.x, coord.y, g_fx.bluestar.rot);
	} while (iter);

	do {
		iter = Sq_getNextCoord('+', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_pinkstar, coord.x, coord.y, g_fx.pinkstar.rot);
	} while (iter);

	do {
		iter = Sq_getNextCoord('X', iter, &coord);
		jpf_draw_sprite(h, g_fx.spid_workshop, coord.x, coord.y, 0);
	} while (iter);



}
