#ifndef BOARD_H
#define BOARD_H

#include <array>

#include "bitboard.h"
#include "types.h"

namespace Dagor {

class Board {
 private:
  std::array<BitBoards::BitBoard, Piece::all.size()> pieces;
  std::array<BitBoards::BitBoard, Color::size> colors;
  std::array<Piece::t, Square::size> mailbox;

 public:
  Board() : pieces{}, colors{}, mailbox{Piece::empty} {}

  Piece::t getPiece(Square::t square) const { return mailbox[square]; }
  Color::t getColor(Square::t square) const {
    if (colors[Color::white].isSet(square)) {
      return Color::white;
    } else {
      return Color::black;
    }
  }

  BitBoards::BitBoard forColor(Color::t color) const { return colors[color]; }
  BitBoards::BitBoard forPiece(Piece::t piece) const { return pieces[piece]; }
  BitBoards::BitBoard occupancy() const {
    return colors[Color::white] | colors[Color::black];
  }

  void unset(Square::t square) {
    Piece::t piece = getPiece(square);
    pieces[piece].unsetSquare(square);
    colors[Color::white].unsetSquare(square);
    colors[Color::black].unsetSquare(square);
  }

  void set(Square::t square, Piece::t piece, Color::t color) {
    pieces[piece].setSquare(square);
    colors[color].setSquare(square);
  }
};

}  // namespace Dagor

#endif