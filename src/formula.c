#include "formula.h"

#include "table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define LHS_COMPUTED_FLAG 1
#define RHS_COMPUTED_FLAG 2

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
static Formula *new_last = NULL;
static Formula *new_first = NULL;

static int *_GetCell(const char *id, size_t length)
{
  static const char *const allowed =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz_";
  size_t i = strspn(id, allowed);
  size_t len = strspn(id + i, "0123456789");
  if (strspn(id + i + len, "\f\r\v\t ") + i + len != length) return NULL;
  return Cell(GetColumn(id, i), GetRow(id + i, len));
}

int AddFormula(const char *eq, int *cell)
{
  Formula *formula;
  size_t len = 0, i = 1;
  if (!(formula = malloc(sizeof(*formula)))) return 0;
  formula->next = NULL;
  formula->cell = cell;
  formula->flags = LHS_COMPUTED_FLAG | RHS_COMPUTED_FLAG;
  i += strspn(eq + i, "\f\r\v\t ");
  if (eq[i] != '-' && eq[i] != '+' && !isdigit(eq[i]))
  {
    len = strcspn(eq + i, "+-*/%,#\n");
    if (!(formula->lhs.cell = _GetCell(eq + i, len)))
    {
      free(formula);
      return 0;
    }
    formula->flags &= ~LHS_COMPUTED_FLAG;
  }
  else sscanf(eq + i, "%i%zn", &formula->lhs.value, &len);
  i += strspn(eq + i + len, "\f\r\v\t ") + len;
  if (!eq[i] || !strchr("+-*/%", eq[i]))
  {
    free(formula);
    return 0;
  }
  len = 0;
  formula->op = eq[i++];
  i += strspn(eq + i, "\f\r\v\t ");
  if (eq[i] != '-' && eq[i] != '+' && !isdigit(eq[i]))
  {
    len = strcspn(eq + i, ",#\n");
    if (!(formula->rhs.cell = _GetCell(eq + i, len)))
    {
      free(formula);
      return 0;
    }
    formula->flags &= ~RHS_COMPUTED_FLAG;
  }
  else sscanf(eq + i, "%i%zn", &formula->rhs.value, &len);
  i += strspn(eq + i + len, "\f\r\v\t ") + len;
  if (eq[i] != ',' && eq[i] != '#' && eq[i] != '\n')
  {
    free(formula);
    return 0;
  }
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
  for (formula = new_first; formula; formula = formula->next)
  {
    if (formula->cell == cell) return 0;
  }
  return 1;
}

static int _ResolveOne(void)
{
  Formula *next;
  if (!first)
  {
    first = new_first;
    last = new_last;
    new_first = NULL;
    new_last = NULL;
    return 1;
  }
  next = first->next;
  if (!(first->flags & LHS_COMPUTED_FLAG) && _IsResolved(first->lhs.cell))
  {
    first->lhs.value = *first->lhs.cell;
    first->flags |= LHS_COMPUTED_FLAG;
  }
  if (!(first->flags & RHS_COMPUTED_FLAG) && _IsResolved(first->rhs.cell))
  {
    first->rhs.value = *first->rhs.cell;
    first->flags |= RHS_COMPUTED_FLAG;
  }
  if ((first->flags & LHS_COMPUTED_FLAG) && (first->flags & RHS_COMPUTED_FLAG))
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
      if (!first->rhs.value)
      {
        fprintf(stderr, "ERROR: Division by zero in column #%d on row #%d!\n",
            GetCellColumn(first->cell), GetCellRow(first->cell));
        return -1;
      }
      *first->cell = first->lhs.value / first->rhs.value;
      break;
    case '%':
      if (!first->rhs.value)
      {
        fprintf(stderr, "ERROR: Division by zero in column #%d on row #%d!\n",
            GetCellColumn(first->cell), GetCellRow(first->cell));
        return -1;
      }
      *first->cell = first->lhs.value % first->rhs.value;
    }
    free(first);
  }
  else
  {
    first->next = NULL;
    if (!new_first) new_first = new_last = first;
    else new_last = new_last->next = first;
  }
  first = next;
  return 0;
}

int ResolveFormulas(void)
{
  int result;
  unsigned index = 0;
  unsigned count = -1;
  if (first) while (count != index)
  {
    count = index;
    index = 0;
    while (!(result = _ResolveOne())) ++index;
    if (result < 0) return 0;
  }
  if (index)
  {
    fprintf(stderr,
        "ERROR: Formulas form a cycle and can not be resolved!\n"
        "       First unresolved formula in column #%u on row #%u\n",
        GetCellColumn(first->cell), GetCellRow(first->cell));
    while (first)
    {
      last = first->next;
      free(first);
      first = last;
    }
    return 0;
  }
  return 1;
}

