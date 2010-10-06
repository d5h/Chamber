/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: gamemode.c,v 1.19 2006/06/15 23:20:45 dsh Exp $ */

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "gamemode.h"
#include "gfxhelp.h"
#include "initialize.h"
#include "scoremodes.h"
#include "surfs.h"

static struct
{
  real left, right, top, bottom;
} wall;

#define INIT_BALL_MASS	((real) 1.0)

#define FPS	((real) 75.0)

#define SECONDS2FRAMES(s)	(rround ((real) (s) * FPS))
#define FRAMES2SECONDS(f)	((real) (f) / FPS)
#define PPS2PPF(p)	((real) (p) / FPS)
#define PPF2PPS(f)	((real) (f) * FPS)

/* Things ending in _PPS are measured in pixels per second. */
#define INIT_BALL_SPEED_MIN_PPS	((real) 100.0)
#define INIT_BALL_SPEED_MAX_PPS	((real) 150.0)

static real pad_speed;
#define INIT_PAD_SPEED_PPS	((real) 250.0)

static ball balls[MAX_NBALLS];
static paddle pad;

/* Index of trigger-ball */
static int trigidx;

static struct
{
  bool left, right;
} keydown;

#define SCORE_MAX		999999UL
#define SCORE_TEXT_LIMIT	16	/* Holds up to "Score: <SCORE_MAX>" */
#define SCORE_XPLACEMENT        8
#define SCORE_YPLACEMENT        8
#define SCORE_PER_NEW_LIFE	50000

/*
 * The score, and various things related to it.
 */
static struct
{
  unsigned long value, next_life, next_level;
  char text[SCORE_TEXT_LIMIT + 1];
  SDL_Rect placement, patch;
  unsigned int redraw:1;
} score;

#define INIT_NLIVES		3
#define MAX_LIVES		99
#define LIVES_TEXT_LIMIT	16	/* Holds up to "Lives: <MAX_LIVES>" */
#define LIVES_XSPACING		24
#define LIVES_YPLACEMENT	SCORE_YPLACEMENT

/*
 * The number of lives remaining to the player.
 */
static struct
{
  unsigned long number;
  char text[LIVES_TEXT_LIMIT + 1];
  SDL_Rect placement, patch;
  int xplacement;
  unsigned int redraw:1;
} lives;

/*
 * Score needed to increment level.
 */
#define SCORE_PER_LEVEL_UP		2500
/*
 * At level 0, this is the minimum (maximum) total speed of all
 * standard (non-special) balls before another ball is added (removed).
 */
#define INIT_TOO_SLOW_PPS 	((real) 500.0)
#define INIT_TOO_FAST_PPS 	((real) 1000.0)
/*
 * The amount that the above are each incremented per level.
 */
#define SPEED_CONTROL_INC_PPS	((real) 50.0)

static struct
{
  unsigned long value;
  real too_slow, too_fast;
} level;

/*
 * Incremented each time we update the game logic.  Use as a timer.
 */
static unsigned long updates;
/*
 * Every once in a while, do a speed check.  When this is done, we
 * see how fast balls are flying around, in general.  If they're
 * going too slow, feel free to throw the player a challenge.  If
 * they're going pretty fast, maybe cut the user some slack.
 */
static unsigned long last_speed_check;
/*
 * Seconds between speed checks.
 */
#define SPEED_CHECK_DELAY_SECONDS	30

static bool paused;

static SDL_Surface *bgsurf;	/* Background.  A pretty picture. */
static SDL_Surface *dsplsurf;	/* Screen.  Where we draw things. */
static bool redraw;		/* Screen need refreshing. */

static font *gfont = NULL;	/* General font */
static SDL_Color default_gfont_color = { 0xFF, 0xFF, 0xFF, 0x00 };

/*
 * Hold info about collisions in the future.  We calculate them in
 * advance and wait for them to happen.  These hold info about all
 * possible collisions that could occur.
 */
static collision_info bp_collisions[MAX_NBALLS];
static collision_info bw_collisions[MAX_NBALLS];
static collision_info pw_collision;
static collision_info bb_collisions[MAX_NBALLS * (MAX_NBALLS - 1) / 2];


/*
 * If nonzero, represents the number of updates remaining that the
 * balls will be "blacked out".
 */
static unsigned int blackout;
/*
 * How many seconds do we stay "blacked out"?
 */
#define BLACKOUT_TIME_LIMIT_SECONDS	7
/*
 * How many seconds the blackout ball will stick around.
 */
#define BLACKOUT_LIFETIME_SECONDS	20

/*
 * If positive, represents the number of updates remaining that the
 * paddle will get a speed bonus.  If negative, the number of updates
 * that the paddle will get a speed penalty.
 */
static int fastpad;
#define FASTPAD_TIME_LIMIT_SECONDS	5
#define SLOWPAD_TIME_LIMIT_SECONDS	10
#define FASTPAD_LIFETIME_SECONDS	20
#define SLOWPAD_LIFETIME_SECONDS	20

static void activate_blackout (ball *);
static void activate_fastpad (ball *);
static void activate_slowpad (ball *);

static void pad_hit_blackout (ball *);
static void pad_hit_fastpad (ball *);
static void pad_hit_slowpad (ball *);

static void move_timeout (ball *);

/*
 * Definitions of special balls.
 */
static event_special blackout_events = {
  /*
   * When the ball hits us, makes all other balls (at least the
   * ones we _can_make_) look just like us.  Hence, the player
   * will have a hard time remembering which ball is the trigger.
   */
  activate_blackout,
  pad_hit_blackout,
  move_timeout
};
static event_special fastpad_events = {
  /*
   * Speed the paddle up for a time when it hits us.
   */
  activate_fastpad,
  pad_hit_fastpad,
  move_timeout
};
static event_special slowpad_events = {
  /*
   * Slow the paddle down for a bit when it hits us.
   */
  activate_slowpad,
  pad_hit_slowpad,
  move_timeout
};

#define NSPECIALS 3

/*
 * We keep these together so we can randomly choose one quickly.
 */
static event_special *specials[NSPECIALS];

static unsigned long last_add_special;
/*
 * How long do we wait before we maybe add another special ball.
 */
#define ADD_SPECIAL_DELAY_SECONDS	15

static real now;		/* Used only for debugging. */

/*
 * Prints a string to debugging file.
 */
#define debug	printf

/*
 * Translate last_hit field into a string and debug-print it.
 */
static void
debug_last_hit (int lh)
{
  switch (lh)
    {
    case LASTHIT_NOTHING:
      break;
    case LASTHIT_PAD:
      debug ("P");
      break;
    case LASTHIT_LWALL:
      debug ("L");
      break;
    case LASTHIT_RWALL:
      debug ("R");
      break;
    case LASTHIT_TWALL:
      debug ("T");
      break;
    case LASTHIT_BWALL:
      debug ("B");
      break;
    default:
      debug ("%d", lh);
    }
}

/*
 * Print short info about ball 'b' to help with debugging.
 */
static void
debug_ball_short (ball * b)
{
  char spcl;

  switch (b->spcl)
    {
    case SPCL_NONE:
      spcl = 'N';
      break;
    case SPCL_TRIGGER:
      spcl = 'T';
      break;
    case SPCL_SPECIAL:
      spcl = 'S';
      break;
    }

  debug ("B %d,%g,%g,%g,%g,%c,",
	 b->index, b->pos.x, b->pos.y, b->vel.dx, b->vel.dy, spcl);

  debug_last_hit (b->last_hit);

  debug (",%d,%s%s\n",
	 b->lifetime, (b->recalc ? "c" : ""), (b->remove ? "r" : ""));
}

static void
debug_pad_short (void)
{
  debug ("P %g,%g,", pad.pos.x, pad.vel.dx);

  debug_last_hit (pad.last_hit);

  debug (",%s\n", (pad.recalc ? "c" : ""));
}

/*
 * Print a summary of the game at this exact state.
 */
static void
debug_stats (void)
{
  int i;

  debug ("S\n");

  debug_pad_short ();

  for (i = 0; i < MAX_NBALLS; ++i)
    if (balls[i].active)
      debug_ball_short (&balls[i]);

  debug ("\n");
}

/*
 * Set the level number.
 */
static void
set_level (unsigned long value)
{
  if (value > 0)
    {
      long diff = value - level.value;
      level.too_slow += (real) diff *PPS2PPF (SPEED_CONTROL_INC_PPS);
      level.too_fast += (real) diff *PPS2PPF (SPEED_CONTROL_INC_PPS);
    }
  else
    {
      level.too_slow = PPS2PPF (INIT_TOO_SLOW_PPS);
      level.too_fast = PPS2PPF (INIT_TOO_FAST_PPS);
    }
  level.value = value;
}

/*
 * Set the number of lives.
 */
static void
set_lives (unsigned long value)
{
  if (value > MAX_LIVES)
    value = MAX_LIVES;
  if (value != lives.number)
    {
      lives.number = value;
      sprintf (lives.text, "Lives: %lu", value);
      lives.placement.x = lives.xplacement;
      lives.placement.y = LIVES_YPLACEMENT;
      medium_text_size (lives.text, &lives.placement);
    }
  lives.redraw = true;
}

/*
 * Set the player's score.
 */
static void
set_score (unsigned long value)
{
  if (value > SCORE_MAX)
    value = SCORE_MAX;
  if (value != score.value)
    {
      int newlives = 0, level_gain = 0;

      while (value > score.next_life)
	{
	  ++newlives;
	  score.next_life += SCORE_PER_NEW_LIFE;
	}
      if (newlives)
	set_lives (lives.number + newlives);

      while (value > score.next_level)
	{
	  ++level_gain;
	  score.next_level += SCORE_PER_LEVEL_UP;
	}
      if (level_gain)
	set_level (level.value + level_gain);

      score.value = value;
      sprintf (score.text, "Score: %lu", value);
      score.placement.x = SCORE_XPLACEMENT;
      score.placement.y = SCORE_YPLACEMENT;
      medium_text_size (score.text, &score.placement);
    }
  score.redraw = true;
}

/*
 * Increment the player's score by a certain amount.
 */
static void
inc_score (unsigned long amount)
{
  set_score (score.value + amount);
}

/*
 * One time initialization. Gather resources, etc.
 */
void
init_gamemode (void)
{
  int i;

  wall.left = (real) 0.0;
  wall.top = (real) 0.0;
  wall.right = (real) dispw ();
  wall.bottom = (real) disph ();

  /* Initialize balls. */
  for (i = 0; i < MAX_NBALLS; ++i)
    {
      ball *b = &balls[i];
      if (i < MAX_STDBALLS)
	{
	  b->img = getballsurf (i);	/* should not fail */
	  b->radius = (real) b->img->w / (real) 2.0;
	  if (b->radius > (wall.right - wall.left - (real) 2.0) / (real) 2.0
	      || b->radius > (wall.bottom - wall.top) / (real) 2.0)
	    fatal ("because ball #%d is too big", i);
	  b->patch.w = b->img->w;
	  b->patch.h = b->img->h;
	}
      else
	{
	  b->img = NULL;
	}
      b->index = i;

      bp_collisions[i].type = BP_COLLISION;
      bw_collisions[i].type = BW_COLLISION;

      b->event = NULL;
    }

  for (i = 0; i < MAX_NBALLS * (MAX_NBALLS - 1) / 2; ++i)
    bb_collisions[i].type = BB_COLLISION;

  /* Initialize paddle. */
  pw_collision.type = PW_COLLISION;
  pad.img = getpadsurf (0);	/* any color will do for now */
  pad.thickness = (real) pad.img->h;
  pad.length = (real) pad.img->w;
  if (pad.length > wall.right - wall.left)
    fatal ("because the paddle won't fit between the left and right walls");
  pad.rectlen = pad.length - pad.thickness;
  if (pad.rectlen <= 0.0)
    fatal ("because the paddle is thicker than it is long");
  pad.patch.w = pad.img->w;
  pad.patch.h = pad.img->h;
  pad.ends[0].radius = pad.ends[1].radius = pad.thickness / (real) 2.0;
  pad.ends[0].spcl = pad.ends[1].spcl = SPCL_NONE;
  pad.ends[0].index = pad.ends[1].index = -1;
  pad.ends[0].active = pad.ends[1].active = true;
  pad.ends[0].recalc = pad.ends[1].recalc = true;
  pad.ends[0].img = pad.ends[1].img = NULL;

  dsplsurf = displaysurf ();

  /* Find how much we want to space over the "Lives" label. */
  /* This is not entirely accurate, because SCORE_MAX may not consist */
  /* of the widest possible characters. */
  set_score (SCORE_MAX);
  lives.xplacement = score.placement.x + score.placement.w + LIVES_XSPACING;

  specials[BLACKOUT - SPECIALS] = &blackout_events;
  specials[FASTPAD - SPECIALS] = &fastpad_events;
  specials[SLOWPAD - SPECIALS] = &slowpad_events;
}

/*
 * Determine if two balls are overlapping.
 */
static bool
overlapping (ball * b1, ball * b2)
{
  vector d;
  real r2, q;

  SUBPOINTS (b1->pos, b2->pos, d);
  r2 = DOTVECS (d, d);		/* distance between them, squared */

  q = b1->radius + b2->radius;
  return r2 <= q * q;
}

static void
reset_ball (int idx)
{
  vector v;
  point center;
  real d, s;
  bool overlap;

  ball *b = &balls[idx];

  center.x = (wall.left + wall.right) / (real) 2.0;
  center.y = (wall.top + wall.bottom) / (real) 2.0;

  b->pos.y = wall.top - b->radius;
  /*
   * Give the ball some x coordinate, but make sure it is not
   * overlapping some other ball.
   */
  do
    {
      int i;
      b->pos.x = rrand (b->radius + (real) 1.0,
			wall.right - b->radius - (real) 1.0);

      overlap = false;
      for (i = 0; i < MAX_NBALLS; ++i)
	if (i != idx && balls[i].active)
	  if ((overlap = overlapping (&balls[idx], &balls[i])) != 0)
	    break;

      /*
       * FIXME: Manually fix ball so it doesn't overlap any others.
       *        Don't rely on random number generated to find an
       *        open spot for us. Potential to hang in some
       *        pathological case?
       */
    }
  while (overlap);

  /* Make the ball's velocity point toward the center of the display. */
  SUBPOINTS (center, b->pos, v);
  d = (real) sqrt (DOTVECS (v, v));
  s = rrand (PPS2PPF (INIT_BALL_SPEED_MIN_PPS),
	     PPS2PPF (INIT_BALL_SPEED_MAX_PPS)) / d;
  SCALEVEC (v, s);
  b->vel = v;

  b->mass = INIT_BALL_MASS;
  b->spcl = SPCL_NONE;
  b->last_hit = LASTHIT_TWALL;
  b->recalc = true;
  b->remove = false;

  memset (&b->patch, 0, sizeof (SDL_Rect));
}

static void debug_setup (const char *filename);

/*
 * Call like when the user dies and we need to reset all the
 * game parameters. Basically, this starts a new life. Anything
 * that retains it's value between lives, like score, should not
 * be touched here.
 */
static void
new_life (void)
{
  int i;

  if (debugging)
    debug ("N %lu,%lu\n", lives.number, score.value);

  for (i = 0; i < MAX_NBALLS; ++i)
    {
      balls[i].active = false;
      balls[i].patched = true;
    }
  for (i = 0; i < MIN_STDBALLS; ++i)
    {
      reset_ball (i);
      balls[i].active = true;
    }
  trigidx = rand () % MIN_STDBALLS;
  balls[trigidx].spcl = SPCL_TRIGGER;

  pad.img = getpadsurf (trigidx);
  pad.pos.x = (wall.left + wall.right) / (real) 2.0;
  pad.pos.y = wall.bottom - pad.thickness / (real) 2.0;
  pad.vel.dx = pad.vel.dy = (real) 0.0;
  pad.mass = (real) 0.0;	/* infinite mass */
  pad.recalc = pad.patched = true;
  pad.last_hit = LASTHIT_NOTHING;
  pad.ends[0].pos.x = pad.pos.x - pad.rectlen / (real) 2.0;
  pad.ends[0].pos.y = pad.pos.y;
  pad.ends[1].pos.x = pad.pos.x + pad.rectlen / (real) 2.0;
  pad.ends[1].pos.y = pad.pos.y;
  pad.ends[0].vel = pad.ends[1].vel = pad.vel;
  pad.ends[0].mass = pad.ends[1].mass = pad.mass;
  memset (&pad.patch, 0, sizeof (SDL_Rect));

  bgsurf = getbgsurf (rand () % NBGSURFS);
  lives.redraw = score.redraw = true;
  redraw = true;
  paused = false;

  updates = SECONDS2FRAMES (0);
  last_speed_check = SECONDS2FRAMES (0);
  last_add_special = SECONDS2FRAMES (0);

  blackout = SECONDS2FRAMES (0);
  fastpad = SECONDS2FRAMES (0);
  pad_speed = PPS2PPF (INIT_PAD_SPEED_PPS);

  memset (&keydown, 0, sizeof (keydown));

  if (debugfile)
    debug_setup (debugfile);
}

/*
 * Call whenever we are setting up for a new game. Reset score
 * and number of lives and things like that.
 */
static void
reset_gamemode (void)
{
  set_score (0);
  set_lives (INIT_NLIVES - 1);
  score.next_life = SCORE_PER_NEW_LIFE;
  set_level (0);
  score.next_level = SCORE_PER_LEVEL_UP;

  new_life ();
}

/*
 * Set the speed of the paddle. Positive means go right.
 * Negative, go left. Zero, go nowhere. The bigger the
 * speed, the faster it goes.
 */
static void
set_pad_vel (real v)
{
  if (pad.vel.dx != v)
    {
      pad.recalc = true;
      pad.last_hit = LASTHIT_NOTHING;
    }
  pad.ends[1].vel.dx = pad.ends[0].vel.dx = pad.vel.dx = v;

  if (debugging && pad.recalc)
    debug_pad_short ();
}

#define CQ_CONTINUE	0
#define CQ_SHUTDOWN	1
#define CQ_ERROR 	2

static void
maybe_start_moving_left(void)
{
  if (pad.vel.dx == (real) 0.0
      && pad.pos.x > wall.left + pad.length / (real) 2.0)
    set_pad_vel (-pad_speed);
}

static void
maybe_start_moving_right(void)
{
  if (pad.vel.dx == (real) 0.0
      && pad.pos.x < wall.right - pad.length / (real) 2.0)
    set_pad_vel (+pad_speed);
}

static void
stop_moving(void)
{
  set_pad_vel (keydown.left ? -pad_speed :
	       (keydown.right ? +pad_speed : (real) 0.0));
}

/*
 * Get events from operating system (key presses, etc.) and process them.
 */
static int
clear_gamemode_queue (void)
{
  static SDL_Event event;

  while (SDL_PollEvent (&event))
    switch (event.type)
      {
      case SDL_KEYDOWN:
	switch (event.key.keysym.sym)
	  {
	  case SDLK_LEFT:
	    keydown.left = true;
	    maybe_start_moving_left();
	    break;
	  case SDLK_RIGHT:
	    keydown.right = true;
	    maybe_start_moving_right();
	    break;
	  case SDLK_p:
	    paused = !paused;
	    break;
	  case SDLK_SPACE:
	    if (debugging)
	      {
		debug_stats ();
		paused = true;
	      }
	    break;
	  case SDLK_ESCAPE:
	    /* Or exit to main menu, etc... */
	    return CQ_SHUTDOWN;
	  default:
	    /* Get rid of annoying warnings from compiler. */
	    break;
	  }

	break;			/* KEYDOWN case */

      case SDL_KEYUP:
	if (event.key.keysym.sym == SDLK_LEFT)
	  keydown.left = false;
	else if (event.key.keysym.sym == SDLK_RIGHT)
	  keydown.right = false;
	else
	  break;
	stop_moving();
	break;

      case SDL_JOYAXISMOTION:
	/* Respond to X axis events.  */
	if (event.jaxis.axis == 0)
	  {
	    Sint16 dir = event.jaxis.value;
	    if (0 < dir)
	      maybe_start_moving_right();
	    else if (dir < 0)
	      maybe_start_moving_left();
	    else
	      stop_moving();
	  }
	break;

      case SDL_ACTIVEEVENT:
	/* Pause when we lose focus, as a courtesy to the user. */
	if (event.active.gain == 0
	    && (event.active.state == SDL_APPINPUTFOCUS
		|| event.active.state == SDL_APPACTIVE))
	  paused = true;
	break;

      case SDL_QUIT:
	return CQ_SHUTDOWN;
      }

  return CQ_CONTINUE;
}

static void
patch_gamemode_display (void)
{
  static SDL_Rect dst;
  int i;

  for (i = 0; i < MAX_NBALLS; ++i)
    if (!balls[i].patched)
      {
	apply_patch (bgsurf, &balls[i].patch, &dst);
	balls[i].patched = true;
	/* If the ball won't be redrawn, then update it now, */
	/* otherwise, let the redraw function do it for efficiency. */
	if (!balls[i].active)
	  SDL_UpdateRects (dsplsurf, 1, &dst);
      }

  if (!pad.patched)
    {
      apply_patch (bgsurf, &pad.patch, NULL);
      pad.patched = true;
    }

  if (score.redraw)
    apply_patch (bgsurf, &score.patch, NULL);
  if (lives.redraw)
    apply_patch (bgsurf, &lives.patch, NULL);
}

static void
redraw_gamemode_display (void)
{
  static SDL_Rect src, dst;
  int i;

  blit_text (gfont, score.text, dsplsurf, SCORE_XPLACEMENT, SCORE_YPLACEMENT);
  if (score.redraw)
    {
      update_dirty (&score.patch, &score.placement);
      score.patch = score.placement;
      score.redraw = false;
    }

  blit_text (gfont, lives.text, dsplsurf, lives.xplacement, LIVES_YPLACEMENT);
  if (lives.redraw)
    {
      update_dirty (&lives.patch, &lives.placement);
      lives.patch = lives.placement;
      lives.redraw = false;
    }

  for (i = 0; i < MAX_NBALLS; ++i)
    if (balls[i].active)
      {
	src.x = src.y = 0;
	dst.x = rround (balls[i].pos.x - balls[i].radius);
	dst.y = rround (balls[i].pos.y - balls[i].radius);
	src.w = dst.w = balls[i].img->w;
	src.h = dst.h = balls[i].img->h;
	adjust_draw_rects (&src, &dst);
	if (dst.w > 0)
	  {
	    SDL_Surface *img = balls[i].img;
	    if (blackout && balls[i].radius == blackout_radius)
	      img = getballsurf (BLACKOUT);
	    SDL_BlitSurface (img, &src, dsplsurf, &dst);
	    balls[i].patched = false;
	  }
	update_dirty (&balls[i].patch, &dst);
	balls[i].patch = dst;
      }

  src.x = src.y = 0;
  dst.x = rround (pad.pos.x - pad.length / (real) 2.0);
  dst.y = rround (pad.pos.y - pad.thickness / (real) 2.0);
  src.w = dst.w = pad.img->w;
  src.h = dst.h = pad.img->h;
  adjust_draw_rects (&src, &dst);
  if (dst.w > 0)
    {
      SDL_BlitSurface (pad.img, &src, dsplsurf, &dst);
      pad.patched = false;
    }
  update_dirty (&pad.patch, &dst);
  pad.patch = dst;

  redraw = false;
}

/*
 * Redraw everything.
 */
static void
refresh_gamemode_display (void)
{
  apply_patch (bgsurf, NULL, NULL);
  SDL_UpdateRect (dsplsurf, 0, 0, 0, 0);
  redraw_gamemode_display ();
}

static void
debug_time (real t)
{
  debug ("T %g+%g\n", now, t);
}

#define NO_SOLUTION	((real) -1.0)

#define NEITHER_SMALLEST	0
#define FIRST_SMALLEST		1
#define SECOND_SMALLEST		2

static int
non_negative_smallest (real f, real s)
{
  if (f < s)
    {
      if (f >= (real) 0.0)
	return FIRST_SMALLEST;
      if (s >= (real) 0.0)
	return SECOND_SMALLEST;
    }

  if (s >= (real) 0.0)
    return SECOND_SMALLEST;
  if (f >= (real) 0.0)
    return FIRST_SMALLEST;

  return NEITHER_SMALLEST;
}

static void
bb_find_collision (ball * b1, ball * b2, collision_info * info)
{
  real Dx, Dy, Ddx, Ddy, r, twoa, b, c, bb_4ac;

  info->extra.bb.b1 = b1;
  info->extra.bb.b2 = b2;

  if (b1->last_hit == b2->index && b2->last_hit == b1->index)
    {
      info->time = NO_SOLUTION;
      return;
    }

  /* The distance between the balls is given by a quadratic (att+bt+c), */
  /* where (a>=0) and if (a==0) then (b==0).  (Think about it...) */
  /* We are looking for the time interval between the left root */
  /* (when the distance between the balls first vanishes) and */
  /* the vertex (when the balls begin to move away from each other). */
  Dx = b1->pos.x - b2->pos.x;
  Dy = b1->pos.y - b2->pos.y;
  Ddx = b1->vel.dx - b2->vel.dx;
  Ddy = b1->vel.dy - b2->vel.dy;
  r = b1->radius + b2->radius;
  twoa = (real) 2.0 *(Ddx * Ddx + Ddy * Ddy);
  b = (real) 2.0 *(Dx * Ddx + Dy * Ddy);
  c = Dx * Dx + Dy * Dy - r * r;
  bb_4ac = b * b - (real) 2.0 *twoa * c;

  /* Think of yourself standing at the origin while the graph of the */
  /* parabola scrolls by from right to left. */
  if (twoa > (real) 0.0 && bb_4ac >= (real) 0.0)
    {
      real m = -b / twoa;
      real n = (real) sqrt (bb_4ac) / twoa;

      /* We see the left root up ahead... */
      if (m - n >= (real) 0.0)
	info->time = m - n;
      /* Well, we passed the left root, but not yet the vertex. */
      else if (m >= (real) 0.0)
	info->time = (real) 0.0;
      /* Balls are moving away from each other. */
      else
	info->time = NO_SOLUTION;

    }
  else
    {
      /* The distance between the balls is constant. */
      info->time = NO_SOLUTION;
      /* What should we do if the balls are overlapping (c<=0)? */
    }

  if (debugging && info->time != NO_SOLUTION)
    {
      debug_time (info->time);
      debug_ball_short (b1);
      debug_ball_short (b2);
    }
}

static void
bp_find_collision (ball * b, collision_info * info)
{
  real t;

  info->extra.bp.b = b;
  if (b->last_hit == LASTHIT_PAD && pad.last_hit == b->index)
    {
      info->time = NO_SOLUTION;
      return;
    }

  /* Ball is moving "down". Remember that the coordinate axis is flipped. */
  if (b->vel.dy > 0)
    t = ((pad.pos.y - b->pos.y)
	 - (pad.thickness / (real) 2.0 + b->radius)) / b->vel.dy;

  /* Ball is moving "up". */
  else if (b->vel.dy < 0)
    t = ((pad.pos.y - b->pos.y)
	 + (pad.thickness / (real) 2.0 + b->radius)) / b->vel.dy;

  /* Ball is moving exactly horizontally. If it hits the paddle at all */
  /* it will hit it at one of the ends. */
  else
    goto try_ends;

  /* t is the time when the vertical distance between the ball and paddle */
  /* will be minimum. */
  if (t >= (real) 0.0 && (real) fabs ((b->pos.x + t * b->vel.dx)
				      - (pad.pos.x + t * pad.vel.dx))
      <= pad.rectlen / (real) 2.0)
    {
      info->time = t;
      info->extra.bp.pad_part = PAD_PART_CENTER;
      goto done;
    }

try_ends:{
    collision_info left, right;
    bb_find_collision (b, &pad.ends[0], &left);
    bb_find_collision (b, &pad.ends[1], &right);

    switch (non_negative_smallest (left.time, right.time))
      {
      case FIRST_SMALLEST:
	info->time = left.time;
	info->extra.bp.pad_part = PAD_PART_LEFT;
	break;
      case SECOND_SMALLEST:
	info->time = right.time;
	info->extra.bp.pad_part = PAD_PART_RIGHT;
	break;
      default:
	info->time = NO_SOLUTION;
      }
  }

done:
  if (debugging && info->time != NO_SOLUTION)
    {
      debug_time (info->time);
      debug_ball_short (b);
      debug_pad_short ();
    }
}

static void
debug_wall (int wall)
{
  char w;

  switch (wall)
    {
    case WALL_LEFT:
      w = 'L';
      break;
    case WALL_RIGHT:
      w = 'R';
      break;
    case WALL_TOP:
      w = 'T';
      break;
    case WALL_BOTTOM:
      w = 'B';
      break;
    }

  debug ("W %c\n", w);
}

static void
bw_find_collision (ball * b, collision_info * info)
{
  real th = NO_SOLUTION, tv = NO_SOLUTION;

  info->extra.bw.b = b;

  /* See when it hits the left or right wall. */
  if (b->vel.dx < (real) 0.0 && b->last_hit != LASTHIT_LWALL)
    th = (b->radius - b->pos.x) / b->vel.dx;
  else if (b->vel.dx > (real) 0.0 && b->last_hit != LASTHIT_RWALL)
    th = (wall.right - b->radius - b->pos.x) / b->vel.dx;

  /* See when it hits the top or bottom wall. */
  if (b->vel.dy < (real) 0.0 && b->last_hit != LASTHIT_TWALL)
    {
      /* If the ball is being removed, it does not "bounce" off the top */
      /* wall. */
      real r = (b->remove) ? -b->radius : b->radius;
      tv = (r - b->pos.y) / b->vel.dy;
      if (tv < (real) 0.0)
	tv = (real) 0.0;	/* For similar reasons as below. */
    }
  else if (b->vel.dy > (real) 0.0 && b->last_hit != LASTHIT_BWALL)
    {
      /* If the ball is the trigger, it does not "bounce" off the bottom */
      /* wall, it falls completely below it. */
      real r = (b->spcl == SPCL_TRIGGER) ? -b->radius : b->radius;
      tv = (wall.bottom - r - b->pos.y) / b->vel.dy;
      /* There is a bug if the trigger is half way below the bottom wall,  */
      /* and another ball hits it, thereby becoming the trigger, then the */
      /* old trigger will continue through the bottom wall because tv < 0. */
      /* This fixes that bug. */
      if (tv < (real) 0.0)
	tv = (real) 0.0;
    }

  switch (non_negative_smallest (th, tv))
    {
    case FIRST_SMALLEST:
      info->time = th;
      info->extra.bw.wall = (b->vel.dx < (real) 0.0) ? WALL_LEFT : WALL_RIGHT;
      break;
    case SECOND_SMALLEST:
      info->time = tv;
      info->extra.bw.wall = (b->vel.dy < (real) 0.0) ? WALL_TOP : WALL_BOTTOM;
      break;
    default:
      info->time = NO_SOLUTION;
    }

  if (debugging && info->time != NO_SOLUTION)
    {
      debug_time (info->time);
      debug_ball_short (b);
      debug_wall (info->extra.bw.wall);
    }
}

static void
pw_find_collision (collision_info * info)
{
  if (pad.vel.dx > (real) 0.0)
    {
      info->time =
	(wall.right - pad.length / (real) 2.0 - pad.pos.x) / pad.vel.dx;
      info->extra.pw.wall = WALL_RIGHT;
    }
  else if (pad.vel.dx < (real) 0.0)
    {
      info->time =
	(wall.left + pad.length / (real) 2.0 - pad.pos.x) / pad.vel.dx;
      info->extra.pw.wall = WALL_LEFT;
    }
  else
    info->time = NO_SOLUTION;

  if (debugging && info->time != NO_SOLUTION)
    {
      debug_time (info->time);
      debug_pad_short ();
      debug_wall (info->extra.pw.wall);
    }
}

static void
move_pad (real t)
{
  pad.pos.x += t * pad.vel.dx;
  /* dy should always be zero. */
  pad.ends[0].pos.x = pad.pos.x - pad.rectlen / (real) 2.0;
  pad.ends[1].pos.x = pad.pos.x + pad.rectlen / (real) 2.0;
}

static void
move_balls (real t)
{
  ball *b;
  int i;
  for (i = 0; i < MAX_NBALLS; ++i)
    {
      b = &balls[i];
      if (b->active)
	{
	  b->pos.x += t * b->vel.dx;
	  b->pos.y += t * b->vel.dy;
	  if (b->event && b->event->move)
	    (*b->event->move) (b);
	}
    }
}

static void
dec_times (real t)
{
  collision_info *bbc = bb_collisions;
  int i, j;

  for (i = 0; i < MAX_NBALLS; ++i)
    if (balls[i].active)
      {
	bp_collisions[i].time -= t;
	bw_collisions[i].time -= t;
      }

  pw_collision.time -= t;

  for (i = 1; i < MAX_NBALLS; ++i)
    for (j = 0; j < i; ++j)
      {
	if (balls[i].active && balls[j].active)
	  bbc->time -= t;
	++bbc;
      }
}

static void
debug_collision (real t)
{
  debug ("C %g\n", t);
}

static void
calculate_bb_collision (collision_info * info)
{
  vector n, t, un, ut;		/* normal, tangent and unit normal, tangent */
  real d, a, b, n1, n2, t1, t2, n3, n4;

  ball *b1 = info->extra.bb.b1, *b2 = info->extra.bb.b2;
  ball *trig = NULL, *other;

  if (debugging)
    {
      debug_ball_short (b1);
      debug_ball_short (b2);
    }

  d = b1->radius + b2->radius;
  a = (b1->pos.x - b2->pos.x) / d;
  b = (b1->pos.y - b2->pos.y) / d;

  un.dx = a;
  un.dy = b;
  ut.dx = -b;
  ut.dy = a;

  /* components of the initial velocity in the normal and tangent */
  /* directions */
  n1 = DOTVECS (b1->vel, un);
  n2 = DOTVECS (b2->vel, un);
  t1 = DOTVECS (b1->vel, ut);
  t2 = DOTVECS (b2->vel, ut);

  /* components of the final velocity in the normal direction */
  /* the initial and final velocities in the tangent directions are the same */
  if (b1->mass == (real) 0.0)
    {				/* infinite mass (paddle) */
      n3 = n1;
      n4 = (real) 2.0 *n1 - n2;
    }
  else if (b2->mass == (real) 0.0)
    {
      n3 = (real) 2.0 *n2 - n1;
      n4 = n2;
    }
  else
    {
      n3 = ((b1->mass - b2->mass) * n1
	    + (real) 2.0 * b2->mass * n2) / (b1->mass + b2->mass);
      n4 = ((b2->mass - b1->mass) * n2 +
	    (real) 2.0 * b1->mass * n1) / (b1->mass + b2->mass);
    }

  n = un;
  SCALEVEC (n, n3);
  t = ut;
  SCALEVEC (t, t1);
  ADDVECS (n, t, b1->vel);

  n = un;
  SCALEVEC (n, n4);
  t = ut;
  SCALEVEC (t, t2);
  ADDVECS (n, t, b2->vel);

  /* Is one of these balls the trigger? */
  if (b1->spcl == SPCL_TRIGGER)
    {
      trig = b1;
      other = b2;
    }
  else if (b2->spcl == SPCL_TRIGGER)
    {
      trig = b2;
      other = b1;
    }

  /* other->index >= 0 because the paddle uses -1 there and we don't */
  /* want to transfer the trigger status to one of the ends of the paddle. */
  /* We also can't give trigger status to any special balls. */
  if (trig && other->index >= 0 && other->spcl != SPCL_SPECIAL)
    {
      trig->spcl = SPCL_NONE;
      other->spcl = SPCL_TRIGGER;
      trig->remove = other->remove;
      other->remove = false;
      trigidx = other->index;
      pad.img = getpadsurf (trigidx);
    }

  b1->recalc = b2->recalc = true;
  b1->last_hit = b2->index;
  b2->last_hit = b1->index;

  if (debugging)
    {
      debug_ball_short (b1);
      debug_ball_short (b2);
    }
}

static int
score_from_speed (real speed)
{
  speed = PPF2PPS (speed);	/* Don't make it depend on FPS. */
  if (speed < (real) 10.0)
    return 10;
  return (rround (speed) / 10) * 10;
}

static void
calculate_bp_collision (collision_info * info)
{
  collision_info dummy;
  ball *b = info->extra.bp.b;

  if (debugging)
    {
      debug_ball_short (b);
      debug_pad_short ();
    }

  /* Give the player a little some'n for hitting a ball. */
  inc_score (score_from_speed (BALL_SPEED (b)));

  /* Figure out the physics. */
  if (info->extra.bp.pad_part == PAD_PART_CENTER)
    b->vel.dy = -b->vel.dy;
  else if (info->extra.bp.pad_part == PAD_PART_LEFT)
    {
      dummy.extra.bb.b1 = b;
      dummy.extra.bb.b2 = &pad.ends[0];
      calculate_bb_collision (&dummy);
      /* Fix paddle's velocity because the collision might have changed. */
      pad.ends[0].vel = pad.vel;
    }
  else
    {				/* right part */
      dummy.extra.bb.b1 = b;
      dummy.extra.bb.b2 = &pad.ends[1];
      calculate_bb_collision (&dummy);
      /* Fix paddle's velocity because the collision might have changed. */
      pad.ends[1].vel = pad.vel;
    }

  b->recalc = true;
  b->last_hit = LASTHIT_PAD;
  pad.last_hit = b->index;

  if (b->event && b->event->pad_hit)
    (*b->event->pad_hit) (b);

  if (debugging)
    debug_ball_short (b);
}

#define LIFEOVER	true

static bool
calculate_bw_collision (collision_info * info)
{
  ball *b = info->extra.bw.b;
  bool retval = (!LIFEOVER);

  if (debugging)
    {
      debug_ball_short (b);
      debug_wall (info->extra.bw.wall);
    }

  switch (info->extra.bw.wall)
    {
    case WALL_LEFT:
      b->last_hit = LASTHIT_LWALL;
      b->vel.dx = (real) fabs (b->vel.dx);
      break;
    case WALL_RIGHT:
      b->last_hit = LASTHIT_RWALL;
      b->vel.dx = -(real) fabs (b->vel.dx);
      break;
    case WALL_TOP:
      if (b->remove)
	{
	  b->active = false;
	  break;
	}
      b->last_hit = LASTHIT_TWALL;
      b->vel.dy = (real) fabs (b->vel.dy);
      break;
    case WALL_BOTTOM:
      if (b->spcl == SPCL_TRIGGER)
	{
	  retval = LIFEOVER;
	  goto done;
	}
      b->last_hit = LASTHIT_BWALL;
      b->vel.dy = -(real) fabs (b->vel.dy);
      break;
    }

  b->recalc = true;

done:
  if (debugging)
    debug_ball_short (b);

  return retval;
}

static void
calculate_pw_collision (collision_info * info)
{
  if (debugging)
    {
      debug_pad_short ();
      debug_wall (info->extra.pw.wall);
    }

  set_pad_vel ((real) 0.0);
}

static void
fastpad_timeup (void)
{
  real v = pad.vel.dx;
  pad_speed = PPS2PPF (INIT_PAD_SPEED_PPS);
  set_pad_vel (v < (real) 0.0 ? -pad_speed :
	       (v > (real) 0.0 ? +pad_speed : (real) 0.0));
}

static void
fastpad_start (void)
{
  real v = pad.vel.dx;
  pad_speed = (real) 3.0 *PPS2PPF (INIT_PAD_SPEED_PPS) / (real) 2.0;
  set_pad_vel (v < (real) 0.0 ? -pad_speed :
	       (v > (real) 0.0 ? +pad_speed : (real) 0.0));
}

static void
slowpad_start (void)
{
  real v = pad.vel.dx;
  pad_speed = PPS2PPF (INIT_PAD_SPEED_PPS) / (real) 2.0;
  set_pad_vel (v < (real) 0.0 ? -pad_speed :
	       (v > (real) 0.0 ? +pad_speed : (real) 0.0));
}

static void
update_counters (void)
{
  ++updates;
  now = (real) updates;

  if (blackout > 0)
    --blackout;

  if (fastpad > 0)
    {
      --fastpad;
      if (fastpad == 0)
	fastpad_timeup ();
    }
  else if (fastpad < 0)
    {
      ++fastpad;
      if (fastpad == 0)
	fastpad_timeup ();
    }
}

static bool
gamemode_update (void)
{
  real tleft = (real) 1.0;
  collision_info *bbc, *next;
  int i, j;

  update_counters ();

  while (tleft > (real) 0.0)
    {
      real tmin = MAX_REAL;

      /* Check for collisions between balls and the paddle. */
      for (i = 0; i < MAX_NBALLS; ++i)
	if (balls[i].active)
	  {
	    if (pad.recalc || balls[i].recalc)
	      bp_find_collision (&balls[i], &bp_collisions[i]);
	    if (bp_collisions[i].time >= (real) 0.0
		&& bp_collisions[i].time < tmin)
	      {
		next = &bp_collisions[i];
		tmin = next->time;
	      }
	  }

      /* Check for collisions between balls and walls. */
      for (i = 0; i < MAX_NBALLS; ++i)
	if (balls[i].active)
	  {
	    if (balls[i].recalc)
	      bw_find_collision (&balls[i], &bw_collisions[i]);
	    if (bw_collisions[i].time >= (real) 0.0
		&& bw_collisions[i].time < tmin)
	      {
		next = &bw_collisions[i];
		tmin = next->time;
	      }
	  }

      /* Check for collisions of the paddle with a wall. */
      if (pad.recalc)
	pw_find_collision (&pw_collision);
      if (pw_collision.time >= (real) 0.0 && pw_collision.time < tmin)
	{
	  next = &pw_collision;
	  tmin = next->time;
	}

      /* Check for collisions between balls. */
      bbc = bb_collisions;
      for (i = 1; i < MAX_NBALLS; ++i)
	for (j = 0; j < i; ++j)
	  {
	    if (balls[i].active && balls[j].active)
	      {
		if (balls[i].recalc || balls[j].recalc)
		  bb_find_collision (&balls[i], &balls[j], bbc);
		if (bbc->time >= (real) 0.0 && bbc->time < tmin)
		  {
		    next = bbc;
		    tmin = next->time;
		  }
	      }
	    ++bbc;
	  }

      pad.recalc = false;
      for (i = 0; i < MAX_NBALLS; ++i)
	if (balls[i].active)
	  balls[i].recalc = false;

      if (tmin <= tleft)
	{
	  move_pad (tmin);
	  move_balls (tmin);
	  dec_times (tmin);

	  now += tmin;
	  tleft -= tmin;

	  if (debugging)
	    debug_collision (now);

	  switch (next->type)
	    {
	    case BP_COLLISION:
	      calculate_bp_collision (next);
	      break;
	    case BW_COLLISION:
	      if (calculate_bw_collision (next) == LIFEOVER)
		return LIFEOVER;
	      break;
	    case BB_COLLISION:
	      calculate_bb_collision (next);
	      break;
	    case PW_COLLISION:
	      calculate_pw_collision (next);
	      break;
	    }
	}
      else
	{
	  move_pad (tleft);
	  move_balls (tleft);
	  dec_times (tleft);
	  break;
	}

    }				/* while tleft >= 0 */
  redraw = true;
  return !LIFEOVER;
}

static void
perform_speed_check (void)
{
  real total_speed = (real) 0.0;
  int i;

  last_speed_check = updates;

  for (i = 0; i < MAX_STDBALLS; ++i)
    if (balls[i].active)
      total_speed += BALL_SPEED (&balls[i]);

  if (total_speed < level.too_slow)
    {
      /* Add a ball */
      for (i = 0; i < MAX_STDBALLS; ++i)
	if (!balls[i].active)
	  {
	    reset_ball (i);
	    balls[i].active = balls[i].patched = true;
	    return;
	  }
    }
  else if (total_speed > level.too_fast)
    {
      int nballs = 0;
      /* Remove a ball */
      for (i = 0; i < MAX_STDBALLS; ++i)
	if (balls[i].active)
	  {
	    if (balls[i].remove)
	      return;		/* One is already being removed. */
	    else
	      ++nballs;
	  }

      if (nballs <= MIN_STDBALLS)
	return;

      for (; i >= 0; --i)
	if (balls[i].active && balls[i].spcl != SPCL_TRIGGER)
	  {
	    balls[i].remove = true;
	    balls[i].recalc = true;
	    return;
	  }
    }
}

static void
activate_special (int i, int type)
{
  ball *b = &balls[i];
  b->img = getballsurf (SPECIALS + type);	/* should not fail */
  b->radius = (real) b->img->w / (real) 2.0;
#if 0				/* FIXME: Can't do this here! */
  if (b->radius > (wall.right - wall.left - (real) 2.0) / (real) 2.0
      || b->radius > (wall.bottom - wall.top) / (real) 2.0)
    fatal ("because ball #%d is too big", i);
#endif
  b->patch.w = b->img->w;
  b->patch.h = b->img->h;

  reset_ball (i);
  b->active = b->patched = true;

  b->spcl = SPCL_SPECIAL;
  b->event = specials[type];

  if (b->event->activate)
    (*b->event->activate) (b);
}

static void
maybe_add_special (void)
{
  int i;
  int nspecial = 0;

  last_add_special = updates;

  for (i = MAX_STDBALLS; i < MAX_NBALLS; ++i)
    if (balls[i].active)
      ++nspecial;

  if (MAX_STDBALLS + nspecial == MAX_NBALLS)
    return;			/* No more slots left. */

  /* 1 out of nspecial times, add a random special ball. */
  if (rrand ((real) 0.0, (real) nspecial) <= (real) 1.0)
    {
      int type = rand () % NSPECIALS;

      /* Find a free slot. */
      for (i = MAX_STDBALLS; i < MAX_NBALLS; ++i)
	if (!balls[i].active)
	  break;

      activate_special (i, type);
    }
}

static void leave_gamemode (void);

static int
gamemode_loopfunc (void)
{
  static Uint32 last_update = 0;

  if (clear_gamemode_queue () != CQ_CONTINUE)
    return LOOPFUNC_BREAK;

  if (!paused && ((real) (SDL_GetTicks () - last_update)
		  >= (real) 1000.0 * FRAMES2SECONDS (1)))
    {

      /* Don't do these if we are using a debug file. */
      if (!debugfile && ((int) (updates - last_speed_check)
			 >= SECONDS2FRAMES (SPEED_CHECK_DELAY_SECONDS)))
	perform_speed_check ();
      if (!debugfile && ((int) (updates - last_add_special)
			 >= SECONDS2FRAMES (ADD_SPECIAL_DELAY_SECONDS)))
	maybe_add_special ();

      if (gamemode_update () == LIFEOVER)
	{
	  if (lives.number > 0)
	    {
	      set_lives (lives.number - 1);
	      new_life ();
	    }
	  else
	    {
	      if (!debugging && !debugfile && is_highscore (score.value))
		{
		  leave_gamemode ();
		  enter_setscoremode (bgsurf, score.value);
		  return LOOPFUNC_CONTINUE;
		}
	      else		/* STUB */
		reset_gamemode ();
	    }
	  refresh_gamemode_display ();
	}

      last_update = SDL_GetTicks ();
    }
  else
    SDL_Delay (1);		/* Give back some CPU cycles. */

  if (redraw)
    {
      patch_gamemode_display ();
      redraw_gamemode_display ();
    }

  return LOOPFUNC_CONTINUE;
}

void
enter_gamemode (void)
{
  if (gfont == NULL)
    {
      SDL_Color color;
      if (!get_color ("Font-Color", &color))
	color = default_gfont_color;
      gfont = acquire_medium_font (color);
    }

  reset_gamemode ();
  refresh_gamemode_display ();
  loopfunc = gamemode_loopfunc;
}

static void
leave_gamemode (void)
{
  release_font (gfont);
  gfont = NULL;
}

static void
activate_blackout (ball * b)
{
  b->lifetime = SECONDS2FRAMES (BLACKOUT_LIFETIME_SECONDS);
}

static void
activate_fastpad (ball * b)
{
  b->lifetime = SECONDS2FRAMES (FASTPAD_LIFETIME_SECONDS);
}

static void
activate_slowpad (ball * b)
{
  b->lifetime = SECONDS2FRAMES (SLOWPAD_LIFETIME_SECONDS);
}

static void
pad_hit_blackout (ball * b)
{
  no_warn_about_unused (b);
  blackout = SECONDS2FRAMES (BLACKOUT_TIME_LIMIT_SECONDS);
}

static void
pad_hit_fastpad (ball * b)
{
  no_warn_about_unused (b);
  if (fastpad < 0)
    {
      fastpad = 0;
      fastpad_timeup ();
    }
  else
    {
      fastpad = SECONDS2FRAMES (FASTPAD_TIME_LIMIT_SECONDS);
      fastpad_start ();
    }
}

static void
pad_hit_slowpad (ball * b)
{
  no_warn_about_unused (b);
  if (fastpad > 0)
    {
      fastpad = 0;
      fastpad_timeup ();
    }
  else
    {
      fastpad = SECONDS2FRAMES (SLOWPAD_TIME_LIMIT_SECONDS);
      slowpad_start ();
    }
}

static void
move_timeout (ball * b)
{
  if (b->lifetime > 0)
    {
      --b->lifetime;
      if (b->lifetime == 0)
	{
	  b->remove = true;
	  b->recalc = true;
	}
    }
}

/*
 * Use 'file' to setup internal state. Useful for debugging.
 */
static void
debug_setup (const char *file)
{
#define LINELEN	80
  char line[LINELEN];
  FILE *fp;
  int i;

  fp = fopen (file, "r");
  if (fp == NULL)
    fatal ("can't open %s: %s", file, strerror (errno));

  /* Deactivate all balls that we don't want. */
  for (i = 0; i < MAX_NBALLS; ++i)
    balls[i].active = false;

  while (fgets (line, LINELEN, fp) && line[0] != '\n')
    {
      /* Trim trailing newline. */
      int len = strlen (line);
      if (line[len - 1] == '\n')
	line[len - 1] = '\0';

      if (line[0] == 'P')
	{
	  const char *format;
	  real x, dx;

	  if (sizeof (real) == sizeof (float))
	    format = "%g,%g";
	  else			/* Assume real == double. */
	    format = "%lg,%lg";

	  if (sscanf (line + 1, format, &x, &dx) != 2)
	    fatal ("%s: don't understand \"%s\"", file, line);

	  /* Sanity check. */
	  if (x < wall.left + pad.thickness / (real) 2.0)
	    x = wall.left + pad.thickness / (real) 2.0;
	  else if (x > wall.right - pad.thickness / (real) 2.0)
	    x = wall.right - pad.thickness / (real) 2.0;

#if 0				/* Don't automatically move paddle. */
	  /* Which way are we going? */
	  if (dx > (real) 0.0)
	    pad.vel.dx = PPS2PPF (INIT_PAD_SPEED_PPS);
	  else if (dx < (real) 0.0)
	    pad.vel.dx = -PPS2PPF (INIT_PAD_SPEED_PPS);
	  else
	    pad.vel.dx = (real) 0.0;
#endif

	  /* How fast? */
	  if (fabs (dx) > PPS2PPF (INIT_PAD_SPEED_PPS))
	    fastpad_start ();
	  else if (fabs (dx) < PPS2PPF (INIT_PAD_SPEED_PPS)
		   && dx != (real) 0.0)
	    slowpad_start ();

	  pad.pos.x = x;
	  move_pad ((real) 0.0);	/* Synchronize ends. */

	}
      else if (line[0] == 'B')
	{
	  const char *format;
	  point pos;
	  vector vel;
	  unsigned int index, spcl;

	  if (sizeof (real) == sizeof (float))
	    format = "%u,%g,%g,%g,%g,%u";
	  else			/* Assume real == double */
	    format = "%u,%lg,%lg,%lg,%lg,%u";

	  if (sscanf (line + 1, format,
		      &index, &pos.x, &pos.y, &vel.dx, &vel.dy, &spcl) != 6)
	    fatal ("%s: don't understand \"%s\"", file, line);

	  if (index >= MAX_NBALLS)
	    fatal ("%s: bad index \"%s\"", file, line);
	  else
	    {
	      ball *b = &balls[index];

	      if (index >= MAX_STDBALLS)
		{
		  if (spcl >= NSPECIALS)
		    fatal ("%s: bad special id \"%s\"", file, line);
		  activate_special (index, spcl);
		}
	      else
		{
		  reset_ball (index);
		  b->active = b->patched = true;
		}

	      if (pos.x < wall.left + b->radius)
		pos.x = wall.left + b->radius;
	      else if (pos.x > wall.right + b->radius)
		pos.x = wall.right + b->radius;
	      if (pos.y < wall.top + b->radius)
		pos.y = wall.top + b->radius;
	      else if (pos.y > wall.bottom + b->radius)
		pos.y = wall.bottom + b->radius;

	      b->pos = pos;
	      b->vel = vel;
	    }

	}
      else
	warning ("%s: ignoring line \"%s\"", file, line);
    }

  if (ferror (fp))
    warning ("read error in %s: %s", file, strerror (errno));
  fclose (fp);

  paused = true;

#undef LINELEN
}
