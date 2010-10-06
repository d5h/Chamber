/* Copyright (C) Daniel Hipschman 2005 */
/* $Id: font.c,v 1.9 2006/06/15 23:20:45 dsh Exp $ */

#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "font.h"
#include "initialize.h"
#include "surfs.h"

static void
make_glyph (raw_glyph * rg, glyph * g, SDL_Color c)
{
  SDL_Color colors[2];
  colors[0].r = 255 - c.r;
  colors[0].g = 255 - c.g;
  colors[0].b = 255 - c.b;
  colors[1] = c;
  rg->bmp.colors = colors;
  rg->bmp.ncolors = 2;

  g->surf = rg->bmp.pixels ? makesurf (&rg->bmp) : NULL;

  g->minx = rg->minx;
  g->maxx = rg->maxx;
  g->miny = rg->miny;
  g->maxy = rg->maxy;
  g->advance = rg->advance;
}

static void
free_glyph (glyph * g)
{
  if (g->surf)
    SDL_FreeSurface (g->surf);
  g->minx = 0;
  g->maxx = 0;
  g->miny = 0;
  g->maxy = 0;
  g->advance = 0;
  g->surf = NULL;
}

void
make_font (raw_font * rf, font * f, SDL_Color c)
{
  int i;

  for (i = 0; i < GLYPHS_PER_FONT; ++i)
    make_glyph (rf->glyphs[i], &f->glyphs[i], c);

  f->ascent = rf->ascent;
  f->descent = rf->descent;
  f->height = rf->height;
  f->line_skip = rf->line_skip;
  f->start = rf->start;
  f->acquired = false;
}

void
free_font (font * f)
{
  int i;

  for (i = 0; i < GLYPHS_PER_FONT; ++i)
    free_glyph (&f->glyphs[i]);

  f->ascent = 0;
  f->descent = 0;
  f->height = 0;
  f->line_skip = 0;
  f->acquired = false;
  f->start = '\0';
}

void
set_glyph_color (glyph * g, SDL_Color fg)
{
  SDL_Color colors[2];
  if (g->surf)
    {
      colors[0].r = 255 - fg.r;
      colors[0].g = 255 - fg.g;
      colors[0].b = 255 - fg.b;
      colors[1] = fg;
      SDL_SetPalette (g->surf, SDL_LOGPAL, colors, 0, 2);
    }
}

void
set_font_color (font * f, SDL_Color fg)
{
  int i;
  for (i = 0; i < GLYPHS_PER_FONT; ++i)
    set_glyph_color (&f->glyphs[i], fg);
}

void
text_size (font * f, const char *t, SDL_Rect * r)
{
  glyph *g;
  int x = 0, y = 0, ascent = f->ascent;
  int left = INT_MAX, right = INT_MIN, top = INT_MAX, bottom = INT_MIN;

  for (; *t; ++t)
    {
      if (*t == '\n')
	{
	  y += f->line_skip;
	  x = 0;
	}
      else
	{
	  int z, i = *t - f->start;

	  if (i < 0 || i >= GLYPHS_PER_FONT)
	    continue;
	  g = &f->glyphs[i];

	  if ((z = x + g->minx) < left)
	    left = z;
	  if ((z = y + ascent - g->maxy) < top)
	    top = z;
	  if ((z = x + g->maxx) > right)
	    right = z;
	  if ((z = y + ascent - g->miny) > bottom)
	    bottom = z;

	  x += g->advance;
	}
    }

  r->x += left;
  r->y += top;
  r->w = right - left;
  r->h = bottom - top;
}

void
blit_text (font * f, const char *t, SDL_Surface * s, int x, int y)
{
  SDL_Rect src, dst;
  glyph *g;
  int sx = x, ascent = f->ascent;

  src.y = src.x = 0;

  for (; *t; ++t)
    {
      if (*t == '\n')
	{
	  y += f->line_skip;
	  x = sx;
	}
      else
	{
	  int i = *t - f->start;

	  if (i < 0 || i >= GLYPHS_PER_FONT)
	    continue;
	  g = &f->glyphs[i];

	  if (g->surf)
	    {
	      dst.x = x + g->minx;
	      dst.y = y + ascent - g->maxy;
	      src.w = g->maxx - g->minx;
	      src.h = g->maxy - g->miny;
	      /* TODO: Put some adjust_rect code here. */
	      SDL_BlitSurface (g->surf, &src, s, &dst);
	    }

	  x += g->advance;
	}
    }
}

void
align_text (font * fnt, const char *text, unsigned flags,
	    coord * blitpt, SDL_Rect * urect, const SDL_Rect * ref)
{
  SDL_Rect r;

  if (ref == NULL)
    ref = display_rect ();;

  r.x = r.y = 0;
  text_size (fnt, text, &r);

  if (flags & ALIGN_LEFT)
    blitpt->x = ref->x - r.x;
  else if (flags & ALIGN_RIGHT)
    blitpt->x = (ref->x + ref->w) - (r.x + r.w);
  else
    blitpt->x = (ref->x - r.x) + (ref->w - r.w) / 2;

  if (flags & ALIGN_TOP)
    blitpt->y = ref->y - r.y;
  else if (flags & ALIGN_BOTTOM)
    blitpt->y = (ref->y + ref->h) - (r.y + r.h);
  else
    blitpt->y = (ref->y - r.y) + (ref->h - r.h) / 2;

  if (urect)
    {
      urect->x = blitpt->x + r.x;
      urect->y = blitpt->y + r.y;
      urect->w = r.w;
      urect->h = r.h;
    }
}
