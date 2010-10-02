#include <stdio.h>

#include <assert.h>

#include "piece_name.h"

#include <string.h>

int main(int argc, char *argv[]) {

  char *str;

  str = piece_name('K');

  if (strcmp(str, "King")) {
    printf("%s: Expected King but got %s.\n", __FUNCTION__, str);
    return -1;
  }

  str = piece_name('N');

  if (strcmp(str, "Knight")) {
    printf("%s: Expected Knight but got %s.\n", __FUNCTION__, str);
    return -1;
  }

  str = piece_name('X');

  if (!strcmp(str, "Unknown")) {
    printf("OK\n");
    return 0;
  }

  printf("%s: Expected Unknown but got %s.\n", __FUNCTION__, str);

  return -1;

}
