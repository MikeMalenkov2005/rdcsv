#include "table.h"
#include "formula.h"
#include "parse.h"

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
  if ((data = calloc(length + 1, 1)))
  {
    fread(data, 1, length, file);
  }
  fclose(file);
  return data;
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

