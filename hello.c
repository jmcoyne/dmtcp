
#include <stdio.h>
#include <unistd.h>

int
main()
{
  int i = 0;
  while (1 != 0) {

    printf("%d", i);
    fflush(stdout);
    i++;
    sleep(1);
  }

  return 0;
}