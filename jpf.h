#ifndef JPF_H_
#define JPF_H_
#ifdef __cplusplus
extern "C" {
#endif

	// jpf.h

	// handle? TODO
	typedef unsigned jpfusr_t; /* n.b: 0 = null */
	typedef struct JPFHANDLE * jpfhandle_t;
	int jpf_create_sprite(char * src);
	void jpf_release_sprite(int spid);
	void jpf_draw_sprite(jpfhandle_t h, int spid, int x, int y, int rot);
	enum keyevt {
		KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
	};
	int is_key(jpfusr_t usr, enum keyevt);
  void ack_key(jpfusr_t h, enum keyevt k);
  // Only meaaningful on the server
#ifdef _WIN32
#define ack_key(a,b)
#endif
	// In math.h? TODO
#define PI (3.14159265)

// Should be improved...? TODO
#define MAX_X (384)
#define MAX_Y (384)


// To be implemented by the application
	void jpf_init();
	void jpf_on_new_user(jpfusr_t usr);
	void jpf_on_remove_user(jpfusr_t usr);
	void jpf_on_tick(jpfhandle_t h);
	void jpf_on_draw(jpfhandle_t h);







#ifdef __cplusplus
}
#endif


#endif
