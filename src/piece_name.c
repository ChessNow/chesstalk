
#include <stdio.h>
#include <assert.h>

#include "piece_name.h"

#include <string.h>

char *piece_names[] = { "Knight", "Bishop", "King", "Rook", "Queen", "Unknown" };

// must match the sequence set out in piece_chars.c

#include "piece_chars.h"

char *piece_name(char c) {

  char *s = piece_chars;

  for ( ; *s; s++) {

    if (*s == c) break;

  }

  return piece_names[s - piece_chars]; // Good match, or Unknown

}

