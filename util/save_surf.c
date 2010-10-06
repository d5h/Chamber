/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: save_surf.c,v 1.7 2005/04/25 22:49:20 sirdan Exp $ */

#include <SDL/SDL.h>
#include <stdio.h>
#include "save_surf.h"
#include "common.h"

/*
 * Write the pixel data of 's' to file 'fp' as a C data
 * structure named 'name'.
 */
void
save_surf_pixels (FILE * fp, SDL_Surface * s, const char *name)
{
  int i, j;

  Uint8 *pixels;
  xprintf (fp, "static Uint8 %s [%d][%d] = {\n", name, s->h, s->w);
  pixels = (Uint8 *) s->pixels;
  for (i = 0; i < s->h; ++i)
    {
      xprintf (fp, "    { %d", (int) *pixels++);
      for (j = 1; j < s->pitch; ++j)
	xprintf (fp, ",%d", (int) *pixels++);
      xprintf (fp, " }%s\n", i < s->h - 1 ? "," : "");
    }
  xprintf (fp, "};\n\n");
}

/*
 * Write info such as width and height (and whatever else is
 * relevant) about 's' to file 'fp'.
 */
void
save_surf_info (FILE * fp, SDL_Surface * s,
		const char *colors_name, const char *pixels_name)
{
  xprintf (fp, "\
    %d /* width */, %d /* height */,\n\
    %d /* depth */, %d /* pitch */,\n\
    %d /* ncolors */,\n\
    %d /* has_key */, %lu /* key */,\n\
    %s, %s", s->w, s->h, s->format->BitsPerPixel, s->pitch, s->format->palette->ncolors, s->flags & SDL_SRCCOLORKEY, s->format->colorkey, colors_name, pixels_name);
}
