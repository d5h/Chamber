/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: main.c,v 1.8 2006/06/15 23:20:45 dsh Exp $ */

#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "initialize.h"
#include "gamemode.h"
#include "scoremodes.h"

extern char usage_txt[];
extern char help_txt[];

loopfunction *loopfunc;
bool debugging = false;
const char *debugfile;

static bool verbose = false;

typedef void option_function (const char *);

typedef struct
{
  const char *name;
  char abbrev;
  option_function *func;
} option;

static void
debug_func (const char *arg)
{
  debugging = true;
  debugfile = arg;
}

static void
help_func (const char *arg)
{
  no_warn_about_unused (arg);

  printf ("%s %s\n", PROGRAM, VERSION);
  fputs (usage_txt, stdout);

  if (!verbose)
    exit (EXIT_SUCCESS);

  fputs (help_txt, stdout);

  exit (EXIT_SUCCESS);
}

static void
verbose_func (const char *arg)
{
  no_warn_about_unused (arg);
  verbose = true;
}

static void
version_func (const char *arg)
{
  no_warn_about_unused (arg);
  printf ("%s\n", VERSION);
  exit (EXIT_SUCCESS);
}

#define NOPTIONS	((int) (sizeof (options) / sizeof (options[0])))

static option options[] = {
  {"debug", '\0', debug_func},
  {"help", 'h', help_func},
  {"verbose", 'v', verbose_func},
  {"version", '\0', version_func}
};

static bool
long_option (const char *opt)
{
  char *arg;
  int i, n;

  arg = strchr (opt, '=');
  if (arg)
    {
      n = arg - opt;
      ++arg;
    }
  else
    n = strlen (opt);


  for (i = 0; i < NOPTIONS; ++i)
    if (strncmp (opt, options[i].name, n) == 0)
      {
	options[i].func (arg);
	return true;
      }

  return false;
}

static void
short_options (const char *opts)
{
  for (; *opts; ++opts)
    {
      int i;
      bool ok = false;

      for (i = 0; i < NOPTIONS; ++i)
	if (options[i].abbrev == *opts)
	  {
	    options[i].func (NULL);
	    ok = true;
	    break;
	  }

      if (!ok)
	fatal ("unknown option -%c", *opts);
    }
}

static void
process_options (char **argv)
{
  for (; *argv; ++argv)
    {
      char *opt = *argv;

      if (opt[0] == '-' && opt[1] == '-')
	{
	  if (!long_option (opt + 2))
	    fatal ("unknown option %s", opt);
	}
      else if (opt[0] == '-')
	{
	  if (!long_option (opt + 1))
	    short_options (opt + 1);
	}
    }
}

int
main (int argc, char **argv)
{
  no_warn_about_unused (argc);

  process_options (argv + 1);
  initialize ();

  enter_gamemode ();
  while ((*loopfunc) () != LOOPFUNC_BREAK);

  cleanup ();
  return 0;
}
