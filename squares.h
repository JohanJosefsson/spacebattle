
struct SqCoord { int x; int y; };

void Sq_init(const char* pattern);

int Sq_getNextCoord(char c, int iter, struct SqCoord * p);
