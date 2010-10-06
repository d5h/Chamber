/* Copyright (C) Daniel Hipschman 2005 */
/* $Id: common.c,v 1.1 2005/04/25 22:47:47 sirdan Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"

/*
 * Given a filename 'name', return the substring after the last
 * directory separator and with a single extension ripped off.
 * E.g.,	/usr/foo.tar.gz  ->  foo.tar
 */
char *
rootname (const char *name)
{
  const char *base, *ext;
  char *root;
  int len;

  base = strrchr (name, DIRSEP);
  if (base == NULL)
    base = name;

  ext = strrchr (base, '.');

  len = ext ? ext - base : strlen (base);
  root = (char *) xmalloc (len + 1);

  strncpy (root, base, len);
  root[len] = '\0';
  return root;
}

/*
 * Prints a message and terminates program.
 */
void
fatal (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);

  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");

  va_end (args);
  exit (EXIT_FAILURE);
}

/*
 * Allocates 'size' bytes or dies.
 */
void *
xmalloc (size_t size)
{
  void *p = malloc (size);
  if (!p)
    fatal ("Out of memory");
  return p;
}

/*
 * Like fprintf but dies on error (for example, if writing
 * to a full disk.
 */
void
xprintf (FILE * fp, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  if (vfprintf (fp, fmt, args) < 0)
    fatal ("file write error");
  va_end (args);
}

/*
 * Appends '.c' to 'name' and attempts to open a file by that name.
 */
FILE *
create_c_file (const char *name)
{
  FILE *fp;
  int len = strlen (name);
  char *file = xmalloc (len + 3);

  strcpy (file, name);
  strcpy (file + len, ".c");

  fp = fopen (file, "w");
  if (fp == NULL)
    fatal ("Couldn't open %s", file);

  free (file);
  return fp;
}
