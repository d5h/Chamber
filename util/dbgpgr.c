/* Copyright (C) Daniel Hipschman, May 2005 */
/* $Id: dbgpgr.c,v 1.2 2005/05/08 18:08:30 sirdan Exp sirdan $ */

#include <stdio.h>
#include <stdlib.h>

#define MAX_LINES	500	/* Number of lines to buffer. */
#define LINE_LEN	80	/* Length of buffered lines. */

char lines[MAX_LINES][LINE_LEN];
int idx;

#define AFTER(i)	((i)+1 == MAX_LINES ? 0 : (i)+1)

/*
 * Read lines from stdin and buffer some number of them.
 * If we find a blank line (end of stats report), spit out what we've
 * got so far.  
 */
int
main (void)
{
  while (fgets (lines[idx], LINE_LEN, stdin))
    {
      if (lines[idx][0] == '\n')
	{
	  /*
	   * Dump what we've buffered so far.
	   */
	  int i;
	  for (i = AFTER (idx); i != idx; i = AFTER (i))
	    {
	      if (lines[i][0])
		fputs (lines[i], stdout);
	      /* Once we've displayed it, remove it. */
	      lines[i][0] = '\0';
	    }
	}
      else
	idx = AFTER (idx);
    }

  return 0;
}
