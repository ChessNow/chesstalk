#include <stdio.h>
#include <assert.h>

#include "validate_input_move.h"

#include "move_spec.h"

#define OK 1
#define FAIL 0

struct test_sample {

  char *string;

  int expected_result;

};

int main(int argc, char *argv[]) {

  struct test_sample ts[] = {
    { "e4", .expected_result = OK }
    , { "a3", .expected_result = OK }
    , { "Nf3", .expected_result = OK }
    , { "Nf6", .expected_result = OK }
    , { "Hg3", .expected_result = FAIL } // bad piece character
    , { "Bh9", .expected_result = FAIL } // coordinate destination out of range
    , { "D3h", .expected_result = FAIL } // coordinate destination is inverted
    , { "cxd4", .expected_result = OK }
    , { "hxg5", .expected_result = OK }
    , { "cx3b", .expected_result = FAIL } // coordinate destination is inverted
    , { "bxx3", .expected_result = FAIL } // extra exchange character
    , { "Bcxd4", .expected_result = OK }
    , { "B7xe6", .expected_result = OK }
    , { "Bd5xe6", .expected_result = OK }    
  }, *s = ts, *e = s + sizeof(ts) / sizeof(struct test_sample);

  int retval;

  int verbose = 1;

  for ( ; s < e; s++) {

    retval = validate_input_move(s->string, verbose);

    if (retval == INVALID && s->expected_result == OK) {

      printf("%s: Expected string=%s to pass but it failed.\n", __FUNCTION__, s->string);

      return -1;

    }

    if (retval != INVALID && s->expected_result == FAIL) {

      printf("%s: That's really bad. Expected string=%s to fail, but it somehow passed!\n", __FUNCTION__, s->string);

      return -1;

    }

  }

  printf("OK\n");

  return 0;

}
