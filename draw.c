
#include "app.h"
#include "draw.h"

#define N (30)  // cf. NSUB
struct {
	int sub;
	int spids[NOF_SPRITES];
    struct Draw draw[N];
	int in_use[N];
}g_draw;

static void on_dispatch(void * receiver, int ev, void * data)
{
	switch (ev)
	{
	case EVT_DRAW:
		g_draw.draw[((struct Draw*)data)->id] = *((struct Draw*)data);
		g_draw.in_use[((struct Draw*)data)->id] = 1;
		break;
	case EVT_LEAVE:
		g_draw.in_use[((struct Draw*)data)->id] = 0;
		break;
	}
	//printf("fixed on_dispatch: %d\n", ev);
}



void draw_init()
{
	g_draw.sub = jeq_subscribe_res(DRAW_SUB, on_dispatch, &g_draw);
	g_draw.spids[laser_sprite] = jpf_create_sprite("laser.png");
	g_draw.spids[explosion_sprite] = jpf_create_sprite("explosion.png");



}

void draw_draw(jpfhandle_t h)
{
	for (int i = 0; i < N; i++) {
		if(g_draw.in_use[i])jpf_draw_sprite(h, g_draw.spids[g_draw.draw[i].sprite], g_draw.draw[i].x, g_draw.draw[i].y, g_draw.draw[i].angle);
	}

}
