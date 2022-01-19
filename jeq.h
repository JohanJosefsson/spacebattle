
void jeq_init(int n);

void jeq_sendto(int evid, void * data, int dest);

void jeq_broadcast(int evid, void * data);

void jeq_send_now(int evid, void * data, int dest);

typedef void(*dispatch_f)(void * receiver, int event, void * data);

int jeq_subscribe(dispatch_f on_dispatch,void * data);

// Subscribe with a reserverd (known) id
void jeq_subscribe_res(int n, dispatch_f on_dispatch, void * data);

void jeq_unsub(int subid);

/* return nonzero if queue empty */
int jeq_dispatch(void);
