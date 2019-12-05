/* sem_wait -- wait on a semaphore.  Generic futex-using version.
   Copyright (C) 2003, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "sem_waitcommon.c"

int
__new_sem_wait (sem_t *sem)
{
  if (__new_sem_wait_fast ((struct new_sem *) sem, 0) == 0)
    return 0;
  else
    return __new_sem_wait_slow((struct new_sem *) sem, NULL);
}
versioned_symbol (libpthread, __new_sem_wait, sem_wait, GLIBC_2_1);

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)
int
attribute_compat_text_section
__old_sem_wait (sem_t *sem)
{
  int *futex = (int *) sem;
  int err;

  do
    {
      if (atomic_decrement_if_positive (futex) > 0)
	return 0;

      /* Enable asynchronous cancellation.  Required by the standard.  */
      int oldtype = __pthread_enable_asynccancel ();

      /* Always assume the semaphore is shared.  */
      err = lll_futex_wait (futex, 0, LLL_SHARED);

      /* Disable asynchronous cancellation.  */
      __pthread_disable_asynccancel (oldtype);
    }
  while (err == 0 || err == -EWOULDBLOCK);

  __set_errno (-err);
  return -1;
}

compat_symbol (libpthread, __old_sem_wait, sem_wait, GLIBC_2_0);
#endif

int
__new_sem_trywait (sem_t *sem)
{
  /* We must not fail spuriously, so require a definitive result even if this
     may lead to a long execution time.  */
  if (__new_sem_wait_fast ((struct new_sem *) sem, 1) == 0)
    return 0;
  __set_errno (EAGAIN);
  return -1;
}
versioned_symbol (libpthread, __new_sem_trywait, sem_trywait, GLIBC_2_1);
#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)
int
__old_sem_trywait (sem_t *sem)
{
  int *futex = (int *) sem;
  int val;

  if (*futex > 0)
    {
      val = atomic_decrement_if_positive (futex);
      if (val > 0)
	return 0;
    }

  __set_errno (EAGAIN);
  return -1;
}
compat_symbol (libpthread, __old_sem_trywait, sem_trywait, GLIBC_2_0);
#endif
