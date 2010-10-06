/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: initialize.c,v 1.13 2006/06/15 23:20:45 dsh Exp $ */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bitmaplist.h"
#include "defines.h"
#include "font.h"
#include "fontlist.h"
#include "gamemode.h"
#include "gfxhelp.h"
#include "initialize.h"
#include "scoremodes.h"
#include "surfs.h"

#define DEFAULT_DISPLAY_WIDTH	600
#define DEFAULT_DISPLAY_HEIGHT	400


/* Set this in a Makefile or something. */
#ifndef CONFIGFILE
#  define CONFIGFILE NULL
#endif

static SDL_Joystick *joystick;

static char *score_file;

const char *
score_file_name (void)
{
  assert (score_file);
  return score_file;
}

static symtab config = NULL;

const char *
get_config (const char *name)
{
  return symtab_find (config, name);
}

static bool isinit = false;	/* Have we already done our work? */

static SDL_Surface *ballsurfs[NBALLSURFS];
static SDL_Surface *padsurfs[NPADSURFS];
static SDL_Surface *bgsurfs[NBGSURFS];

real blackout_radius;

static int display_width;
static int display_height;

int
dispw (void)
{
  assert (display_width);
  return display_width;
}

int
disph (void)
{
  assert (display_height);
  return display_height;
}

/*
 * Fonts are big, so we only keep a few of them around.
 * This is how many medium-sized fonts we have.
 */
#define MAX_MEDIUM_FONTS	1
static font medium_fonts[MAX_MEDIUM_FONTS];

static SDL_Surface *display;	/* Da screen. */

static symtab color_table = NULL;

static void
init_color_table (void)
{
  /* Fixme: Add the rest. */
  symtab_add (&color_table, "Aqua", "00ffff");
  symtab_add (&color_table, "AquaMarine", "7fffd4");
  symtab_add (&color_table, "Azure", "f0ffff");
  symtab_add (&color_table, "Beige", "f5f5dc");
  symtab_add (&color_table, "Black", "000000");
  symtab_add (&color_table, "Blue", "0000ff");
  symtab_add (&color_table, "Brown", "a52a2a");
  symtab_add (&color_table, "Chocolate", "d2691e");	/* Mmm... */
  symtab_add (&color_table, "Crimson", "dc143c");
  symtab_add (&color_table, "Cyan", "00ffff");
  symtab_add (&color_table, "DarkBlue", "00008b");
  symtab_add (&color_table, "DarkCyan", "008b8b");
  symtab_add (&color_table, "DarkGray", "a9a9a9");
  symtab_add (&color_table, "DarkGreen", "006400");
  symtab_add (&color_table, "DarkGrey", "a9a9a9");
  symtab_add (&color_table, "DarkKhaki", "bdb76b");
  symtab_add (&color_table, "DarkMagenta", "8b008b");
  symtab_add (&color_table, "DarkRed", "8b0000");
  symtab_add (&color_table, "FireBrick", "b22222");
  symtab_add (&color_table, "ForestGreen", "228b22");
  symtab_add (&color_table, "Fuchsia", "ff00ff");
  symtab_add (&color_table, "GhostWhite", "f8f8ff");	/* Boo! */
  symtab_add (&color_table, "Gray", "808080");
  symtab_add (&color_table, "Grey", "808080");
  symtab_add (&color_table, "Green", "008000");
  symtab_add (&color_table, "GreenYellow", "adff2f");
  symtab_add (&color_table, "Indigo", "4b0082");
  symtab_add (&color_table, "Ivory", "fffff0");
  symtab_add (&color_table, "Khaki", "f0e68c");
  symtab_add (&color_table, "Lavender", "e6e6fa");
  symtab_add (&color_table, "LawnGreen", "7cfc00");
  symtab_add (&color_table, "LightBlue", "add8e6");
  symtab_add (&color_table, "LightCyan", "e0ffff");
  symtab_add (&color_table, "LightGray", "d3d3d3");
  symtab_add (&color_table, "LightGreen", "90ee90");
  symtab_add (&color_table, "LightGrey", "d3d3d3");
  symtab_add (&color_table, "LightYellow", "ffffe0");
  symtab_add (&color_table, "Lime", "00ff00");
  symtab_add (&color_table, "LimeGreen", "32cd32");
  symtab_add (&color_table, "Magenta", "ff00ff");
  symtab_add (&color_table, "Maroon", "800000");
  symtab_add (&color_table, "MediumBlue", "0000cd");
  symtab_add (&color_table, "MediumPurple", "9370db");
  symtab_add (&color_table, "MidnightBlue", "191970");
  symtab_add (&color_table, "Navy", "000080");
  symtab_add (&color_table, "Olive", "808000");
  symtab_add (&color_table, "Orange", "ffa500");
  symtab_add (&color_table, "OrangeRed", "ff4500");
  symtab_add (&color_table, "Orchid", "da70d6");
  symtab_add (&color_table, "PaleGreen", "98fb98");
  symtab_add (&color_table, "Pink", "ffc0cb");
  symtab_add (&color_table, "Plum", "dda0dd");
  symtab_add (&color_table, "Purple", "800080");
  symtab_add (&color_table, "Red", "ff0000");
  symtab_add (&color_table, "RoyalBlue", "4169e1");
  symtab_add (&color_table, "SeaGreen", "2e8b57");
  symtab_add (&color_table, "Silver", "c0c0c0");
  symtab_add (&color_table, "SkyBlue", "87ceeb");
  symtab_add (&color_table, "Snow", "fffafa");
  symtab_add (&color_table, "SpringGreen", "00ff7f");
  symtab_add (&color_table, "Tan", "d2b48c");
  symtab_add (&color_table, "Teal", "008080");
  symtab_add (&color_table, "Tomato", "ff6347");
  symtab_add (&color_table, "Turquoise", "40e0d0");
  symtab_add (&color_table, "Violet", "ee82ee");
  symtab_add (&color_table, "White", "ffffff");
  symtab_add (&color_table, "Yellow", "ffff00");
  symtab_add (&color_table, "YellowGreen", "9acd32");
}

static void
set_score_file_name (void)
{
  static const char *path = ".chamber/scores";
  const char *home = getenv ("HOME");

  if (home)
    {
      score_file = XALLOC (char, strlen (home) + strlen (path) + 2);
      sprintf (score_file, "%s/%s", home, path);
    }
}

bool
get_int (const char *name, int *value)
{
  static const char *limit = "32767";
  static int len_limit;

  const char *user_str, *scan, *digits;
  int len_digits;

  if (len_limit == 0)
    len_limit = strlen (limit);

  user_str = get_config (name);
  if (user_str == NULL)
    return false;

  scan = user_str;
  if (*scan == '+' || *scan == '-')
    ++scan;
  while (*scan == '0')
    ++scan;
  digits = scan;

  for (; *scan; ++scan)
    if (!isdigit (*scan))
      return false;

  len_digits = strlen (digits);
  if (len_digits > len_limit)
    return false;

  if (len_digits == len_limit && strcmp (digits, limit) > 0)
    return false;

  *value = atoi (user_str);
  return true;
}

static int
get_dimension (const char *name, int dfault, int min_dispv)
{
  int user_val;
  if (get_int (name, &user_val))
    return MAX (user_val, min_dispv);
  else
    return MAX (dfault, min_dispv);
}

static void
create_display (void)
{
  int min_dispw = DEFAULT_DISPLAY_WIDTH;	/* STUB */
  int min_disph = DEFAULT_DISPLAY_HEIGHT;	/* STUB */

  display_width = get_dimension ("Display-Width",
				 DEFAULT_DISPLAY_WIDTH, min_dispw);
  display_height = get_dimension ("Display-Height",
				  DEFAULT_DISPLAY_HEIGHT, min_disph);

  display = SDL_SetVideoMode (display_width, display_height,
			      0, SDL_ANYFORMAT);
  if (!display)
    fatal ("creating the display surface: %s", SDL_GetError ());

  SDL_WM_SetCaption (PROGRAM " " VERSION, PROGRAM " " VERSION);
}

const SDL_Rect *
display_rect (void)
{
  static SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.w = display_width;
  r.h = display_height;
  return &r;
}

static void
read_config (symtab * tab, const char *filename)
{
/*
 * Fixme: Yeah, I know this is a really big buffer, that will hold any
 * reasonably sized line, it's still not proper and should be dynamic.
 */
#define BUFFER_SIZE	4096
  static char buffer[BUFFER_SIZE];
  int lineno = 0;

  FILE *fp = fopen (filename, "r");
  if (fp == NULL)
    {
      warning ("couldn't open configuration file %s: %s",
	       filename, strerror (errno));
      return;
    }

  while (fgets (buffer, BUFFER_SIZE, fp))
    {
      char *name, *value;

      ++lineno;
      /* See if we've exceeded the buffer. */
      if (buffer[0] != '\0'
	  && buffer[strlen (buffer) - 1] != '\n' && !feof (fp))
	fatal ("%s:%d: line too long (sorry)", filename, lineno);

      name = first_nonspace (buffer);
      if (*name == '\0' || *name == '#')
	continue;

      /* Expecting a NAME=VALUE pair here. */
      value = strchr (name, '=');
      if (value == NULL)
	fatal ("%s:%d: line is not a valid NAME=VALUE pair",
	       filename, lineno);
      *value = '\0';
      ++value;

      /* Discard leading and trailing spaces. */
      *one_after_last_nonspace (name) = '\0';
      value = first_nonspace (value);
      *one_after_last_nonspace (value) = '\0';

      /* Should I check to make sure name is not empty? */
      symtab_add (tab, name, value);
    }

  if (ferror (fp))
    fatal ("%s: read error: %s", filename, strerror (errno));

  fclose (fp);

#undef BUFFER_SIZE
}

static void
init_fonts (void)
{
  SDL_Color c = { 0xFF, 0xFF, 0xFF, 0x00 };
  int i;
  for (i = 0; i < MAX_MEDIUM_FONTS; ++i)
    make_font (&medium_font_raw_font, &medium_fonts[i], c);
}

static void
free_fonts (void)
{
  int i;
  for (i = 0; i < MAX_MEDIUM_FONTS; ++i)
    free_font (&medium_fonts[i]);
}

static SDL_Surface *
loadsurf (const char *name, bitmap * dfault)
{
  SDL_Surface *surf = NULL;
  const char *option;

  option = get_config (name);
  if (option && option[0])
    {
      surf = load_image (option);
      if (surf == NULL)
	warning ("%s: couldn't load image: %s", option, SDL_GetError ());
    }
  if (surf == NULL)
    surf = makesurf (dfault);

  /* Is this even possible?  Well, if it is, it is sure to screw us up. */
  if (surf->w == 0 || surf->h == 0)
    fatal ("%s has zero dimension", name);

  return surf;
}

static void
maybe_add_transparency (const char *name, SDL_Surface * img)
{
  static const char tsuffix[] = "-transparent";

  /* Default bitmaps will already have a color-key. */
  if ((img->flags & SDL_SRCCOLORKEY) == 0)
    {
      SDL_Color tcolor;

      char *tname = XALLOC (char, strlen (name) + strlen (tsuffix) + 1);
      sprintf (tname, "%s%s", name, tsuffix);

      if (get_color (tname, &tcolor))
	{
	  Uint32 key = SDL_MapRGB (img->format,
				   tcolor.r,
				   tcolor.g,
				   tcolor.b);
	  if (SDL_SetColorKey (img, SDL_SRCCOLORKEY, key))
	    warning ("%s: couldn't set transparency: %s",
		     name, SDL_GetError ());
	}

      xfree (tname);
    }
}

static SDL_Surface *
loadball (const char *name, bitmap * dfault)
{
  static int diameter = 0;

  SDL_Surface *img = loadsurf (name, dfault);

  if (img->w != img->h)
    fatal ("%s is not square", name);

  if (diameter == 0)
    {
      diameter = img->w;
      /* Hopefully maintain some sanity! */
      if (diameter > MIN (display_width, display_height) / 4)
	fatal ("%s exceeds maximum ball size", name);
    }

  if (img->w != diameter)
    fatal ("%s is not the same size as the other balls", name);

  maybe_add_transparency (name, img);
  return img;
}

static SDL_Surface *
loadpad (const char *name, bitmap * dfault)
{
  static int width = 0;
  static int height = 0;

  SDL_Surface *img = loadsurf (name, dfault);

  if (img->h > img->w)
    fatal ("%s is taller than it is wide", name);

  if (width == 0)
    {
      width = img->w;
      height = img->h;
      /* Maintain sanity with more or less arbitrary restrictions. */
      if (width > display_width / 2)
	fatal ("%s exceeds maximum paddle length", name);
      if (height > display_height / 4)
	fatal ("%s exceeds maximum paddle thickness", name);
    }

  if (img->w != width || img->h != height)
    fatal ("%s is not the same size as the other paddles", name);

  maybe_add_transparency (name, img);
  return img;
}

static void
init_surfs (void)
{
  ballsurfs[REDBALL] = loadball ("Ball-1", &redball_bitmap);
  ballsurfs[GREENBALL] = loadball ("Ball-2", &greenball_bitmap);
  ballsurfs[BLUEBALL] = loadball ("Ball-3", &blueball_bitmap);
  ballsurfs[YELLOWBALL] = loadball ("Ball-4", &yellowball_bitmap);
  ballsurfs[PURPLEBALL] = loadball ("Ball-5", &purpleball_bitmap);
  ballsurfs[CYANBALL] = loadball ("Ball-6", &cyanball_bitmap);
  ballsurfs[BLACKOUT] = loadball ("Blackout-Ball", &blackout_bitmap);
  ballsurfs[FASTPAD] = loadball ("Fastpad-Ball", &fastpad_bitmap);
  ballsurfs[SLOWPAD] = loadball ("Slowpad-Ball", &slowpad_bitmap);

  blackout_radius = (real) ballsurfs[BLACKOUT]->w / (real) 2.0;

  padsurfs[REDPAD] = loadpad ("Pad-1", &redpad_bitmap);
  padsurfs[GREENPAD] = loadpad ("Pad-2", &greenpad_bitmap);
  padsurfs[BLUEPAD] = loadpad ("Pad-3", &bluepad_bitmap);
  padsurfs[YELLOWPAD] = loadpad ("Pad-4", &yellowpad_bitmap);
  padsurfs[PURPLEPAD] = loadpad ("Pad-5", &purplepad_bitmap);
  padsurfs[CYANPAD] = loadpad ("Pad-6", &cyanpad_bitmap);

  bgsurfs[BG1] = loadsurf ("Background", &bg1_bitmap);
}

static void
free_surfs (void)
{
  int i;

  for (i = 0; i < NBALLSURFS; ++i)
    {
      SDL_FreeSurface (ballsurfs[i]);
      ballsurfs[i] = NULL;
    }

  for (i = 0; i < NPADSURFS; ++i)
    {
      SDL_FreeSurface (padsurfs[i]);
      padsurfs[i] = NULL;
    }

  for (i = 0; i < NBGSURFS; ++i)
    {
      SDL_FreeSurface (bgsurfs[i]);
      bgsurfs[i] = NULL;
    }
}

static font *
find_unacquired_font (font * list, int n)
{
  for (; n > 0; --n)
    {
      if (!list[0].acquired)
	return &list[0];
      ++list;
    }
  return NULL;
}

font *
acquire_medium_font (SDL_Color c)
{
  font *f = find_unacquired_font (medium_fonts, MAX_MEDIUM_FONTS);
  if (f == NULL)
    fatal ("ran out of fonts");

  f->acquired = true;
  set_font_color (f, c);
  return f;
}

void
medium_text_size (const char *text, SDL_Rect * r)
{
  text_size (medium_fonts, text, r);
}

void
release_font (font * f)
{
  f->acquired = false;
}

SDL_Surface *
getballsurf (int n)
{
  if (n >= 0 && n < NBALLSURFS)
    return ballsurfs[n];
  return NULL;
}

SDL_Surface *
getpadsurf (int n)
{
  if (n >= 0 && n < NPADSURFS)
    return padsurfs[n];
  return NULL;
}

SDL_Surface *
getbgsurf (int n)
{
  if (n >= 0 && n < NBGSURFS)
    return bgsurfs[n];
  return NULL;
}

SDL_Surface *
displaysurf (void)
{
  return display;
}

static void
init_joystick(void)
{
  int i, n = SDL_NumJoysticks();

  /* We need to open a joystick before we can receive events for it.  */
  for (i= 0; i < n; ++i)
    if ((joystick = SDL_JoystickOpen(i)))
      break;

  if (joystick)
    SDL_JoystickEventState(SDL_ENABLE);
}

void
initialize (void)
{
  if (isinit)
    return;

  if (CONFIGFILE)
    read_config (&config, CONFIGFILE);

  set_score_file_name ();

  if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1)
    fatal ("initializing SDL: %s", SDL_GetError ());
  if (atexit (SDL_Quit))
    {
      SDL_Quit ();
      fatal ("registering exit procedure");
    }

  init_joystick();
  create_display ();

#ifndef DEBUG_BUILD
  srand ((unsigned int) time (NULL));
#endif

  init_color_table ();
  init_surfs ();
  init_fonts ();
  init_gfxhelp ();
  init_scoremodes ();
  init_gamemode ();

  isinit = true;
}

void
cleanup (void)
{
  if (!isinit)
    return;
  isinit = false;

  save_scores ();
  free_fonts ();
  free_surfs ();
  symtab_free (&color_table);

  xfree (score_file);
  score_file = NULL;

  symtab_free (&config);

  if (joystick)
    SDL_JoystickClose(joystick);
}

/*---------------------------------------------------------------------------*/
/* get_color() and friends. */

/* Converts a hex digit [0-9a-fA-F] to its numeric value 0-15. */
static Uint8
xctoi (char xc)
{
  if (isdigit (xc))
    return xc - '0';
  return 10 + tolower (xc) - 'a';
}

/* Converts a 6-digit hex number into a RGB triplet. */
static bool
parse_color_triple (const char *triple, SDL_Color * color)
{
  int i;

  /* Sanity checks. */
  for (i = 0; i < 6; ++i)
    if (!isxdigit (triple[i]))
      return false;
  if (triple[6])
    return false;

  color->r = xctoi (triple[0]) * 16 + xctoi (triple[1]);
  color->g = xctoi (triple[2]) * 16 + xctoi (triple[3]);
  color->b = xctoi (triple[4]) * 16 + xctoi (triple[5]);

  return true;
}

/*
 * Converts a named color or 6-digit hex number preceded by '#'
 * into a RGB triplet.  
 */
static bool
parse_color_string (const char *str, SDL_Color * color)
{
  const char *builtin;

  if (str[0] == '#')
    return parse_color_triple (str + 1, color);

  builtin = symtab_find (color_table, str);
  if (builtin)
    return parse_color_triple (builtin, color);

  return false;
}

bool
get_color (const char *name, SDL_Color * c)
{
  SDL_Color color;
  const char *confcolor;

  confcolor = get_config (name);
  if (!confcolor || !parse_color_string (confcolor, &color))
    return false;

  *c = color;
  return true;
}
