/* Copyright (C) Daniel Hipschman, Apr 2005 */
/* $Id: txt2c.c,v 1.2 2005/05/08 18:08:30 sirdan Exp sirdan $ */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

static void
usage (void)
{
  xprintf (stderr, "txt2c text.txt ...\n");
  exit (EXIT_FAILURE);
}

static void
txt2c (const char *fname)
{
  int c;
  FILE *ifp, *ofp;
  char *newname = rootname (fname);

  ifp = fopen (fname, "r");
  if (ifp == NULL)
    fatal ("%s: %s\n", fname, strerror (errno));

  ofp = create_c_file (newname);

  xprintf (ofp, "/* Automatically generated from \"%s\". */\n\n", fname);
  xprintf (ofp, "extern char %s_txt[];\n", newname);
  xprintf (ofp, "char %s_txt[] = \"\\\n", newname);

  while ((c = fgetc (ifp)) != EOF)
    {
      switch (c)
	{
	case '\n':
	  xprintf (ofp, "\\n\\\n");
	  break;
	case '\r':
	  xprintf (ofp, "\\r");
	  break;
	case '\t':
	  xprintf (ofp, "\\t");
	  break;
	case '\"':
	  xprintf (ofp, "\\\"");
	  break;
	case '\\':
	  xprintf (ofp, "\\\\");
	  break;
	default:
	  if (isprint (c))
	    xprintf (ofp, "%c", c);
	  else
	    xprintf (ofp, "\\%o", c);
	}
    }

  if (ferror (ifp))
    fatal ("%s: %s\n", fname, strerror (errno));

  xprintf (ofp, "\";\n");

  fclose (ofp);
  fclose (ifp);
  free (newname);
}

int
main (int argc, char *argv[])
{
  int i;

  if (argc == 1)
    usage ();

  for (i = 1; i < argc; ++i)
    txt2c (argv[i]);

  return 0;
}
