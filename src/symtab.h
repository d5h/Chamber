/* Copyright (C) Daniel Hipschman 2005/05 */
/* $Id: symtab.h,v 1.2 2006/06/15 23:20:45 dsh Exp $ */

#ifndef SYMTAB_H
#define SYMTAB_H

/* Initialize to NULL for empty list. */
typedef struct sym *symtab;

void symtab_add (symtab * opts, const char *name, const char *value);
void symtab_free (symtab * opts);
const char *symtab_find (symtab opts, const char *name);

#endif /* SYMTAB_H */
