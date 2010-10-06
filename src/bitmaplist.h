/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: bitmaplist.h,v 1.7 2006/06/15 23:20:45 dsh Exp $ */

#ifndef BITMAPLIST_H
#define BITMAPLIST_H

#include "bitmap.h"

#define DECLARE_BITMAP(name, ID, val)	\
    enum { ID = (val) };		\
    extern bitmap name

DECLARE_BITMAP (redball_bitmap, REDBALL, 0);
DECLARE_BITMAP (greenball_bitmap, GREENBALL, 1);
DECLARE_BITMAP (blueball_bitmap, BLUEBALL, 2);
DECLARE_BITMAP (yellowball_bitmap, YELLOWBALL, 3);
DECLARE_BITMAP (purpleball_bitmap, PURPLEBALL, 4);
DECLARE_BITMAP (cyanball_bitmap, CYANBALL, 5);
#define SPECIALS	6	/* Start of specials. */
DECLARE_BITMAP (blackout_bitmap, BLACKOUT, 6);
DECLARE_BITMAP (fastpad_bitmap, FASTPAD, 7);
DECLARE_BITMAP (slowpad_bitmap, SLOWPAD, 8);
#define NBALLSURFS	9

DECLARE_BITMAP (redpad_bitmap, REDPAD, 0);
DECLARE_BITMAP (greenpad_bitmap, GREENPAD, 1);
DECLARE_BITMAP (bluepad_bitmap, BLUEPAD, 2);
DECLARE_BITMAP (yellowpad_bitmap, YELLOWPAD, 3);
DECLARE_BITMAP (purplepad_bitmap, PURPLEPAD, 4);
DECLARE_BITMAP (cyanpad_bitmap, CYANPAD, 5);
#define NPADSURFS	6

DECLARE_BITMAP (bg1_bitmap, BG1, 0);
#define NBGSURFS	1

#endif /* header guard */
