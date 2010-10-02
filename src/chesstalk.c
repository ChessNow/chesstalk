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

#include <dlfcn.h>

// festival_client --async --ttw --aucommand 'aplay $FILE'

char *basic_command = "festival_client --async --ttw --aucommand 'sox $FILE $FILE.sox.wav bass +2 rate 48k gain -3 pad 0 3 reverb channels 2 ; aplay --quiet $FILE.sox.wav'";

char *env_festival_prolog;

int blocking_speak_festival(char *str, char *command) {

  FILE *p;

  int written;

  int exit_retval;

  assert(str!=NULL && command!=NULL);

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
    printf("%s: Wrote %d characters of text. Expected %lu.\n", __FUNCTION__, written, strlen(str)+1);
  }

  exit_retval = pclose(p);

  if (exit_retval == -1) {
    perror("pclose");
    return -1;
  }
	 
  return 0;

}

int regular_festival(char *str) {

  return blocking_speak_festival(str, basic_command);

}

int prolog_prep_festival(char *str) {
  
  char command[2048];

  char *command_template = "festival_client --withlisp --prolog %s --async --ttw --aucommand 'sox $FILE $FILE.sox.wav bass +2 rate 48k gain -3 pad 0 3 reverb channels 2 ; aplay --quiet $FILE.sox.wav'";

  char *prolog_filename = env_festival_prolog; 

  assert(prolog_filename!=NULL);

  sprintf(command, command_template, prolog_filename);

  return blocking_speak_festival(str, command);

}

#include "validate_input_move.h"

#include "move_spec.h"

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
  printf("save   file away the move list as output.pgn.\n");
  printf("quit   leave the program.\n");

  printf("\n");
  printf("valid moves are in the form e4 Nf3 [Rab4 R1c7 Na1b3] cxd4 Nxd4 Bcxd4 [B7xc6 Bb8xc7] and use 0-1 or 1-0 to terminate the game. Castle with O-O or O-O-O for king or queen side.\n");

  return 0;

}

#include "piece_name.h"

char *piece_reformulate(char *line, char *extended_description) {

  int len;

  void uncharacterized_encoding() {

      // Uncharacterized exchange.
      sprintf(extended_description, "Move was %s.\n", line);

  }

  assert(line!=NULL);

  len = strlen(line);

  printf("%s: Using line len=%d\n", __FUNCTION__, len);

  if (strchr(piece_chars, line[0]) != NULL) {

    if (line[1] >= 'a' && line[1] <= 'h') {

	if (len == 4 && valid_coordinate(line+2)) {

	  // Nab2

	  sprintf(extended_description, "%s %c to %s", piece_name(line[0]), line[1], line+2);

	}

      else

	if (valid_coordinate(line+1)) {

	  switch(len) {

	  case 3:

	  // Nc3

	  sprintf(extended_description, "%s to %s", piece_name(line[0]), line+1);

	  break;

	  case 5:

	    if (valid_coordinate(line+3)) {

	    // Na1c2

	    sprintf(extended_description, "%s %.2s to %s", piece_name(line[0]), line+1, line+3);

	    }

	    break;

	  default: uncharacterized_encoding();

	  }

	}

    }

    else

      if (line[1] >= '1' && line[1] <= '8') {

	if (valid_coordinate(line+2)) {

	  assert(len==4);

	  // N2c4

	  sprintf(extended_description, "%s %c to %s", piece_name(line[0]), line[1], line+2);

	}

      }

      else     uncharacterized_encoding();

  }

  else uncharacterized_encoding();

  return extended_description;

}

char *exchange_reformulate(char *line, char *extended_description) {

  void uncharacterized_encoding() {

      // Uncharacterized exchange.
      sprintf(extended_description, "Move was %s.\n", line);

  }

  assert(line!=NULL);

  assert((line[0] >= 'a' && line[0] <= 'h') || (strchr(piece_chars, line[0]) != NULL));

  if (line[0] >= 'a' && line[0] <= 'h' && line[1] == 'x') {
    sprintf(extended_description, "%c exchange with %s", line[0], line+2);
  }
  else

    if (strchr(piece_chars, line[0]) != NULL) {

      if (line[1] == 'x') {
	// Nxc3
	assert(valid_coordinate(line+2));
	sprintf(extended_description, "%s exchange with %s", piece_name(line[0]), line+2);
      }

      else

	if (line[2] == 'x') {
	  // B7xd6 or Bcxd4
	  assert(valid_coordinate(line+3));
	  sprintf(extended_description, "%s %c  exchange with %s", piece_name(line[0]), line[1], line+3);
	}
      
	else 
	  
	  if (line[3] == 'x') {
	    // Be4xf5
	    assert(valid_coordinate(line+4));
	    sprintf(extended_description, "%s %.2s exchange with %s", piece_name(line[0]), line+1, line+4);
	  }

	  else uncharacterized_encoding();

    }

    else {

      uncharacterized_encoding();

    }

  return extended_description;

}

struct move_node {

  int move_number;

  int game_status;

  char move[6];

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

  off_t length = 0;

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
      if (retval>0) length += retval;
    }

    else if (final->game_status==PLAY_BLACK) {
      sprintf(workstring, " %s ", final->move);
      retval = write(fd, workstring, strlen(workstring));
      if (retval>0) length += retval;
    }

    final = final->previous;

  }

  retval = write(fd, "\n", 1);
  if (retval>0) length += retval;

  retval = ftruncate(fd, length);
  if (retval==-1) {
    perror("ftruncate");
    close(fd);
    return -1;
  }

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

  if (movelist==NULL) {
    printf("%s: Empty move list, not saving.\n", __FUNCTION__);
    return -1;
  }

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

#include "chesstalk_module.h"

struct module_list {

  void *handle;

  struct chesstalk_module *m;
  
  struct module_list *next, *prev;

};

// prepends a new module otherwise returns prior list on failure.

struct module_list *load_modules(struct module_list *current_list, char *module_names) {

  struct module_list *p;

  struct chesstalk_module *m;

  char *filename;

  int (*module_entry)(struct chesstalk_module *m);

  void *handle;

  int retval;

  char *error;

#define free_leave() { free(p); free(m); return current_list; }

  assert(module_names!=NULL);

  p = malloc(sizeof(struct module_list));
  if (p==NULL) return current_list;

  m = malloc(sizeof(struct chesstalk_module));
  if (m==NULL) {
    free(p);
    return current_list;
  }

  filename = module_names;

  handle = dlopen(filename, RTLD_LAZY);
  if (handle==NULL) {
    printf("%s: dlopen failure for %s.\n", __FUNCTION__, filename);
    free_leave();
  }

  dlerror();

  module_entry = dlsym(handle, "module_entry");
  if (module_entry==NULL) {
    error=dlerror();
    if (error!=NULL) { printf("%s: Trouble with call to dlsym. error=%s\n", __FUNCTION__, error); }
    dlclose(handle);
    free_leave();
  }

  retval = module_entry(m);
  if (retval==-1) {
    printf("%s: Problem calling module_entry for %s.\n", __FUNCTION__, filename);
    dlclose(handle);
    free_leave();
  }

  p->handle = handle;

  p->m = m;

  p->next = current_list;
  p->prev = NULL;

  current_list = p;

#undef free_leave

  return current_list;

}

int unload_modules(struct module_list *p) {

  int count = 0, success = 0;

  int retval;

  for ( ; p != NULL; p = p->next, count++) {

    if (p->handle!=NULL) {

      struct chesstalk_module *m = p->m;

      if (m!=NULL && m->module_shutdown!=NULL) {

	retval = m->module_shutdown();
	if (retval==-1) {
	  printf("%s: Warning. Unclean shutdown for module m=%p.\n", __FUNCTION__, m);
	}

      }
      
      if (0 != dlclose(p->handle)) {

	printf("%s: Trouble closing handle=%p.\n", __FUNCTION__, p->handle);

      }

      else success++;

    }

  }

  if (p!=NULL) return -1;

  return (count - success);

}

int modules_work(struct module_list *p, int move_number, char *move_string, int white_move) {

  struct chesstalk_module *m;

  int retval;

  for ( ; p != NULL; p = p->next) {

    m = p->m;

    if (m != NULL && m->move_submission!=NULL) {
      retval = m->move_submission(move_number, move_string, white_move);
      if (retval!=0) {
	printf("%s: Trouble with call to m->move_submission=%p.\n", __FUNCTION__, m->move_submission);
      }
    }

  }

  return 0;

}

int show_modules(struct module_list *modules) {

  struct module_list *p = modules;

  int count;

  for ( count = 0; p != NULL; p = p->next, count++) {

    if (p!=NULL) {
      struct chesstalk_module *m = p->m;
      printf("[%d]: m=%p\n", count, m);
      if (m!=NULL) {
	printf("[%d]: m->move_submission=%p\n", count, m->move_submission);
	printf("[%d]: m->module_shutdown=%p\n", count, m->module_shutdown);
      }

    }

  }

  return 0;

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

  char *module_names = getenv("CHESSTALK_MODULE");

  struct module_list *modules = module_names != NULL ? load_modules(NULL, module_names) : NULL;

  int retval;

  int (*block_fest)(char *);

  char extended_description[80];

  env_festival_prolog = getenv("FESTIVAL_PROLOG");

  block_fest = env_festival_prolog != NULL ? prolog_prep_festival : regular_festival;

  block_fest(start_string);

  printf("game_status=%s\n", game_status_str(game_status));

  do {

    printf("[%d%s]: ", move_number, game_status==PLAY_WHITE ? "" : "...");

    read_characters = getline(&line, &len, stdin);

    if (read_characters == -1) { putchar('\r'); continue; }

  if (line!=NULL) {

    if (len>0) {

      int valid_move;

      if (debug) printf("%s: Got len=%lu for line %s.\n", __FUNCTION__, len, line);
      { char *p; for (p=line; *p; p++) if (*p == '\n') *p = 0; }

      if (len>=4) {

	if (!strncmp(line, "help", 4)) { show_help(); continue; }
	if (!strncmp(line, "show", 4)) { show_moves(movelist); continue; }
	if (!strncmp(line, "modules", 7)) { show_modules(modules); continue; }
	if (!strncmp(line, "save", 4)) { save(movelist); continue; }
	if (!strncmp(line, "quit", 4)) { break; }

      }

      valid_move = validate_input_move(line, debug>2);

      if (valid_move != INVALID) {

	movelist = append_move(movelist, valid_move, game_status, move_number, line, debug>2);

	if (modules!=NULL) { modules_work(modules, move_number, line, game_status & PLAY_WHITE); }

      }

      switch (valid_move) {
      case INVALID: block_fest(invalid_move); break;
      case RESIGN: block_fest(game_complete); game_status = GAMEOVER; break;
      case CASTLE_KS: block_fest("King-side Castle"); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case CASTLE_QS: block_fest("Queen-side Castle"); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case PIECE_MOVE: block_fest(piece_reformulate(line, extended_description)); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case COORDINATE_MOVE: block_fest(line); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case FALLOUT_EXCHANGE: block_fest(exchange_reformulate(line, extended_description)); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      case PAWN_MOVE: block_fest(line); game_status ^= (PLAY_WHITE | PLAY_BLACK); break;
      default: block_fest("What did you say?"); break;
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

  if (modules!=NULL) {
    retval = unload_modules(modules);
    if (retval==-1) {
      printf("%s: Trouble unloading one or more modules.\n", __FUNCTION__);
    }
  }

  if (line) free(line);

  printf("Leaving.\n");

  return 0;

}
