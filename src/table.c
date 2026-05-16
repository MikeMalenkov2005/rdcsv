#include "table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int *table = NULL;

static unsigned width = 0;
static unsigned height = 0;

static const char **columns = NULL;
static unsigned *rows = NULL;

int AddColumn(const char *name)
{
  const char **new;
  static unsigned length = 0;
  if (width == length)
  {
    if (!(new = realloc(columns, (length += 32) * sizeof(*columns))))
    {
      length -= 32;
      return -1;
    }
    columns = new;
  }
  columns[width] = name;
  return width++;
}

int GetColumn(const char *name)
{
  unsigned i;
  for (i = 0; i < width; ++i) if (!strcmp(name, columns[i])) return i;
  return -1;
}

int AddRow(unsigned index)
{
  unsigned *new;
  static unsigned length = 0;
  if (width == length)
  {
    if (!(new = realloc(rows, (length += 32) * sizeof(*rows))))
    {
      length -= 32;
      return -1;
    }
    rows = new;
  }
  rows[width] = index;
  return width++;
}

int GetRow(unsigned index)
{
  unsigned i;
  for (i = 0; i < height; ++i) if (index == rows[i]) return i;
  return -1;
}

int *Cell(unsigned column, unsigned row)
{
  if (column >= width || row >= height) return NULL;
  return table + column + (size_t)width * row;
}

void PrintTable(void)
{
  unsigned i, j;
  for (i = 0; i < width; ++i)
  {
    printf(",%s", columns[i]);
  }
  putchar('\n');
  for (j = 0; j < height; ++j)
  {
    printf("%u", rows[j]);
    for (i = 0; i < width; ++i)
    {
      printf(",%d", *Cell(i, j));
    }
    putchar('\n');
  }
}

void FreeTable(void)
{
  if (table)
  {
    free(table);
    table = NULL;
  }
  if (columns)
  {
    free(columns);
    columns = NULL;
  }
  if (rows)
  {
    free(rows);
    rows = NULL;
  }
  width = height = 0;
}

