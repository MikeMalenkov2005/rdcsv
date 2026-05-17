#include "parse.h"

#include "table.h"
#include "formula.h"

#include <string.h>
#include <stdio.h>

int ParseCSV(const char *csv)
{
  int *cell;
  unsigned column, row = 0;
  size_t len, i = 0;
  csv += strspn(csv, "\f\r\v\t ");
  while (*csv == '#' || *csv == '\n')
  {
    /* Skipping leading whitespace, comments and empty lines */
    csv += strcspn(csv, "\n") + 1;
    csv += strspn(csv, "\f\r\v\t ");
  }
  if (*csv != ',')
  {
    fprintf(stderr, "ERROR: Table must begin with a comma!\n");
    return 0;
  }
  while (csv[i] == ',')
  {
    /* Parsing column headers */
    len = strcspn(csv + ++i, ",#\n");
    if (AddColumn(csv + i, len) < 0)
    {
      fprintf(stderr, "ERROR: Failed to add a column #%u!\n", GetWidth());
      return 0;
    }
    i += strcspn(csv + i, ",\n");
  }
  while (csv[i++])
  {
    i += strspn(csv + i, "\f\r\v\t ");
    while (csv[i] == '#' || csv[i] == '\n')
    {
      /* Skipping leading whitespace, comments and empty lines */
      i += strcspn(csv + i, "\n") + 1;
      i += strspn(csv + i, "\f\r\v\t ");
    }
    /* Parsing row indices */
    len = strcspn(csv + i, ",#\n");
    if (len && AddRow(csv + i, len) < 0)
    {
      fprintf(stderr, "ERROR: Failed to add a row #%u!\n", GetHeight());
      return 0;
    }
    i += strcspn(csv + i, "\n");
  }
  if (!InitTable())
  {
    fprintf(stderr,
        "ERROR: Invalid table size (%u x %u)!\n", GetWidth(), GetHeight());
    return 0;
  }
  i = strcspn(csv, "\n");
  while (csv[i++])
  {
    column = 0;
    i += strspn(csv + i, "\f\r\v\t ");
    while (csv[i] == '#' || csv[i] == '\n')
    {
      /* Skipping leading whitespace, comments and empty lines */
      i += strcspn(csv + i, "\n") + 1;
      i += strspn(csv + i, "\f\r\v\t ");
    }
    /* Parsing rows of cells */
    i += strcspn(csv + i, ",#\n");
    while (csv[i] == ',')
    {
      if (column == GetWidth())
      {
        fprintf(stderr, "ERROR: Row #%u is too long!\n", row);
        return 0;
      }
      cell = Cell(column++, row);
      /* Parsing cells in a row */
      i += strspn(csv + i + 1, "\f\r\v\t ") + 1;
      if (csv[i] == ',' || csv[i] == '#' || csv[i] == '\n')
      {
        fprintf(stderr, "ERROR Empty cell in column #%u on row #%u!\n",
            column - 1, row);
        return 0;
      }
      if (csv[i] == '=')
      {
        /* Parsing cell with formula */
        if (!AddFormula(csv + i, cell))
        {
          fprintf(stderr, "ERROR: Invalid formula in column #%u on row #%u!\n",
              column - 1, row);
          return 0;
        }
        i += strcspn(csv + i, ",#\n");
      }
      else
      {
        /* Parsing cell with integer */
        if (sscanf(csv + i, "%d%zn", cell, &len) != 1)
        {
          fprintf(stderr, "ERROR: Invalid cell in column #%u on row #%u!\n",
              column - 1, row);
          return 0;
        }
        i += strspn(csv + i + len, "\f\r\v\t ") + len;
        if (csv[i] != ',' && csv[i] != '#' && csv[i] != '\n')
        {
          fprintf(stderr, "ERROR: Invalid cell in column #%u on row #%u!\n",
              column - 1, row);
          return 0;
        }
      }
    }
    if (column && column < GetWidth())
    {
      fprintf(stderr, "ERROR: Row #%u is too short!\n", row);
      return 0;
    }
    i += strcspn(csv + i, "\n");
    ++row;
  }
  return 1;
}
