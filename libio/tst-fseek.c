/* Verify that fseek/ftell combination works for wide chars.

   Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>
#include <wchar.h>
#include <unistd.h>
#include <string.h>

/* Defined in test-skeleton.c.  */
static int create_temp_file (const char *base, char **filename);


static int
do_seek_end (FILE *fp)
{
  long save;

  if (fp == NULL)
    {
      printf ("do_seek_end: fopen: %s\n", strerror (errno));
      return 1;
    }

  if (fputws (L"abc\n", fp) == -1)
    {
      printf ("do_seek_end: fputws: %s\n", strerror (errno));
      return 1;
    }

  save = ftell (fp);
  rewind (fp);

  if (fseek (fp, 0, SEEK_END) == -1)
    {
      printf ("do_seek_end: fseek: %s\n", strerror (errno));
      return 1;
    }

  if (save != ftell (fp))
    {
      printf ("save = %ld, ftell = %ld\n", save, ftell (fp));
      return 1;
    }

  return 0;
}

int
do_seek_set (FILE *fp)
{
  long save;

  if (fputws (L"abc\n", fp) == -1)
    {
      printf ("seek_set: fputws: %s\n", strerror (errno));
      return 1;
    }

  save = ftell (fp);

  if (fputws (L"xyz\n", fp) == -1)
    {
      printf ("seek_set: fputws: %s\n", strerror (errno));
      return 1;
    }

  if (fseek (fp, save, SEEK_SET) == -1)
    {
      printf ("seek_set: fseek: %s\n", strerror (errno));
      return 1;
    }

  if (save != ftell (fp))
    {
      printf ("save = %ld, ftell = %ld\n", save, ftell (fp));
      return 1;
    }

  return 0;
}

static int
do_test (void)
{
  if (setlocale (LC_ALL, "en_US.utf8") == NULL)
    {
      printf ("Cannot set en_US.utf8 locale.\n");
      exit (1);
    }

  int ret = 0;
  char *filename;
  int fd = create_temp_file ("tst-fseek.out", &filename);

  if (fd == -1)
    return 1;

  FILE *fp = fdopen (fd, "w+");
  if (fp == NULL)
    {
      printf ("seek_set: fopen: %s\n", strerror (errno));
      close (fd);
      return 1;
    }

  if (do_seek_set (fp))
    {
      printf ("SEEK_SET test failed\n");
      ret = 1;
    }

  /* Reopen the file.  */
  fclose (fp);
  fp = fopen (filename, "w+");
  if (fp == NULL)
    {
      printf ("seek_end: fopen: %s\n", strerror (errno));
      return 1;
    }

  if (do_seek_end (fp))
    {
      printf ("SEEK_END test failed\n");
      ret = 1;
    }

  fclose (fp);

  return ret;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
