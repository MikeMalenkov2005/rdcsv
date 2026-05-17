#include "table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _String
{
  const char *data;
  size_t      size;
} String;

typedef struct _StringVector
{
  String    *data;
  unsigned  count;
  unsigned  limit;
} StringVector;

static int *table = NULL;
static StringVector rows = { 0 };
static StringVector columns = { 0 };

static int _Add(StringVector *vector, const char *string, size_t length)
{
  String *data;
  if (length > INT_MAX) return -1; /* If the length is greater printf fails! */
  if (vector->count == vector->limit)
  {
    data = realloc(vector->data, (vector->limit += 32) * sizeof(*data));
    if (!data)
    {
      vector->limit -= 32;
      return -1;
    }
    vector->data = data;
  }
  vector->data[vector->count].data = string;
  vector->data[vector->count].size = length;
  return vector->count++;
}

static int _Get(StringVector *vector, const char *string, size_t length)
{
  unsigned i = vector->count;
  while (i--)
  {
    if (vector->data[i].size == length &&
        !strncmp(vector->data[i].data, string, length)) return i;
  }
  return -1;
}

int AddColumn(const char *name, size_t length)
{
  /* No other characters are allowed in a column name! */
  static const char *const allowed =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz_";
  size_t len = strspn(name, "\f\r\v\t ");
  name += len;
  length -= len;
  len = strspn(name, allowed);
  if (len + strspn(name + len, "\f\r\v\t ") != length) return -1;
  return _Add(&columns, name, len);
}

int GetColumn(const char *name, size_t length)
{
  return _Get(&columns, name, length);
}

int AddRow(const char *name, size_t length)
{
  size_t len = strspn(name, "0123456789");
  if (!len || (len > 1 && *name == '0')) return -1;
  if (len + strspn(name + len, "\f\r\v\t ") != length) return -1;
  return _Add(&rows, name, len);
}

int GetRow(const char *name, size_t length)
{
  return _Get(&rows, name, length);
}

unsigned GetWidth(void)
{
  return columns.count;
}

unsigned GetHeight(void)
{
  return rows.count;
}

int *Cell(unsigned column, unsigned row)
{
  if (!table || column >= columns.count || row >= rows.count) return NULL;
  return table + (column + (size_t)columns.count * row);
}

void PrintTable(void)
{
  unsigned i, j;
  if (table)
  {
    for (i = 0; i < columns.count; ++i)
    {
      printf(",%.*s", (int)columns.data[i].size, columns.data[i].data);
    }
    putchar('\n');
    for (j = 0; j < rows.count; ++j)
    {
      printf("%.*s", (int)rows.data[j].size, rows.data[j].data);
      for (i = 0; i < columns.count; ++i)
      {
        printf(",%d", *Cell(i, j));
      }
      putchar('\n');
    }
  }
}

int InitTable(void)
{
  if (table) free(table);
  table = calloc(rows.count * columns.count, sizeof(*table));
  return !!table;
}

void FreeTable(void)
{
  if (table)
  {
    free(table);
    table = NULL;
  }
  if (columns.data)
  {
    free(columns.data);
    memset(&columns, 0, sizeof(columns));
  }
  if (rows.data)
  {
    free(rows.data);
    memset(&rows, 0, sizeof(rows));
  }
}

