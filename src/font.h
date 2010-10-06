/* Copyright (C) Daniel Hipschman 2005 */
/* $Id: font.h,v 1.9 2006/06/15 23:20:45 dsh Exp $ */

#ifndef UTIL_FONT_H
#define UTIL_FONT_H

#include <SDL/SDL.h>
#include "defines.h"
#include "bitmap.h"

/* Only printable ASCII characters */
#define GLYPHS_PER_FONT 95

typedef struct
{
  bitmap bmp;
  int minx, maxx, miny, maxy, advance;
} raw_glyph;

typedef struct
{
  raw_glyph *glyphs[GLYPHS_PER_FONT];
  int ascent, descent, height, line_skip;
  char start;
} raw_font;

typedef struct
{
  SDL_Surface *surf;
  int minx, maxx, miny, maxy, advance;
} glyph;

typedef struct
{
  glyph glyphs[GLYPHS_PER_FONT];
  int ascent, descent, height, line_skip;
  bool acquired;
  char start;
} font;

#define font_height(f)		((f)->height)
#define font_line_skip(f)	((f)->line_skip)
#define font_ascent(f)		((f)->ascent)
#define font_descent(f)		((f)->descent)

void init_font_utils (void);
void cleanup_font_utils (void);

void make_font (raw_font *, font *, SDL_Color);
void free_font (font *);
void set_glyph_color (glyph *, SDL_Color);
void set_font_color (font *, SDL_Color);

/*
 * This next one is a tricky function:
 * text_size(font, string, rect)
 * expects that string will be blitted at rect->x, rect->y and so it
 * sets rect to the area that will need to be updated to display
 * the text. It may change rect->x and rect->y, so it is NOT correct
 * to blit the text at these new coordinates. E.g,
 * THIS IS *NOT* CORRECT:
 *	rect->x = rect->y = 100;
 *	text_size (font, "HELLO", rect);
 *	blit_text (font, "HELLO", rect->x, rect->y);
 *	SDL_UpdateRects (screen, 1, rect);
 * But rather you should do this:
 *	rect->x = rect->y = 100;
 *	text_size (font, "HELLO", rect);
 *	blit_text (font, "HELLO", 100, 100);
 *	SDL_UpdateRects (screen, 1, rect);
 */
void text_size (font *, const char *, SDL_Rect *);
void blit_text (font *, const char *, SDL_Surface *, int x, int y);

#define ALIGN_CENTER		0x00U
#define ALIGN_LEFT		0x01U
#define ALIGN_RIGHT		0x02U
#define ALIGN_TOP		0x04U
#define ALIGN_BOTTOM		0x08U
#define ALIGN_TOPLEFT		(ALIGN_TOP | ALIGN_LEFT)
#define ALIGN_TOPRIGHT		(ALIGN_TOP | ALIGN_RIGHT)
#define ALIGN_BOTTOMLEFT	(ALIGN_BOTTOM | ALIGN_LEFT)
#define ALIGN_BOTTOMRIGHT	(ALIGN_BOTTOM | ALIGN_RIGHT)
#define ALIGN_CENTERLEFT	(ALIGN_CENTER | ALIGN_LEFT)
#define ALIGN_CENTERRIGHT	(ALIGN_CENTER | ALIGN_RIGHT)
#define ALIGN_CENTERTOP		(ALIGN_CENTER | ALIGN_TOP)
#define ALIGN_CENTERBOTTOM	(ALIGN_CENTER | ALIGN_BOTTOM)

void align_text (font * fnt, const char *text, unsigned flags,
		 coord * blitpt, SDL_Rect * urect, const SDL_Rect * ref);

#endif /* UTIL_FONT_H */
