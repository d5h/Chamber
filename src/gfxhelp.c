/* Copyright (C) Daniel Hipschman, March 2005 */
/* $Id: gfxhelp.c,v 1.6 2006/06/15 23:20:45 dsh Exp $ */

#include <string.h>

#include "defines.h"
#include "gfxhelp.h"
#include "surfs.h"

SDL_Surface *dsplsurf = NULL;

void
init_gfxhelp (void)
{
  dsplsurf = displaysurf ();
}

void
adjust_draw_rects (SDL_Rect * src, SDL_Rect * dst)
{
  /* Assume src.w == dst.w && src.h == dst.h. */
  int display_width = dispw ();
  int display_height = disph ();

  /* At least partially off screen to the left. */
  if (dst->x < 0)
    {
      /* Totally off screen. */
      if (dst->x + dst->w <= 0)
	goto null_rects;
      dst->w += dst->x;
      src->w += dst->x;
      src->x -= dst->x;
      dst->x = 0;
    }

  /* At least partially off screen to the right. */
  if (dst->x + dst->w > display_width)
    {
      /* Totally off screen. */
      if (dst->x > display_width)
	goto null_rects;
      src->w = dst->w = display_width - dst->x;
    }

  /* At least partially above screen. */
  if (dst->y < 0)
    {
      /* Totally off screen. */
      if (dst->y + dst->h <= 0)
	goto null_rects;
      dst->h += dst->y;
      src->h += dst->y;
      src->y -= dst->y;
      dst->y = 0;
    }

  /* At least partially below screen. */
  if (dst->y + dst->h > display_height)
    {
      /* Totally off screen. */
      if (dst->y > display_height)
	goto null_rects;
      src->h = dst->h = display_height - dst->y;
    }

  return;

null_rects:
  memset (src, 0, sizeof (SDL_Rect));
  memset (dst, 0, sizeof (SDL_Rect));
}

void
apply_patch (SDL_Surface * bgsurf, SDL_Rect * p, SDL_Rect * d)
{
  SDL_Rect src, dst;
  const SDL_Rect *patch = (p ? p : display_rect ());

  int x, y, bx, by;
  int xi = patch->x;
  int yi = patch->y;
  int xf = xi + patch->w;
  int yf = yi + patch->h;
  int bw = bgsurf->w;
  int bh = bgsurf->h;

  for (y = yi; y < yf; y += bh - by)
    {
      by = y % bh;
      for (x = xi; x < xf; x += bw - bx)
	{
	  bx = x % bw;

	  src.x = bx;
	  src.y = by;
	  src.w = MIN (bw - bx, xf - x);
	  src.h = MIN (bh - by, yf - y);
	  dst.x = x;
	  dst.y = y;
	  SDL_BlitSurface (bgsurf, &src, dsplsurf, &dst);
	}
    }

  if (d)
    {
      d->x = xi;
      d->y = yi;
      d->w = xf - xi;
      d->h = yf - yi;
    }
}

void
update_dirty (SDL_Rect * a, SDL_Rect * b)
{
  if (a->w == 0 && b->w == 0)
    return;

  if (a->w == 0)
    SDL_UpdateRects (dsplsurf, 1, b);
  else if (b->w == 0)
    SDL_UpdateRects (dsplsurf, 1, a);

  else
    {
      int x = MIN (a->x, b->x);
      int y = MIN (a->y, b->y);
      int w = MAX (a->x + a->w, b->x + b->w) - x;
      int h = MAX (a->y + a->h, b->y + b->h) - y;
      SDL_UpdateRect (dsplsurf, x, y, w, h);
    }
}
