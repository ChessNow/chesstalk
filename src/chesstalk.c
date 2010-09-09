#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>

// pgn file output module
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "output_module.h"

// festival_client --async --ttw --aucommand 'aplay $FILE'

int blocking_speak_festival(char *str) {

  FILE *p;

  char *command = "festival_client --async --ttw --aucommand 'aplay --quiet $FILE'";

  int written;

  int exit_retval;

  assert(str!=NULL);

  p = popen(command, "w");
  if (p==NULL) {

    perror("popen");
    fprintf(stderr, "%s: Trouble with call to popen for command=%s\n", __FUNCTION__, command);
    return -1;

  }

  written = fprintf(p, "%s.", str);

  if (fflush(p)) {
    perror("fflush");
    fprintf(stderr, "%s: Trouble with call to fflush.\n", __FUNCTION__);
    return -1;
  }

  if (written != strlen(str)+1) {
    printf("%s: Wrote %d characters of text. Expected %ld.\n", __FUNCTION__, written, strlen(str)+1);
  }

  exit_retval = pclose(p);

  if (exit_retval == -1) {
    perror("pclose");
    return -1;
  }
	 
  return 0;

}

#define PAWN_MOVE 0x1
#define PIECE_MOVE 0x2
#define COORDINATE_MOVE 0x4
#define CASTLE_KS 0x8
#define CASTLE_QS 0x10
#define INVALID 0x20
#define RESIGN 0x40

char *piece_chars = "nbrqk";

int validate_input_move(char *str, int verbose) {

  int len;

  int valid_coordinate(char *cs) {

    assert(cs!=NULL);

    return (cs[0] >= 'a' && cs[0] <= 'h') && 
      (cs[1] >= '1' && cs[1] <= '8');

  }

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

    if (valid_coordinate(str) && valid_coordinate(str+2)) {

      return COORDINATE_MOVE;
      
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

  return INVALID;

}

#define PLAY_WHITE 0x1
#define PLAY_BLACK 0x2
#define GAMEOVER 0x4

char *game_status_str(int game_status) {

  if (game_status==PLAY_WHITE) return "PLAY_WHITE";
  if (game_status==PLAY_BLACK) return "PLAY_BLACK";
  if (game_status==GAMEOVER) return "GAMEOVER";

  return "UNKNOWN";

}

int show_help() {

  printf("help   this help menu.\n");
  printf("show   list out currently played moves.\n");
  printf("save   file way the move list as output.pgn.\n");
  printf("quit   leave the program.\n");

  printf("\n");
  printf("valid moves are in the form e4 nf3 e7e8 and use 0-1 or 1-0 to terminate the game.\n");

  return 0;

}

char *piece_names[] = { "Knight", "Bishop", "King", "Rook", "Queen", "Unknown" };

char *piece_name(char c) {

  char *s;

  switch (c) {
  case 'n': s=piece_names[0]; break;
  case 'b': s=piece_names[1]; break;
  case 'k': s=piece_names[2]; break;
  case 'r': s=piece_names[3]; break;
  case 'q': s=piece_names[4]; break;
  default: s=piece_names[5]; break;
  }

  return s;

}

char *reformulate(char *line) {

  static char extended_description[20];

  assert(line!=NULL);

  if (strchr(piece_chars, line[0]) != NULL) {

    sprintf(extended_description, "%s to %s", piece_name(line[0]), line+1);

  }

  return extended_description;

}

struct move_node {

  int move_number;

  int game_status;

  char move[5];

  struct move_node *previous, *next;

};

int show_moves(struct move_node *movelist) {

  struct move_node *final;

  final = movelist;

  while (final !=NULL && final->next != NULL) { final = final->next; }

  while (final != NULL) {

    if (final->game_status==PLAY_WHITE) {
      printf("%d. %s", final->move_number, final->move);
    }

    else if (final->game_status==PLAY_BLACK) {
      printf(" %s ", final->move);
    }

    final = final->previous;

  }

  putchar('\n');

  return 0;

}

int save_pgn(struct move_node *movelist, char *pgn_filename) {

  struct move_node *final;

  int fd;

  int retval;

  char workstring[20];

  assert(movelist!=NULL);

  assert(pgn_filename!=NULL);

  fd = open(pgn_filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd==-1) {
    fprintf(stderr, "%s: Trouble opening pgn_filename=%s.\n", __FUNCTION__, pgn_filename);
    return -1;
  }

  final = movelist!=NULL ? movelist->next : NULL;

  while (final !=NULL && final->next != NULL) { final = final->next; }

  while (final != NULL) {

    if (final->game_status==PLAY_WHITE) {
      sprintf(workstring, "%d. %s", final->move_number, final->move);
      retval = write(fd, workstring, strlen(workstring));
    }

    else if (final->game_status==PLAY_BLACK) {
      sprintf(workstring, " %s ", final->move);
      retval = write(fd, workstring, strlen(workstring));
    }

    final = final->previous;

  }

  write(fd, "\n", 1);

  retval = close(fd);
  if (retval==-1) {
    perror("close");
    fprintf(stderr, "%s: Trouble closing fd=%d.\n", __FUNCTION__, fd);
    return -1;
  }

  printf("%s: Wrote pgn_filename=%s.\n", __FUNCTION__, pgn_filename); 

  return 0;

}

int save(struct move_node *movelist) {

  char *filename = "output.move_nodes";

  assert(movelist!=NULL);

  save_pgn(movelist, "output.pgn");

  return 0;

}

struct move_node *append_move(struct move_node *movelist, int valid_move, int game_status, int move_number, char *line, int verbose) {

  struct move_node *m = malloc(sizeof(struct move_node));

  int len;

  assert(line!=NULL);

  if (m==NULL) {
    fprintf(stderr, "%s: Trouble with allocation of move_node.\n", __FUNCTION__);
    return m;
  }

  if (verbose) {
    printf("%s: Appending move %s into memory tree.\n", __FUNCTION__, line != NULL ? line : "NULL");
  }

  m->game_status = game_status;

  m->move_number = move_number;

  len = strlen(line);

  if (len>sizeof(m->move)) len = 4;

  strncpy(m->move, line, len);
  m->move[len] = 0;

  if (verbose) {
    printf("%s: Inserted move %s into memory tree.\n", __FUNCTION__, m->move);
  }

  m->previous = NULL;

  if (movelist!=NULL) {

    assert(movelist->previous == NULL);
    
    m->next = movelist;

    movelist->previous = m;

  }

  else {

    // we are the root handle.
    
    m->next = NULL;

  }

  return m;

}

int main(int argc, char *argv[]) {

  char *line = NULL;
  size_t len = 0;

  int move_number = 1;

  int game_status = PLAY_WHITE;

  char *start_string = "Starting chess game. What is your first move?";

  char *invalid_move = "Invalid move, please try again.";

  char *game_complete = "Game complete. Goodbye.";

  ssize_t read_characters;

  char *env_debug = getenv("DEBUG");

  int debug = env_debug != NULL ? strtol(env_debug, NULL, 10) : 0;

  struct move_node *movelist = NULL;

  blocking_speak_festival(start_string);

  printf("game_status=%s\n", game_status_str(game_status));

  do {

    printf("[%d%s]: ", move_number, game_status==PLAY_WHITE ? "" : "...");

    read_characters = getline(&line, &len, stdin);
  if (line!=NULL) {

    if (len>0) {

      int valid_move;

      if (debug) printf("%s: Got len=%ld for line %s.\n", __FUNCTION__, len, line);
      { char *p; for (p=line; *p; p++) if (*p == '\n') *p = 0; }

      if (len>=4) {

	if (!strncmp(line, "help", 4)) { show_help(); continue; }
	if (!strncmp(line, "show", 4)) { show_moves(movelist); continue; }
	if (!strncmp(line, "save", 4)) { save(movelist); continue; }
	if (!strncmp(line, "quit", 4)) { break; }

      }

      valid_move = validate_input_move(line, debug>2);

      if (valid_move != INVALID) {

	movelist = append_move(movelist, valid_move, game_status, move_number, line, debug>2);

      }

      switch (valid_move) {
      case INVALID: blocking_speak_festival(invalid_move); break;
      case RESIGN: blocking_speak_festival(game_complete); game_status = GAMEOVER; break;
      case CASTLE_KS: blocking_speak_festival("King-side Castle"); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case CASTLE_QS: blocking_speak_festival("Queen-side Castle"); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case PIECE_MOVE: blocking_speak_festival(reformulate(line)); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case COORDINATE_MOVE: blocking_speak_festival(line); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case PAWN_MOVE: blocking_speak_festival(line); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      default: blocking_speak_festival("What did you say?"); break;
      }

      if (valid_move!=INVALID && valid_move != RESIGN) {
	if (game_status==PLAY_WHITE) {
	  move_number++;
	}
      }

    }

  }

  printf("game_status=%s\n", game_status_str(game_status));

  } while (game_status != GAMEOVER);

  if (line) free(line);

  printf("Leaving.\n");

  return 0;

}
