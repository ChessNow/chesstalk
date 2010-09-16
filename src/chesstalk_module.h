#ifndef CHESSTALK_MODULE_H
#define CHESSTALK_MODULE_H

struct chesstalk_module {

  int (*move_submission)(int move_number, char *move_string, int white_move);
  int (*module_shutdown)();

};

#endif
