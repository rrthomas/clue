#include <stdio.h>
#include "clue.h"

int main(void)
{
  clue_State *L = clue_init();
  int x = 0;
  printf("%d\n", x);
  CLUE_SET(L, x, number, x);
  clue_do(L, "print(x) x=x+1");
  CLUE_GET(L, x, number, x);
  printf("%d\n", x);

  return 0;
}
