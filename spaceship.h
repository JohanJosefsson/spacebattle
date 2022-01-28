
struct Spaceship;


void Spaceship_tick(struct Spaceship * me, jpfhandle_t h);//remove?
int Spaceship_draw(struct Spaceship * me, jpfhandle_t h);

struct Spaceship *  new_Spaceship(jpfusr_t usr);
void free_Spaceship(struct Spaceship * me);
