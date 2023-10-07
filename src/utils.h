#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

FILE *open_for_writing(char *filename);
FILE *open_append(char *filename);
void *smalloc(size_t size, int init);

#endif /* not UTILS_H */

