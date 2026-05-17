#include "formula.h"
#include "table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *LoadFile(const char *path)
{
  char *data;
  size_t length;
  FILE *file = fopen(path, "r");
  if (!file) return NULL;
  fseek(file, 0, SEEK_END);
  length = ftell(file);
  fseek(file, 0, SEEK_SET);
  if ((data = malloc(length + 1)))
  {
    fread(data, 1, length, file);
    data[length] = 0;
  }
  fclose(file);
  return data;
}

int ParseCSV(const char *csv)
{
  int *cell;
  unsigned column, row = 0;
  size_t len, i = strcspn(csv, ",\n");
  while (csv[i] == ',')
  {
    len = strcspn(csv + ++i, ",\n");
    if (AddColumn(csv + i, len) < 0) return 0;
    i += len;
  }
  while (csv[i++])
  {
    len = strcspn(csv + i, ",\n");
    if (csv[i + len] == ',' && AddRow(csv + i, len) < 0) return 0;
    i += strcspn(csv + i, "\n");
  }
  if (!InitTable()) return 0;
  i = strcspn(csv, "\n");
  while (csv[i++])
  {
    column = 0;
    i += strcspn(csv + i, ",\n");
    while (csv[i] == ',')
    {
      cell = Cell(column++, row);
      if (csv[++i] == '=') AddFormula(csv + i, cell);
      else sscanf(csv + i, "%d", cell);
      i += strcspn(csv + i, ",\n");
    }
    ++row;
  }
  return 1;
}

int main(int argc, char *argv[])
{
  char *csv;
  if (argc != 2)
  {
    fprintf(stderr, "ERROR: Invalid number of arguments!\n");
    return EXIT_FAILURE;
  }
  if (!(csv = LoadFile(argv[1])))
  {
    fprintf(stderr, "ERROR: Could not load the file!\n");
    return EXIT_FAILURE;
  }
  if (!ParseCSV(csv))
  {
    FreeTable();
    fprintf(stderr, "ERROR: Failed to parse CSV!\n");
    return EXIT_FAILURE;
  }
  ResolveFormulas();
  PrintTable();
  FreeTable();
  free(csv);
  return EXIT_SUCCESS;
}

