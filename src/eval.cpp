#include "eval.h"

#include <array>

#include "types.h"

namespace Dagor::Eval {

constexpr int centiPawns(Piece::t piece) {
  switch (piece) {
    case Piece::pawn:
      return 100;
    case Piece::knight:
      return 325;
    case Piece::bishop:
      return 350;
    case Piece::rook:
      return 500;
    case Piece::queen:
      return 900;

    default:
      return 0;
  }
}

int eval(const GameState& state) {
  int result = 0;

  for (Piece::t piece : Piece::nonKing) {
    int diff = state.bitboardFor(piece, state.us()).populationCount() -
               state.bitboardFor(piece, state.them()).populationCount();
    result += diff * centiPawns(piece);
  }

  /*for (Piece::t piece : Piece::nonKing) {
    BitBoards::BitBoard center{0x3c3c3c3c0000};
    result +=
        (state.bitboardFor(piece, state.us()) & center).populationCount() *
        centiPawns(piece) / 10;
  }*/

  return result;
}

}  // namespace Dagor::Eval