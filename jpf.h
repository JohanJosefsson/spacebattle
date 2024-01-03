#ifndef JPF_H_
#define JPF_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "jpfappdefines.h"

	// jpf.h

	// handle? TODO
	typedef struct JPFUSR * jpfusr_t;
	typedef struct JPFHANDLE * jpfhandle_t;
	int jpf_create_sprite(char * src);
	void jpf_release_sprite(int spid);
	void jpf_draw_sprite(jpfhandle_t h, int spid, int x, int y, int rot);
	void jpf_camera_follow(jpfusr_t usr, int x, int y);
	enum keyevt {
		KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
	};
	int is_key(jpfusr_t usr, enum keyevt);
  void ack_key(jpfusr_t usr, enum keyevt k);
  // Only meaaningful on the server
#ifdef _WIN32
#define ack_key(a,b)
#endif
	// In math.h? TODO
#define PI (3.14159265)

// Should be improved...? TODO
#define MAX_X ((NSQW - 1)*32)
  //(384)
#define MAX_Y ((NSQH - 1)*32)
  //(384)



// To be implemented by the application
	void jpf_init();
	void jpf_on_new_user(jpfusr_t usr);
	void jpf_on_remove_user(jpfusr_t usr);
	void jpf_on_tick(void);
	void jpf_on_draw(jpfhandle_t h);







#ifdef __cplusplus
}
#endif


#endif
