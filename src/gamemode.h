/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: gamemode.h,v 1.10 2006/06/15 23:20:45 dsh Exp $ */

#ifndef GAMEMODE_H
#define GAMEMODE_H

#include <SDL/SDL.h>
#include "defines.h"

typedef struct ball ball;

/*
 * A ball can be one of these things.
 * SPCL_TRIGGER defines the trigger ball. SPCL_SPECIAL implies
 * that the ball has a special ability and responds to events.
 * SPCL_NONE covers the case of a standard ball that is not
 * currently the trigger.
 */
typedef enum
{
  SPCL_NONE, SPCL_TRIGGER, SPCL_SPECIAL
} special;

/*
 * Events that special balls can respond to.
 */
typedef struct
{
  void (*activate) (ball *);
  void (*pad_hit) (ball *);
  void (*move) (ball *);
} event_special;

/*
 * The fundamental ball structure.
 */
struct ball
{
  point pos;			/* coordinate of center */
  vector vel;
  real mass, radius;
  special spcl;
  /* Index of the ball in the array of balls; */
  /* and an indicator of the last object hit by the ball. */
  int index, last_hit;
  SDL_Surface *img;
  SDL_Rect patch;
  event_special *event;
  unsigned int lifetime;	/* Special balls may need this. */
  unsigned int active:1;
  unsigned int recalc:1;
  unsigned int patched:1;
  unsigned int remove:1;
};

/* Measured in FPS. */
#define BALL_SPEED(b) ((real) sqrt (DOTVECS ((b)->vel, (b)->vel)))

/*
 * STDBALLS are the colored ones that may become the trigger. The MIN
 * and MAX refer to what limit has to be reached before no more will
 * be removed or added. Obviously, the MAX is limited by the number
 * of distinct surfaces we have. MAX_NBALLS simply refers to the
 * maximum number of balls, standard or special, can be in play at once.
 */
#define MIN_STDBALLS	3
#define MAX_STDBALLS	6
#define MAX_NBALLS	10

/* LASTHIT_NOTHING should be different from all other possible LASTHIT values */
/* which include those shown below, and implicitly includes indices of balls. */
/* Usually, ball indices range from (0) to (MAX_NBALLS - 1), but the paddle */
/* uses (-1) as the index of each of its ends, so DON'T use (-1) !!! */
#define LASTHIT_NOTHING	(-2)
#define LASTHIT_PAD	(MAX_NBALLS)
#define LASTHIT_LWALL	(MAX_NBALLS + 1)
#define LASTHIT_RWALL	(MAX_NBALLS + 2)
#define LASTHIT_TWALL	(MAX_NBALLS + 3)
#define LASTHIT_BWALL	(MAX_NBALLS + 4)

/*
 * Paddle structure.
 */
typedef struct
{
  point pos;			/* center of paddle */
  vector vel;
  real mass, thickness, length;	/* total length */
  real rectlen;			/* length of rectangular part */
  SDL_Surface *img;
  SDL_Rect patch;
  int last_hit;			/* Similar to the ball's, but a little different */
  unsigned int recalc:1;
  unsigned int patched:1;
  ball ends[2];
} paddle;

/*
 * Different types of collisions that can occur:
 *	BB = ball-ball
 *	BP = ball-paddle
 *	BW = ball-wall
 *	PW = paddle wall
 */
typedef enum
{
  BB_COLLISION, BP_COLLISION, BW_COLLISION, PW_COLLISION
} collision_type;

/*
 * A structure holding information about some collision that will occur.
 */
typedef struct
{
  real time;			/* When will it occur? */
  collision_type type;		/* What's colliding? */
  union
  {
    struct
    {
      ball *b1, *b2;
    } bb;
    struct
    {
      ball *b;
      int pad_part;
    } bp;
    struct
    {
      ball *b;
      int wall;
    } bw;
    struct
    {
      int wall;
    } pw;
  } extra;
} collision_info;

/*
 * Used for paddle collisions. What part of the paddle was hit?
 */
#define PAD_PART_LEFT	0
#define PAD_PART_RIGHT	1
#define PAD_PART_CENTER	2

/*
 * Wall collisions.
 */
#define WALL_LEFT	1
#define WALL_RIGHT	2
#define WALL_TOP	3
#define WALL_BOTTOM	4

/* One-time initialization of the "game mode" state. */
void init_gamemode (void);
void enter_gamemode (void);

#endif /* header guard */
