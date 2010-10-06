/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: save_surf.h,v 1.2 2005/04/25 22:49:20 sirdan Exp $ */

#ifndef BMP2C_H
#define BMP2C_H

#include <SDL/SDL.h>

void save_surf_pixels (FILE * fp, SDL_Surface * s, const char *name);
void save_surf_info (FILE * fp, SDL_Surface * s,
		     const char *colors_name, const char *pixels_name);

#endif /* BMP2C_H */
