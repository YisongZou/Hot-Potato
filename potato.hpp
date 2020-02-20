#include <cstdio>
#include <cstdlib>
#include <string>
struct Potato {
  int hops;
  int count;
  char ip[];
};
typedef struct Potato potato;
