/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: bitmap.c,v 1.5 2006/06/15 23:20:45 dsh Exp $ */

#include "defines.h"
#include "bitmap.h"

SDL_Surface *
makesurf (bitmap * bmp)
{
  int s;

  SDL_Surface *surf = SDL_CreateRGBSurfaceFrom (bmp->pixels,
						bmp->width,
						bmp->height,
						bmp->depth,
						bmp->pitch,
						0, 0, 0, 0);
  if (!surf)
    fatal ("creating a surface: %s", SDL_GetError ());

  if (bmp->depth == 8)
    {
      s = SDL_SetPalette (surf, SDL_LOGPAL, bmp->colors, 0, bmp->ncolors);
      if (s == -1)
	fatal ("setting a palette: %s", SDL_GetError ());
    }

  if (bmp->has_key)
    {
      s = SDL_SetColorKey (surf, SDL_SRCCOLORKEY, bmp->key);
      if (s == -1)
	fatal ("setting a color key: %s", SDL_GetError ());
    }

  return surf;
}
