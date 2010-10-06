/* Copyright (C) Daniel Hipschman, March 2005 */
/* $Id: scoremodes.c,v 1.9 2006/06/15 23:20:45 dsh Exp $ */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gamemode.h"
#include "gfxhelp.h"
#include "initialize.h"
#include "scoremodes.h"
#include "surfs.h"

static const char *scorefile;

/*
 * Both modes implemented by this file, set-score-mode and
 * show-scores-mode, share most of these globals.
 */

static struct
{
  char name[HIGHSCORER_NAMELEN + 1];
  unsigned long score;
} highscores[NHIGHSCORES];

static SDL_Surface *bgsurf = NULL;
static SDL_Surface *dsplsurf = NULL;

static font *gfont = NULL;	/* General font */
static SDL_Color default_gfont_color = { 0xFF, 0xFF, 0xFF, 0x00 };
static SDL_Color gfont_color;

static void
clear_scores (void)
{
  memset (highscores, 0, sizeof (highscores));
}

static void
parse_score_line (const char *line,
		  char name[], unsigned long *score, int lineno)
{
  /* Scan backwards for a number. This is the score. */
  /* Everything before the score is the name. */
  const char *pname, *pscore, *pend = line + strlen (line);
  int n;

  /* Discard newline and spaces after score. */
  while (pend > line)
    if (!isspace (*--pend))
      break;
  if (pend == line || !isdigit (*pend))
    fatal ("%s:%d: corruption: missing score", scorefile, lineno);

  /* Find start of score. */
  pscore = pend;
  while (pscore > line)
    if (!isdigit (*--pscore))
      break;
  if (!isspace (*pscore))	/* Need space to seperate from name. */
    fatal ("%s:%d: corruption: no name or garbled score", scorefile, lineno);
  ++pscore;

  /* FIXME: Check for overflow. */
  *score = strtoul (pscore, NULL, 10);

  /* Strip away whitespace after name. */
  pend = pscore - 1;
  while (pend > line)
    if (!isspace (*--pend))
      break;
  ++pend;

  /* Strip away whitespace before name. */
  pname = line;
  while (isspace (*pname++))
    if (pname == pend)
      fatal ("%s:%d: corruption: missing name", scorefile, lineno);
  --pname;

  n = MIN (pend - pname, HIGHSCORER_NAMELEN);
  strncpy (name, pname, n);
  name[n] = '\0';
}

void
init_scoremodes (void)
{
#define BUFFER_SZ	(HIGHSCORER_NAMELEN+16)
  static char buffer[BUFFER_SZ];
  FILE *fp;
  int i;

  scorefile = score_file_name ();
  dsplsurf = displaysurf ();
  clear_scores ();

  if (scorefile == NULL)
    return;

  fp = fopen (scorefile, "r");
  if (fp == NULL)
    {
      warning ("%s: couldn't open: %s", scorefile, strerror (errno));
      return;			/* File probably doesn't exist. */
    }

  for (i = 0; i < NHIGHSCORES; ++i)
    {
      if (fgets (buffer, BUFFER_SZ, fp) == NULL)
	{
	  if (ferror (fp))
	    fatal ("%s: read error: %s", scorefile, strerror (errno));
	  break;
	}
      if (buffer[0] != '\0'
	  && buffer[strlen (buffer) - 1] != '\n' && !feof (fp))
	fatal ("%s:%d: corruption: line too long", scorefile, i + 1);
      parse_score_line (buffer,
			highscores[i].name, &highscores[i].score, i + 1);
      if (i > 0 && highscores[i].score > highscores[i - 1].score)
	fatal ("%s:%d: corruption: scores unsorted", scorefile, i + 1);
    }

  /* Ignore remaining lines if there are any (might be nicer */
  /* if we complained about it, but for now I'm lazy) */
  fclose (fp);

#undef BUFFER_SZ
}

void
save_scores (void)
{
  if (scorefile != NULL)
    {
      FILE *fp = fopen (scorefile, "w");
      int i;

      if (fp == NULL)
	{
	  warning ("%s: couldn't save scores: %s",
		   scorefile, strerror (errno));
	  return;
	}

      for (i = 0; i < NHIGHSCORES; ++i)
	{
	  int s;
	  if (highscores[i].name[0] == '\0')
	    break;
	  s = fprintf (fp, "%-*s\t%lu\n", HIGHSCORER_NAMELEN,
		       highscores[i].name, highscores[i].score);
	  if (s == EOF)
	    {
	      warning ("%s: write error: %s", scorefile, strerror (errno));
	      break;
	    }
	}

      fclose (fp);
    }
}

bool
is_highscore (unsigned long score)
{
  return (highscores[NHIGHSCORES - 1].name[0] == '\0'
	  || score > highscores[NHIGHSCORES - 1].score);
}

static void
add_score (const char *name, unsigned long score)
{
  int i;
  for (i = 0; i < NHIGHSCORES; ++i)
    if (highscores[i].name[0] == '\0' || score > highscores[i].score)
      {
	if (highscores[i].name[0] != '\0' && i < NHIGHSCORES - 1)
	  memmove (&highscores[i + 1], &highscores[i],
		   (NHIGHSCORES - (i + 1)) * sizeof (highscores[0]));
	strncpy (highscores[i].name, name, HIGHSCORER_NAMELEN);
	highscores[i].name[HIGHSCORER_NAMELEN] = '\0';
	highscores[i].score = score;
	break;
      }
}

static void leave_setscoremode (void);

static struct
{
  char chars[HIGHSCORER_NAMELEN + 1];
  unsigned long score;
  int length;
  int y_placement;
  SDL_Rect patch, placement;
} entered;

static int
setscoremode_loopfunc (void)
{
  static SDL_Event event;
  coord pt;
  Uint16 code;
  SDLKey key;

  if (SDL_WaitEvent (&event) == 0)
    fatal ("%s", SDL_GetError ());

  switch (event.type)
    {
    case SDL_KEYDOWN:

      key = event.key.keysym.sym;
      if (key == SDLK_RETURN && entered.length > 0)
	{
	  add_score (entered.chars, entered.score);
	  leave_setscoremode ();
	  enter_showscoresmode (bgsurf);
	  return LOOPFUNC_CONTINUE;
	}
      else if (key == SDLK_BACKSPACE && entered.length > 0)
	{
	  entered.chars[--entered.length] = '\0';
	}
      else if (key == SDLK_ESCAPE)
	{
	  goto quit_case;
	}
      else
	{
	  code = event.key.keysym.unicode;
	  if (code > 127)
	    break;
	  if (isspace ((char) code) && entered.length == 0)
	    break;
	  if (isprint ((char) code) && entered.length < HIGHSCORER_NAMELEN)
	    entered.chars[entered.length++] = (char) code;
	  else
	    break;
	}

      apply_patch (bgsurf, &entered.patch, NULL);

      align_text (gfont, entered.chars, ALIGN_CENTER,
		  &pt, &entered.placement, NULL);
      /* Shift it down a little bit. */
      entered.placement.y += entered.y_placement - pt.y;
      blit_text (gfont, entered.chars, dsplsurf, pt.x, entered.y_placement);
      update_dirty (&entered.patch, &entered.placement);
      entered.patch = entered.placement;

      break;
    case SDL_QUIT:
    quit_case:
      if (entered.length > 0)
	add_score (entered.chars, entered.score);
      return LOOPFUNC_BREAK;
    }
  return LOOPFUNC_CONTINUE;
}

static void
reset_setscoremode (unsigned long score)
{
  static const char msg[] = "Enter your name:";
  coord pt;

  memset (entered.chars, 0, sizeof (entered.chars));
  memset (&entered.patch, 0, sizeof (entered.patch));
  entered.length = 0;
  entered.score = score;

  apply_patch (bgsurf, NULL, NULL);

  align_text (gfont, msg, ALIGN_CENTER, &pt, NULL, NULL);
  pt.y -= font_line_skip (gfont) / 2;
  blit_text (gfont, msg, dsplsurf, pt.x, pt.y);
  entered.y_placement = pt.y + font_line_skip (gfont);

  SDL_UpdateRect (dsplsurf, 0, 0, 0, 0);
}

void
enter_setscoremode (SDL_Surface * bg, unsigned long score)
{
  bgsurf = bg;
  if (gfont == NULL)
    {
      if (!get_color ("Font-Color", &gfont_color))
	gfont_color = default_gfont_color;
      gfont = acquire_medium_font (gfont_color);
    }

  reset_setscoremode (score);
  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE (1);
  loopfunc = setscoremode_loopfunc;
}

static void
leave_setscoremode (void)
{
  SDL_EnableUNICODE (0);
  SDL_EnableKeyRepeat (0, 0);
  release_font (gfont);
  gfont = NULL;
}

static void leave_showscoresmode (void);

static int
showscoresmode_loopfunc (void)
{
  static SDL_Event event;
  SDLKey key;

  if (SDL_WaitEvent (&event) == 0)
    fatal ("%s", SDL_GetError ());

  switch (event.type)
    {
    case SDL_KEYDOWN:

      key = event.key.keysym.sym;
      if (key != SDLK_ESCAPE)
	{
	  leave_showscoresmode ();
	  enter_gamemode ();
	  return LOOPFUNC_CONTINUE;
	}
      /* Fall-through */
    case SDL_QUIT:
      return LOOPFUNC_BREAK;
    }
  return LOOPFUNC_CONTINUE;
}

#define MARGIN			20
#define ULINE_THICKNESS		2

static void
reset_showscoresmode (void)
{
#define BUFFER_SZ	16
  static const char name_text[] = "Name:";
  static const char score_text[] = "Score:";
  static char buffer[BUFFER_SZ];
  SDL_Rect r;
  coord pt;
  Uint32 color;
  int lineskip = font_line_skip (gfont);
  int i;

  apply_patch (bgsurf, NULL, NULL);

  r.x = r.y = MARGIN;
  r.w = r.h = 0;
  align_text (gfont, name_text, ALIGN_TOPLEFT, &pt, NULL, &r);
  blit_text (gfont, name_text, dsplsurf, pt.x, pt.y);

  r.y += ULINE_THICKNESS + 2;	/* Space for line. */
  for (i = 0; i < NHIGHSCORES; ++i)
    {
      if (highscores[i].name[0] == '\0')
	break;
      r.y += lineskip;
      align_text (gfont, highscores[i].name, ALIGN_TOPLEFT, &pt, NULL, &r);
      blit_text (gfont, highscores[i].name, dsplsurf, pt.x, pt.y);
    }

  r.x = dispw () - MARGIN;
  r.y = MARGIN;
  align_text (gfont, score_text, ALIGN_TOPRIGHT, &pt, NULL, &r);
  blit_text (gfont, score_text, dsplsurf, pt.x, pt.y);

  r.y += ULINE_THICKNESS + 2;
  for (i = 0; i < NHIGHSCORES; ++i)
    {
      if (highscores[i].name[0] == '\0')
	break;
      r.y += lineskip;
      sprintf (buffer, "%lu", highscores[i].score);
      align_text (gfont, buffer, ALIGN_TOPRIGHT, &pt, NULL, &r);
      blit_text (gfont, buffer, dsplsurf, pt.x, pt.y);
    }

  r.x = MARGIN;
  r.y = MARGIN + font_ascent (gfont) + 1;
  r.w = dispw () - 2 * MARGIN;
  r.h = ULINE_THICKNESS;
  color = SDL_MapRGB (dsplsurf->format,
		      gfont_color.r, gfont_color.g, gfont_color.b);
  SDL_FillRect (dsplsurf, &r, color);

  SDL_UpdateRect (dsplsurf, 0, 0, 0, 0);

#undef BUFFER_SZ
}

void
enter_showscoresmode (SDL_Surface * bg)
{
  bgsurf = bg;
  if (gfont == NULL)
    {
      if (!get_color ("Font-Color", &gfont_color))
	gfont_color = default_gfont_color;
      gfont = acquire_medium_font (gfont_color);
    }

  reset_showscoresmode ();
  loopfunc = showscoresmode_loopfunc;
}

static void
leave_showscoresmode (void)
{
  release_font (gfont);
  gfont = NULL;
}
