/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: bmp2c.c,v 1.2 2005/04/25 22:49:20 sirdan Exp $ */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "save_surf.h"
#include "common.h"

static void
usage (void)
{
  xprintf (stderr,
	   "bmp2c [-tRRGGBB | -t] bitmap.bmp ...\n"
	   "	where -t gives the transparency color in hex,\n",
	   "	or if no argument is given, turns it off.\n");
  exit (EXIT_FAILURE);
}

int usekey = 0;
SDL_Color key;

/*
 * Writes the surface 's' to file 'fp' with name 'name'.c
 * as C code so that the image can be compiled directly
 * into a program and loaded later with special routines.
 */
static void
save_surf_as_c (FILE * fp, SDL_Surface * s, const char *name)
{
  int i;
  int ncolors = s->format->palette->ncolors;
  SDL_Color *colors = s->format->palette->colors;

  xprintf (fp, "/* Generated automatically from: %s.bmp */\n", name);
  xprintf (fp, "#include \"bitmap.h\"\n\n");

  xprintf (fp, "static SDL_Color colors [%d] = {\n", ncolors);
  for (i = 0; i < ncolors; ++i)
    xprintf (fp, "    { %d,%d,%d,0 }%s\n",
	     colors[i].r, colors[i].g, colors[i].b,
	     i < ncolors - 1 ? "," : "");
  xprintf (fp, "};\n\n");

  save_surf_pixels (fp, s, "pixels");

  xprintf (fp, "extern bitmap %s_bitmap;\n", name);
  xprintf (fp, "bitmap %s_bitmap = {\n", name);

  save_surf_info (fp, s, "colors", "pixels");
  xprintf (fp, " };\n\n");
}

/*
 * Opens the file given by 'fname' which should be a Windows BMP.
 * It then converts the image data of the bitmap into C code that
 * can be compiled into an application and loaded at runtime.
 * The C file has the same name as the bitmap file with the
 * extension replaced by '.c'.
 */
static void
bmp2c (const char *fname)
{
  Uint32 color;
  FILE *fp;
  SDL_Surface *s;
  char *newname = rootname (fname);

  s = SDL_LoadBMP (fname);
  if (s == NULL)
    fatal ("%s", SDL_GetError ());
  if (s->format->BitsPerPixel != 8)
    fatal ("Only supporting paletted bitmaps right now");

  if (usekey)
    {
      color = SDL_MapRGB (s->format, key.r, key.g, key.b);
      if (SDL_SetColorKey (s, SDL_SRCCOLORKEY, color) == -1)
	fatal ("%s", SDL_GetError ());
    }

  fp = create_c_file (newname);
  save_surf_as_c (fp, s, newname);
  fclose (fp);

  SDL_FreeSurface (s);
  free (newname);
}

/*
 * Converts a hex digit into the corresponding integer.
 */
#define parsehex(x) (isdigit (x) ? (x) - '0' : tolower (x) - 'a' + 10)

/*
 * Handles the -t option. If no argument is given, turn off transparency
 * for subsequent files. If a six digit hex value is given in 'arg',
 * interpret it as 3 bytes: red, green and blue intensity.
 */
static void
color_key_option (const char *arg)
{
  int i;

  /* -t turn off color keys */
  if (*arg == '\0')
    {
      usekey = 0;
      return;
    }

  /* -tRRGGBB set color key */
  for (i = 0; i < 6; ++i)
    if (!isxdigit (arg[i]))
      fatal ("not a 6-digit hex after -t");
  usekey = 1;

  key.r = (parsehex (arg[0]) << 4) | parsehex (arg[1]);
  key.g = (parsehex (arg[2]) << 4) | parsehex (arg[3]);
  key.b = (parsehex (arg[4]) << 4) | parsehex (arg[5]);
}

/*
 * Process a list of files and options.
 */
int
main (int argc, char **argv)
{
  int i;

  if (argc == 1)
    usage ();

  if (SDL_Init (SDL_INIT_VIDEO) == -1)
    fatal ("%s", SDL_GetError ());
  atexit (SDL_Quit);

  for (i = 1; i < argc; ++i)
    {
      if (argv[i][0] == '-' && argv[i][1] == 't')
	color_key_option (argv[i] + 2);
      else
	bmp2c (argv[i]);
    }
  return 0;
}
