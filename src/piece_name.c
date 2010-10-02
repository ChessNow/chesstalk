
#include <stdio.h>
#include <assert.h>

#include "piece_name.h"

#include <string.h>

char *piece_names[] = { "Knight", "Bishop", "King", "Rook", "Queen", "Unknown" };

#include "piece_chars.h"

char *piece_name(char c) {

  char *match_seq = piece_chars;

  char *s = match_seq;

  for ( ; *s; s++) {

    if (*s == c) break;

  }

  return piece_names[s - match_seq]; // Good match, or Unknown

}

