
#include <stdio.h>
#include <assert.h>

#include "move_spec.h"

int valid_coordinate(char *cs) {

  assert(cs!=NULL);

  return (cs[0] >= 'a' && cs[0] <= 'h') && 
    (cs[1] >= '1' && cs[1] <= '8');

}



