#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

int AddColumn(const char *name, size_t length);
int GetColumn(const char *name, size_t length);

int AddRow(const char *name, size_t length);
int GetRow(const char *name, size_t length);

unsigned GetWidth(void);
unsigned GetHeight(void);

int *Cell(unsigned column, unsigned row);

int GetCellColumn(int *cell);
int GetCellRow(int *cell);

void PrintTable(void);

int InitTable(void);
void FreeTable(void);

#endif

