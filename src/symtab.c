/* Copyright (C) Daniel Hipschman 2005/05 */
/* $Id: symtab.c,v 1.2 2006/06/15 23:20:45 dsh Exp $ */

#include "symtab.h"
#include "defines.h"

typedef struct sym sym;
struct sym
{
  char *name, *value;
  sym *next;
};

void
symtab_add (symtab * tab, const char *name, const char *value)
{
  sym *s = XALLOC (sym, 1);
  s->name = xstrdup (name);
  s->value = xstrdup (value);
  s->next = *tab;
  *tab = s;
}

void
symtab_free (symtab * tab)
{
  sym *head, *tail;
  for (head = *tab; head; head = tail)
    {
      tail = head->next;
      xfree (head->name);
      xfree (head->value);
      xfree (head);
    }
  *tab = NULL;
}

const char *
symtab_find (symtab tab, const char *name)
{
  sym *head;
  for (head = tab; head; head = head->next)
    if (stricmp (head->name, name) == 0)
      return head->value;
  return NULL;
}
