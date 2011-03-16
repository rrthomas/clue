#include <stdio.h>
#include "clue.h"

int main(void)
{
  clue_State *L = clue_init();
  int x = 0;
  char *hello;
  int a = 2, b = 4, c;
  printf("%d\n", x);
  CLUE_SET(L, x, number, x);
  clue_do(L, "print(x) x=x+1");
  CLUE_GET(L, x, number, x);
  printf("%d\n", x);
  printf("clue_pdo result %d\n",clue_pdo(L,"fail()"));
  clue_do(L, "function hello(a,b) return 'Hello', a+b end");
  clue_call_va(L, "hello", "ii>si", a, b, &hello, &c);
  printf("%s\t%d\n", hello, c);

  return 0;
}
