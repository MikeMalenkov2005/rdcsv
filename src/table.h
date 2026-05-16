#ifndef TABLE_H
#define TABLE_H

int AddColumn(const char *name);
int GetColumn(const char *name);

int AddRow(unsigned index);
int GetRow(unsigned index);

int *Cell(unsigned column, unsigned row);

void PrintTable(void);
void FreeTable(void);

#endif

