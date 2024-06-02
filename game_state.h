#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <array>
#include <string_view>

#include "bitboard.h"
#include "constants.h"

namespace Dagor {

class Move {
 public:
  std::uint8_t start;
  std::uint8_t end;
  std::uint8_t promotion;
  std::uint8_t flags;

  Move(std::uint8_t start, std::uint8_t end, std::uint8_t promotion)
      : start{start}, end{end}, promotion{promotion}, flags{0} {}
};

class GameState {
 public:
  static inline const std::string_view startingPosition =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

  std::array<BitBoards::BitBoard, Piece::noPieces> pieces;
  std::array<BitBoards::BitBoard, Color::noColors> colors;
  std::size_t moveCounter;
  std::uint8_t castlingRights;
  std::uint8_t uneventfulHalfMoves;
  std::uint8_t enPassantSquare;
  bool isWhiteNext;

  GameState()
      : pieces(),
        colors(),
        moveCounter{0},
        castlingRights{CastlingRights::all},
        uneventfulHalfMoves{0},
        enPassantSquare{Board::Square::no_square},
        isWhiteNext{true} {
    parseFenString(startingPosition);
  }

  BitBoards::BitBoard bitboardFor(unsigned piece, unsigned color) {
    return pieces[piece] & colors[color];
  }

  unsigned getPiece(int square) const {
    for (unsigned type = 0; type < Piece::noPieces; type++) {
      if (pieces[type].is_set(square)) {
        return type;
      }
    }
    return Piece::noPieces;
  }

  unsigned getColor(int square) const {
    if (colors[Color::black].is_set(square)) return Color::black;
    if (colors[Color::white].is_set(square)) return Color::white;
    return Color::noColors;
  }

  void executeMove(Move move);
  void parseFenString(std::string_view fenString);
};

std::ostream &operator<<(std::ostream &out, const GameState &board);

}  // namespace Dagor

#endif
