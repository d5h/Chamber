/* Copyright (C) Daniel Hipschman 2005 */
/* $Id: ttf2c.c,v 1.4 2005/04/25 22:49:20 sirdan Exp $ */

#include <SDL/SDL_ttf.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "save_surf.h"

/*
 * Only need printable characters. These are ' ' through '~'.
 */
#define NGLYPHS		95
#define FONT_START	' '

/*
 * Write the glyph with Unicode value 'ch' of font 'font' as
 * a C data structure to file 'f'.
 */
static int
process_glyph (TTF_Font * font, Uint16 ch, FILE * f)
{
  static char name[96];
  static SDL_Color color = { 0xFF, 0xFF, 0xFF, 0x0 };
  SDL_Surface *surf;
  int minx, miny, maxx, maxy, advance;

  if (TTF_GlyphMetrics (font, ch, &minx, &maxx, &miny, &maxy, &advance) < 0)
    return 0;

  surf = TTF_RenderGlyph_Solid (font, ch, color);

  if (surf->w)
    {
      sprintf (name, "pixels_%d", (int) ch);
      save_surf_pixels (f, surf, name);
    }
  else
    strcpy (name, "NULL");

  xprintf (f, "static raw_glyph glyph_%d /* (%c) */ = {\n", (int) ch,
	   (char) ch);
  xprintf (f, "  {\n");
  save_surf_info (f, surf, "NULL", name);
  xprintf (f, "\n  },\n");
  xprintf (f, "  %d /* minx */,\n  %d /* maxx */,\n  %d /* miny */,\n"
	   "  %d /* maxy */,\n  %d /* advance */\n};\n\n",
	   minx, maxx, miny, maxy, advance);

  SDL_FreeSurface (surf);
  return 1;
}

/*
 * Write the font 'font' called 'name' as a C data structure
 * to file 'name'.c. This file can be compiled as part of
 * an application which can then load the font from memory
 * instead of from disk.
 */
static void
process_font (TTF_Font * font, const char *name)
{
  static char done[NGLYPHS];
  Uint16 i;
  FILE *f = create_c_file (name);

  xprintf (f, "/* Automatically generated font file. */\n");
  xprintf (f, "#include \"font.h\"\n\n");
  xprintf (f, "extern raw_font %s_raw_font;\n\n", name);

  for (i = 0; i < NGLYPHS; ++i)
    done[i] = process_glyph (font, i + FONT_START, f);

  xprintf (f, "raw_font %s_raw_font = {\n  {\n", name);

  for (i = 0; i < NGLYPHS; ++i)
    if (done[i])
      xprintf (f, "    &glyph_%d,\n", i + FONT_START);
    else
      xprintf (f, "    NULL,\n");

  xprintf (f, "  },\n  %d /* ascent */,\n  %d /* descent */,\n",
	   TTF_FontAscent (font), TTF_FontDescent (font));
  xprintf (f,
	   "  %d /* height */,\n  %d /* line_skip */,\n  %d /* start */\n",
	   TTF_FontHeight (font), TTF_FontLineSkip (font), FONT_START);
  xprintf (f, "};\n\n");
}

/*
 * Process a font file.
 */
int
main (int argc, char **argv)
{
  TTF_Font *font;
  int ptsize;
  char *root;

  if (argc - 1 != 2)
    fatal ("ttf2c font.ttf ptsize");

  ptsize = atoi (argv[2]);
  if (ptsize <= 0)
    fatal ("Invalid point size");

  if (SDL_Init (SDL_INIT_VIDEO) == -1)
    fatal ("Initializing SDL: %s", SDL_GetError ());
  atexit (SDL_Quit);

  if (TTF_Init () == -1)
    fatal ("Initializing SDL_ttf: %s", TTF_GetError ());
  atexit (TTF_Quit);

  font = TTF_OpenFont (argv[1], ptsize);
  if (font == NULL)
    fatal ("Loading font: %s", TTF_GetError ());

  root = rootname (argv[1]);
  process_font (font, root);

  free (root);
  TTF_CloseFont (font);
  exit (EXIT_SUCCESS);
  return 0;
}
