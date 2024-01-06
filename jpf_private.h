
// Internal, private for JPF

#include "jpfappdefines.h"

struct Camera {
	int x;
	int y;
	int tgt_x;
	int tgt_y;
};

// World coordinates <-> screen coordinates
inline static int jpfpriv_x_w2s(int wx, struct Camera * c) {
	return wx - c->x + SCR_W / 2;
}
inline static int jpfpriv_y_w2s(int wy, struct Camera* c) {
	return wy - c->y + SCR_H / 2;
}
inline static int jpfpriv_x_s2w(int sx, struct Camera* c) {
	return c->x - SCR_W / 2 + sx;
}
inline static int jpfpriv_y_s2w(int sy, struct Camera* c) {
	return c->y - SCR_H / 2 + sy;
}


inline static void jpfpriv_jpfpriv_set_camera(struct Camera* c, int x, int y)
{
	int y0 = c->tgt_y;
	const int marg = 2 * 32;

	int sx = jpfpriv_x_w2s(x, c);
	int sy = jpfpriv_y_w2s(y, c);

	if (sx < marg)c->tgt_x = jpfpriv_x_s2w(marg, c);
	if (sx > SCR_W - marg)c->tgt_x = jpfpriv_x_s2w((SCR_W - marg), c);
	if (c->tgt_x < SCR_W / 2)c->tgt_x = SCR_W / 2;
	if (c->tgt_x > WRD_W - SCR_W / 2)c->tgt_x = WRD_W - SCR_W / 2;

	if (sy < marg)c->tgt_y = jpfpriv_y_s2w(marg, c);
	if (sy > SCR_H - marg)c->tgt_y = jpfpriv_y_s2w((SCR_H - marg), c);
	if (c->tgt_y < SCR_H / 2)c->tgt_y = SCR_H / 2;
	if (c->tgt_y > WRD_H - SCR_H / 2)c->tgt_y = WRD_H - SCR_H / 2;

	if (c->tgt_y != y0) {
		printf("%i %i\n", y0, c->tgt_y);
	}
}

#if 0
inline static void jpfpriv_jpfpriv_set_camera(struct Camera* c, int x, int y)
{
  c-> x = x;
  c-> y = y;
}
#endif


inline static void jpfpriv_tick_camera(struct Camera* c)
{
	const int step = 10;
	if (abs(c->x - c->tgt_x) < step)c->x = c->tgt_x;
	if (abs(c->y - c->tgt_y) < step)c->y = c->tgt_y;
	if (c->x < c->tgt_x) c->x += step;
	if (c->x > c->tgt_x) c->x -= step;
	if (c->y < c->tgt_y) c->y += step;
	if (c->y > c->tgt_y) c->y -= step;
}
