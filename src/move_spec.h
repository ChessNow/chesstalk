#ifndef MOVE_SPEC_H
#define MOVE_SPEC_H

#define PAWN_MOVE 0x1
#define PIECE_MOVE 0x2
#define COORDINATE_MOVE 0x4
#define CASTLE_KS 0x8
#define CASTLE_QS 0x10
#define FALLOUT_EXCHANGE 0x20
#define INVALID 0x40
#define RESIGN 0x80

int valid_coordinate(char *cs);

#endif
