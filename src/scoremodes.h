/* Copyright (C) Daniel Hipschman, March 2005 */
/* $Id: scoremodes.h,v 1.4 2006/06/15 23:20:45 dsh Exp $ */
#ifndef SCOREMODES_H
#define SCOREMODES_H

#include <SDL/SDL.h>
#include "defines.h"

#define NHIGHSCORES 10
#define HIGHSCORER_NAMELEN 31

void init_scoremodes (void);
void save_scores (void);

bool is_highscore (unsigned long score);
void enter_setscoremode (SDL_Surface * bg, unsigned long score);
void enter_showscoresmode (SDL_Surface * bg);

#endif /* header guard */
