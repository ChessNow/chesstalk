#ifndef MOVE_SPEC_H
#define MOVE_SPEC_H

#define PAWN_MOVE 0x1
#define PIECE_MOVE 0x2
#define CASTLE_KS 0x4
#define CASTLE_QS 0x8
#define FALLOUT_EXCHANGE 0x10
#define INVALID 0x20
#define RESIGN 0x40

int valid_coordinate(char *cs);

#endif
