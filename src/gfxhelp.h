/* Copyright (C) Daniel Hipschman, March 2005 */
/* $Id: gfxhelp.h,v 1.3 2006/06/15 23:20:45 dsh Exp $ */
#ifndef GFXHELP_H
#define GFXHELP_H

#include <SDL/SDL.h>

void init_gfxhelp (void);
/* If dst is partially off screen, update the two rectangles so that */
/* no part of a blit of size src to dst will be off screen. */
/* If dst.w == 0 after the call, then dst is completely off screen. */
void adjust_draw_rects (SDL_Rect * src, SDL_Rect * dst);
/* Blit bg onto prect. The actual rectangle patched is stored in arect */
/* which may be NULL. */
void apply_patch (SDL_Surface * bg, SDL_Rect * prect, SDL_Rect * arect);
/* Used to update the display for a moving object. As one rectangle */
/* we pass the previous position of the object, or the patch. As the */
/* other rectangle, we pass the present position. */
void update_dirty (SDL_Rect * r1, SDL_Rect * r2);

#endif /* header guard */
