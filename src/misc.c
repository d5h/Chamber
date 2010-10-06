/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: misc.c,v 1.7 2006/06/15 23:20:45 dsh Exp $ */

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

void
fatal (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);

  fprintf (stderr, "Fatal error: ");
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");

  va_end (args);
  exit (EXIT_FAILURE);
}

void
warning (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);

  fprintf (stderr, "Warning: ");
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");

  va_end (args);
}

void *
xalloc (size_t nbytes)
{
  void *xptr = malloc (nbytes);
  if (xptr == NULL)
    fatal ("out of memory");
  return xptr;
}

void
xfree (void *xptr)
{
  if (xptr)
    free (xptr);
}

char *
xstrdup (const char *str)
{
  /* Look Ma, one line! */
  return strcpy (xalloc (strlen (str) + 1), str);
}

int
stricmp (const char *s1, const char *s2)
{
  int cmp;
  while ((cmp = toupper (*s1) - toupper (*s2)) == 0)
    {
      if (*s1 == '\0')
	return 0;
      ++s1;
      ++s2;
    }
  return cmp;
}

char *
first_nonspace (const char *str)
{
  while (isspace (*str))
    ++str;
  return (char *) str;
}

char *
one_after_last_nonspace (const char *str)
{
  const char *end = str + strlen (str);
  while (end > str && isspace (end[-1]))
    --end;
  return (char *) end;
}


real
rrand (real a, real b)
{
  return ((real) rand () / (real) RAND_MAX) * (b - a) + a;
}

int
rround (real n)
{
  double ip;
  real r = (real) modf (n, &ip);
  if (r >= (real) 0.5)
    return (int) ((real) ip + (real) 1.0);
  if (r <= (real) - 0.5)
    return (int) ((real) ip - (real) 1.0);
  else
    return (int) ip;
}

static const void *dummy;

void
#undef no_warn_about_unused
no_warn_about_unused (const void *ptr)
{
  dummy = ptr;
}
