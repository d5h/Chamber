/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: surfs.h,v 1.7 2006/06/15 23:20:45 dsh Exp $ */

#ifndef SURFS_H
#define SURFS_H

#include <SDL/SDL.h>
#include "bitmaplist.h"
#include "font.h"

/* Display width and height. */
int dispw (void);
int disph (void);

/* If you happen to need a rectangle representing the whole display, */
/* this function conveniently returns a pointer to such a surface. */
/* It is static, and owned by us, so don't attempt to free it or anything. */
const SDL_Rect *display_rect (void);

/* Get the display surface. */
SDL_Surface *displaysurf (void);

/* Get a surface for a ball, paddle or background. */
/* Pass an index.  The balls and paddles are synchronized so that */
/* getballsurf(n) will be the same color as getpadsurf(n) */
/* if there is a color corresponding to that ball. */
SDL_Surface *getballsurf (int);
SDL_Surface *getpadsurf (int);
SDL_Surface *getbgsurf (int);

/*
 * Need a font?  Feel free to grab one, but please give it back
 * when you are done because we only keep so many lying around.
 */
font *acquire_medium_font (SDL_Color);
void medium_text_size (const char *text, SDL_Rect *);
void release_font (font *);

extern real blackout_radius;

#endif /* header guard */
