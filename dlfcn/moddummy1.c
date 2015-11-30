/* Provide a dummy DSO for tst-recursive-dlopen to use.  */
#include <stdio.h>
#include <stdlib.h>

int called_dummy1;

void
dummy1 (void)
{
  printf ("Called dummy1()\n");
  called_dummy1++;
}

