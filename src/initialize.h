/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: initialize.h,v 1.7 2006/06/15 23:20:45 dsh Exp $ */

#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <SDL/SDL.h>
#include "symtab.h"

/*
 * Look something up in the configuration file.
 * If it's not there, return NULL.
 */
const char *get_config (const char *name);
/*
 * Looks up name in the configuration file.  If it is there, and is a
 * valid color, then returns true and stores the color in c.
 * Otherwise returns false and c will not be touched. 
 */
bool get_color (const char *name, SDL_Color * c);
/*
 * Looks up name in the configuration file.  If it is there, and is a
 * valid integer, then returns true and stores the value in i.
 * Otherwise returns false and leaves i untouched.
 */
bool get_int (const char *name, int *i);

/* Name of score file. */
const char *score_file_name (void);

/* Loads all images and initializes dynamically linked libraries, etc. */
void initialize (void);
/* Does the opposite of initialize (). */
void cleanup (void);

#endif /* header guard */
