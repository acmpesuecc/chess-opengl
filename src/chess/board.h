#if !defined(BOARD_H)
#define BOARD_H

#include <stdbool.h>

#include "../types.h"
#include "piece.h"

// forward declaration to avoid circular dependency
struct ChessMove;

typedef struct
{
    bool white_king_side : 1;
    bool white_queen_side : 1;
    bool black_king_side : 1;
    bool black_queen_side : 1;
} CastlingRights;

typedef struct
{
    ChessPiece *squares[8][8];
    struct ChessMove *last_move;
    CastlingRights castling_rights;

    Vec2i white_king_pos;
    Vec2i black_king_pos;
    ChessColor turn;
} ChessBoard;

void chess_board_init(ChessBoard *self);
void chess_board_make_move(ChessBoard *self, ChessPiece *piece, struct ChessMove *move);
void chess_board_promote_pawn(ChessBoard *self, ChessPiece *pawn, struct ChessMove *move,
                              PieceType promoted_type);
void chess_board_undo_last_move(ChessBoard *self, struct ChessMove *prev_last_move);
void chess_board_from_fen(ChessBoard *self, const char *fen);
bool chess_board_is_square_attacked(ChessBoard *self, Vec2i square, ChessColor color);
bool chess_board_does_side_have_legal_moves(ChessBoard *self, ChessColor color);
bool chess_board_is_in_check(ChessBoard *self, ChessColor color);
bool chess_board_is_in_checkmate(ChessBoard *self, ChessColor color, bool do_check_detection);
bool chess_board_is_in_stalemate(ChessBoard *self, ChessColor color, bool do_check_detection);

#endif
