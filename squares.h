
struct SqCoord { int x; int y; };

void Sq_init(const char* pattern, const char* char_to_index_map, int n);

int Sq_getNextCoord(char c, int iter, struct SqCoord * p);
