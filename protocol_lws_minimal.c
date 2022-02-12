/*
 * ws protocol handler plugin for "lws-minimal"
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This version holds a single message at a time, which may be lost if a new
 * message comes.  See the minimal-ws-server-ring sample for the same thing
 * but using an lws_ring ringbuffer to hold up to 8 messages at a time.
 */

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>
#include <assert.h>

// The applications interface towards the implementation in this file
#include "jpf.h"

#define printf lwsl_user

// Global sprite list
// (Global i.e. no vhost handling)
static struct {
  struct {
    char src[40];
    int inuse;
  }arr[100];
  int cnt;
}sprites;

int jpf_create_sprite(char * src)
{
  assert(sprites.cnt < 100); // TODO...
  for(int i = 0; i < sprites.cnt; i++) {
    if(!sprites.arr[i].inuse && !strcmp(src, sprites.arr[i].src)) {
      sprites.arr[i].inuse = 1;
      return i;
    }
  }
  strcpy(sprites.arr[sprites.cnt].src, src);
  sprites.arr[sprites.cnt].inuse = 1;
  assert(sprites.cnt < 100); // TODO...
  return sprites.cnt++;
}

void jpf_release_sprite(int spid)
{
  sprites.arr[spid].inuse = 0;
}


/* one of these created for each message */

struct msg {
        struct msg * next;
	void *payload; /* is malloc'd */
	size_t len;
};

/* one of these is created for each client connecting to us */

struct per_session_data__minimal {
	struct per_session_data__minimal *pss_list;
	struct lws *wsi;
	int last; /* the last message number we sent */
        struct msg * msglist;
        // Application (jpf platform) data
        enum keyevt keys[NROF_KEYEVT];
        int spritecnt;
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;

	struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

//	struct msg amsg; /* the one pending message... */
//	struct msg * msglist; /* the pending messages */
	int current; /* the current message number we are caching */
	struct per_session_data__minimal *masterusr;
        
};

/* destroys the message when everyone has had a copy of it */

static void
__minimal_destroy_message(void *_msg)
{
	struct msg *msg = _msg;

	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}



////////////////////////// The platform (jpf)
//  fwd decl
void jpf_broadcast_string(struct per_vhost_data__minimal *vhd, char * s);

#define JPF_PERIOD_us (50000)


void jpf_draw_sprite(jpfhandle_t h, int spid, int x, int y, int rot)
{
  struct per_vhost_data__minimal *vhd = (struct per_vhost_data__minimal *)h;   
  // json example:
  // { "Id":102, "x":80, "y":120 }
  char * s = malloc(100);
  sprintf(s, "{ \"jpfcmd\":\"draw\", \"spid\":%d, \"x\":%d, \"y\":%d, \"r\":%d }", spid, x, y, (rot + 90));
  jpf_broadcast_string(vhd, s);
  free(s);
}

void jpf_broadcast_string(struct per_vhost_data__minimal * vhd, char * s)
{
//                struct per_vhost_data__minimal *vhd = (struct per_vhost_data__minimal *)vhd;
		lws_start_foreach_llp(struct per_session_data__minimal **,
				      ppss, vhd->pss_list) {

                	int len = strlen(s);
			//vhd->amsg.len = len;
			/* notice we over-allocate by LWS_PRE */
			struct msg * msg = malloc(sizeof(struct msg));
			msg->payload = malloc(LWS_PRE + len);
			msg->next = 0;
			msg->len = len;
			memcpy((char *)msg->payload + LWS_PRE, s, len);


			if((*ppss)->msglist) {
				msg->next = (*ppss)->msglist;
			}
			(*ppss)->msglist = msg;

		} lws_end_foreach_llp(ppss, pss_list);




		lws_start_foreach_llp(struct per_session_data__minimal **,
				      ppss, vhd->pss_list) {




		int l = 0;
		struct msg * p = (*ppss)->msglist;
		while(p) {
			l++;
			p = p->next;
		}



		} lws_end_foreach_llp(ppss, pss_list);
		
//TOD in writable too...
		lws_start_foreach_llp(struct per_session_data__minimal **,
				      ppss, vhd->pss_list) {
			lws_callback_on_writable((*ppss)->wsi);
		} lws_end_foreach_llp(ppss, pss_list);
}

void jpf_pf_tick(struct per_session_data__minimal * pss)
{
  jpf_on_tick((jpfhandle_t)pss);
}

void jpf_pf_draw(struct per_vhost_data__minimal *vhd)
{
  jpf_on_draw((jpfhandle_t)vhd);
//  jpf_broadcast_string(vhd, "{ \"Update\":1 }");
  jpf_broadcast_string(vhd, "{ \"jpfcmd\":\"update\" }");
}

void jpf_pf_rec(jpfusr_t usr, void * in, int len)
{
  if(1 || 12 == len) {
    char buf[len+1];
    buf[len] = '\0';
    memcpy(buf, in, len);
  }

    const int reflen = 10;
    char buf[reflen+1];
    buf[reflen] = '\0';
    memcpy(buf, in, reflen);

  int v = 1;
  char c = '#';

  static const char * ref1 =  "{\"keydn\":\""; 
  if(13 == len && !strcmp(buf, ref1)) {
    c =((char*)in)[reflen];
    v = 1;
  }

  static const char * ref2 =  "{\"keyup\":\""; 
  if(13 == len && !strcmp(buf, ref2)) {
    c =((char*)in)[reflen];
    v = 0;
  }



  struct per_session_data__minimal *pss = (struct per_session_data__minimal *)usr;

  //enum keyevt {
  //  KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE, NROF_KEYEVT
  //};

  switch(c) {
  case 'w':
    pss->keys[KEY_W] = v;
    break;
  case 's':
    pss->keys[KEY_S] = v;
    break;
  case 'a':
    pss->keys[KEY_A] = v;
    break;
  case 'd':
    pss->keys[KEY_D] = v;
    break;
  case ' ':
    pss->keys[KEY_SPACE] = v;
    break;
  default:
    printf("key not handled\n");
  }
}



int is_key(jpfusr_t h, enum keyevt k)
{
  struct per_session_data__minimal *pss = (struct per_session_data__minimal *)h;
  int ret = pss->keys[k];
  //pss->keys[k] = 0;
//  if(KEY_SPACE==k && ret)pss->keys[KEY_SPACE] = 0;

  //if(ret)printf("is_key %i  %u\n", ret, (unsigned)h);
  return ret;
}

void ack_key(jpfusr_t h, enum keyevt k)
{
  struct per_session_data__minimal *pss = (struct per_session_data__minimal *)h;
  pss->keys[k] = 0;
}

////////////////////////////////


static int
callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	struct per_session_data__minimal *pss =
			(struct per_session_data__minimal *)user;
	struct per_vhost_data__minimal *vhd =
			(struct per_vhost_data__minimal *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi),
					lws_get_protocol(wsi));
	int m;


	// Make sure that a master user resets handles the timeout
	switch(reason) {
        	//case LWS_CALLBACK_PROTOCOL_INIT:
       		 case LWS_CALLBACK_ESTABLISHED:
        	case LWS_CALLBACK_TIMER:
        	//case LWS_CALLBACK_CLOSED:
        	case LWS_CALLBACK_SERVER_WRITEABLE:
        	case LWS_CALLBACK_RECEIVE:
        	if(LWS_CALLBACK_PROTOCOL_INIT!=reason && !vhd->masterusr) {
                printf("jpf_init()\n");
                jpf_init();
        	printf("Establish master timer %u %u\n", (unsigned)pss, (unsigned)vhd); 
                  vhd->masterusr = pss;
                  lws_set_timer_usecs(wsi, JPF_PERIOD_us);
        	}
	}




	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
          printf("LWS_CALLBACK_PROTOCOL_INIT:\n");
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
				lws_get_protocol(wsi),
				sizeof(struct per_vhost_data__minimal));
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);
                vhd->masterusr = 0;
		break;

	case LWS_CALLBACK_ESTABLISHED:
                printf("jpf_on_new_user(%lx)\n", (unsigned)user);
		jpf_on_new_user((unsigned)user);
// should clear app data?
		/* add ourselves to the list of live pss held in the vhd */
		lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
		pss->wsi = wsi;
		pss->last = vhd->current;
                pss->msglist = 0;
		break;

        case LWS_CALLBACK_TIMER:
                        lws_set_timer_usecs(wsi, JPF_PERIOD_us);
			jpf_pf_tick(pss);
			jpf_pf_draw(vhd);
                break;

	case LWS_CALLBACK_CLOSED:
                printf("user removed\n");
		jpf_on_remove_user((unsigned)pss);//fix types!
                if(pss == vhd->masterusr) {
			printf("master user removed\n");
                	vhd->masterusr = 0;
        		// Make someone else master user (responsibe for timeouts)
			if(!vhd->masterusr && vhd->pss_list) {
				lws_callback_on_writable(vhd->pss_list->wsi);
			}
		}
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
				  pss, vhd->pss_list);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
                if(pss->spritecnt < sprites.cnt) {
                    char fmt[] = "{\"jpfcmd\": \"createsprite\", \"spid\": %i, \"src\": \"%s\"}";
                    char buf[LWS_PRE + sizeof(sprites.arr[0].src) + sizeof(fmt) + 3] = {0};
		    sprintf(buf + LWS_PRE, fmt, pss->spritecnt, sprites.arr[pss->spritecnt].src);


				/* notice we allowed for LWS_PRE in the payload already */
				m = lws_write(wsi, buf + LWS_PRE, strlen(buf + LWS_PRE), LWS_WRITE_TEXT);
/*
				if (m < (int)msg->len) {
					lwsl_err("ERROR %d writing to ws\n", m);
				return -1;
				}*/
				lws_callback_on_writable(pss->wsi);
                                pss->spritecnt++;






		} else {
			struct msg * msg = pss->msglist;
			if(msg) {
				/* notice we allowed for LWS_PRE in the payload already */
				m = lws_write(wsi, ((unsigned char *)msg->payload) +
			      	LWS_PRE, msg->len, LWS_WRITE_TEXT);

				if (m < (int)msg->len) {
					lwsl_err("ERROR %d writing to ws\n", m);
				return -1;
				}
				pss->msglist = msg->next;
                                __minimal_destroy_message(msg);
				free(msg);
			}
			if(pss->msglist) {
				lws_callback_on_writable(pss->wsi);
			}
		}




//		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_RECEIVE:
//TODO destry msg correctly
	 	jpf_pf_rec((jpfusr_t)user, in, len);
		break;

	default:
		break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL \
	{ \
		"lws-minimal", \
		callback_minimal, \
		sizeof(struct per_session_data__minimal), \
		128, \
		0, NULL, 0 \
	}
