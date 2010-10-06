/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: bitmap.h,v 1.4 2006/06/15 23:20:45 dsh Exp $ */

#ifndef BITMAP_H
#define BITMAP_H

#include <SDL/SDL.h>
#if (HAVE_SDL_IMAGE)
#  include <SDL/SDL_image.h>
#  define load_image IMG_Load
#else
#  define load_image SDL_LoadBMP
#endif

typedef struct
{
  int width, height;
  int depth, pitch;
  int ncolors;			/* for palette */
  int has_key;			/* color key transparency? */
  Uint32 key;
  SDL_Color *colors;
  void *pixels;
} bitmap;

SDL_Surface *makesurf (bitmap *);

#endif /* header guard */
