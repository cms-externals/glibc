/* Provide a dummy DSO for tst-recursive-dlopen to use.  */
#include <stdio.h>
#include <stdlib.h>

int called_dummy2;

void
dummy2 (void)
{
  printf ("Called dummy2()\n");
  called_dummy2++;
}

