#include "formula.h"

#include "table.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define F_LHS 1
#define F_RHS 2

typedef union _Argument
{
  int *cell;
  int value;
} Argument;

typedef struct _Formula
{
  struct _Formula *next;
  int             *cell;
  Argument        lhs, rhs;
  int             op, flags;
} Formula;

static Formula stop = { 0 };
static Formula *last = &stop;
static Formula *first = NULL;

void AddFormula(const char *eq, int *cell)
{
  Formula *formula;
  if (*eq != '=' || !(formula = malloc(sizeof(*formula)))) return;
  if (!first) first = formula;
  last = last->next = formula;
  formula->next = &stop;
  formula->cell = cell;
  /* TODO: NOT IMPLEMENTED! */
}

static int _IsResolved(int *cell)
{
  Formula *formula;
  for (formula = first; formula; formula = formula->next)
  {
    if (formula->cell == cell) return 0;
  }
  return 1;
}

static int _ResolveOne(int force)
{
  Formula *next = first->next;
  if (!(first->flags & F_LHS) && (force || _IsResolved(first->lhs.cell)))
  {
    first->lhs.value = *first->lhs.cell;
    first->flags |= F_LHS;
  }
  if (!(first->flags & F_RHS) && (force || _IsResolved(first->rhs.cell)))
  {
    first->rhs.value = *first->rhs.cell;
    first->flags |= F_RHS;
  }
  if ((first->flags & (F_LHS | F_RHS)) == (F_LHS | F_RHS))
  {
    switch (first->op)
    {
    case '+':
      *first->cell = first->lhs.value + first->rhs.value;
      break;
    case '-':
      *first->cell = first->lhs.value - first->rhs.value;
      break;
    case '*':
      *first->cell = first->lhs.value * first->rhs.value;
      break;
    case '/':
      *first->cell = first->lhs.value / first->rhs.value;
      break;
    }
    free(first);
  }
  else last = last->next = first;
  if ((first = next) == &stop)
  {
    first = first->next;
    last->next = &stop;
    stop.next = NULL;
    return 0;
  }
  return 1;
}

void ResolveFormulas(void)
{
  unsigned index = 0;
  unsigned count = -1;
  if (first) while (count != index)
  {
    count = index;
    index = 0;
    last = &stop;
    while (_ResolveOne(0)) ++index;
  }
  if (index)
  {
    last = &stop;
    while (_ResolveOne(1));
  }
}

