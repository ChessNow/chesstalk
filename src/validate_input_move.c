#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "move_spec.h"

int validate_input_move(char *str, int verbose) {

  int len;

  assert(str!=NULL);

  len = strlen(str);

  if (verbose) {
    printf("%s: len=%d\n", __FUNCTION__, len);
  }

  if ((len==3 || len==5) && str[0]=='O') {

    if (str[1]=='-' && str[2] == 'O') {

      if (str[3] == 0) return CASTLE_KS;

      if (len==5 && str[3]=='-' && str[4]=='O' && str[5]==0) return CASTLE_QS;

    }

  }

  else

    if (len==4) {

      if (str[1] == 'x' && 
	  ((str[0] >= 'a' && str[0] <= 'h') || (strchr(piece_chars, str[0]) != NULL))) {

	// exchange

	if (valid_coordinate(str+2)) {
	  return FALLOUT_EXCHANGE;
	}

      }

    }
  
    else

      if (len==3 && strchr(piece_chars, str[0]) != NULL) {

	// piece move

	if (valid_coordinate(str+1)) return PIECE_MOVE;

      }

      else

	if (len==3 && str[1] == '-') {
	    
	  // resignation

	  if ((str[0] == '0' && str[2] == '1')
	      || (str[0] == '1' && str[2] == '0')) {
	    return RESIGN;
	  }
      
	}

	else

	  if (len==2) {
	      
	    // pawn move
	      
	    if (valid_coordinate(str)) return PAWN_MOVE;
	      
	  }

	  else 

	    if (len==5) {

	      if (strchr(piece_chars, str[0]) != NULL) {

		if (str[1] >= 'a' && str[1] <= 'h') {

		  if (str[2] == 'x')

		    if (valid_coordinate(str+3)) return FALLOUT_EXCHANGE;

		}

		if (str[1] >= '1' && str[1] <= '8') {

		  if (str[2] == 'x')

		    if (valid_coordinate(str+3)) return FALLOUT_EXCHANGE;

		}

	      }

	    }

	    else if (len==6) {

	      if (strchr(piece_chars, str[0]) != NULL) {

		if (valid_coordinate(str+1)) {

		  if (str[3] == 'x')

		    if (valid_coordinate(str+4)) return FALLOUT_EXCHANGE;

		}

	      }

	    }

  return INVALID;

}

