/* Copyright (C) Daniel Hipschman, Dec 2004 */
/* $Id: defines.h,v 1.9 2006/06/15 23:20:45 dsh Exp $ */

#ifndef DEFINES_H
#define DEFINES_H

#include <float.h>
#include <stddef.h>

#ifndef __cplusplus
#  if (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#    include <stdbool.h>
#  else
typedef unsigned int bool;
#    define true 1
#    define false 0
#  endif
#endif /* __cplusplus */

#undef MIN
#undef MAX
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* A point on or off the screen. */
typedef struct
{
  int x, y;
} coord;

/* All physics are calculated with the "real" type. */
typedef float real;
#define MAX_REAL	(FLT_MAX)
/* Generate a random real number in the range [a,b]. */
real rrand (real a, real b);
int rround (real s);

typedef struct
{
  real x, y;
} point;

typedef struct
{
  real dx, dy;
} vector;

/* Operations on points and vectors. */
#define SUBPOINTS(a,b,v) ((void)	\
    ((v).dx = (a).x - (b).x,		\
     (v).dy = (a).y - (b).y))

#define SETVEC(v,x,y) ((void)		\
    ((v).dx = (x),			\
     (v).dy = (y)))

#define SCALEVEC(v,r) ((void)		\
    ((v).dx *= (r),			\
     (v).dy *= (r)))

#define DOTVECS(u,v) (			\
    (u).dx * (v).dx + (u).dy * (v).dy)

#define ADDVECS(u,v,w) ((void)		\
    ((w).dx = (u).dx + (v).dx,		\
     (w).dy = (u).dy + (v).dy))

/* Function called repeatedly until it returns LOOPFUNC_BREAK. */
typedef int loopfunction (void);
extern loopfunction *loopfunc;
/* Values returned by "loopfunction"s to indicate whether the main loop */
/* should terminate, or continue. */
#define LOOPFUNC_BREAK		0
#define LOOPFUNC_CONTINUE	1

extern bool debugging;
extern const char *debugfile;

void fatal (const char *fmt, ...);
void warning (const char *fmt, ...);

void *xalloc (size_t nbytes);
void xfree (void *xptr);
char *xstrdup (const char *str);

#define XALLOC(T, n) ((T *) xalloc ((n) * sizeof (T)))

/* Case insensitive string compare. */
int stricmp (const char *s1, const char *s2);

/*
 * Returns a pointer to the first non-whitespace character in the string.
 * The return value, while not explicitly const, will be const if the 
 * argument is const.
 */
char *first_nonspace (const char *str);
/*
 * Returns a pointer to the first character after the last non-whitespace
 * character in the string.  The return value, while not explicitly const,
 * will be const if the argument is const.
 */
char *one_after_last_nonspace (const char *str);

/* Shut compiler up about unused function parameters. */
void no_warn_about_unused (const void *);
#define no_warn_about_unused(p) (no_warn_about_unused (&(p)))

#endif /* header guard */
