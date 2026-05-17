#include "formula.h"

#include "table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

static Formula *last = NULL;
static Formula *first = NULL;

static int *_GetCell(const char *id, size_t length)
{
  size_t i = length - 1;
  while (isdigit(id[i - 1])) --i;
  while (i < length - 1 && id[i] == '0') ++i;
  return Cell(GetColumn(id, i), GetRow(id + i, length - i));
}

int AddFormula(const char *eq, int *cell)
{
  Formula *formula;
  size_t len = 0, i = 1;
  if (!(formula = malloc(sizeof(*formula)))) return 0;
  formula->next = NULL;
  formula->cell = cell;
  formula->flags = F_LHS | F_RHS;
  if (eq[i] != '-' && eq[i] != '+' && !isdigit(eq[i]))
  {
    len = strcspn(eq + i, "+-*/,\n");
    if (!(formula->lhs.cell = _GetCell(eq + i, len)))
    {
      free(formula);
      return 0;
    }
    formula->flags &= ~F_LHS;
  }
  else sscanf(eq + i, "%i%zn", &formula->lhs.value, &len);
  i += len;
  if (eq[i] != '+' && eq[i] != '-' && eq[i] != '*' && eq[i] != '/')
  {
    free(formula);
    return 0;
  }
  formula->op = eq[i++];
  if (eq[i] != '-' && eq[i] != '+' && !isdigit(eq[i]))
  {
    len = strcspn(eq + i, ",\n");
    if (!(formula->rhs.cell = _GetCell(eq + i, len)))
    {
      free(formula);
      return 0;
    }
    formula->flags &= ~F_RHS;
  }
  else sscanf(eq + i, "%i", &formula->rhs.value);
  if (!first) first = last = formula;
  else last = last->next = formula;
  return 1;
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
  static Formula *new_first = NULL;
  static Formula *new_last = NULL;
  Formula *next;
  if (!first)
  {
    first = new_first;
    last = new_last;
    new_first = NULL;
    new_last = NULL;
    return 0;
  }
  next = first->next;
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
    memset(first, 0, sizeof(*first));
    free(first);
  }
  else
  {
    first->next = NULL;
    if (!new_first) new_first = new_last = first;
    else new_last = last->next = first;
  }
  first = next;
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
    while (_ResolveOne(1)) ++index;
  }
  if (index) while (_ResolveOne(1));
}

