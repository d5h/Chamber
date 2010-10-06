/* Copyright (C) Daniel Hipschman 2005 */
/* $Id: common.h,v 1.1 2005/04/25 22:47:47 sirdan Exp $ */

#ifndef UTIL_COMMON_H
#define UTIL_COMMON_H

/*
 * Define whatever the directory separator for your machine is.
 * Note, '/' will probably work on Windows because the standard
 * libraries are usually kind enough to allow it.
 */
#define DIRSEP '/'

/* Remove directory prefix and extension. */
char *rootname (const char *filename);
void fatal (const char *fmt, ...);
void *xmalloc (size_t size);
void xprintf (FILE * fp, const char *fmt, ...);
FILE *create_c_file (const char *name);

#endif /* UTIL_COMMON_H */
