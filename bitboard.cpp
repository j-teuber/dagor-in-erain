#include "bitboard.h"

#include <ios>

#include "board.h"

namespace Dagor::BitBoard {

BitBoard::BitBoard() : board{0} {}
BitBoard::BitBoard(std::uint64_t bitboard) : board{bitboard} {}

std::ostream &operator<<(std::ostream &out, const BitBoard &board) {
  for (int rank = Board::width - 1; rank >= 0; rank--) {
    out << (rank + 1) << " | ";
    for (int file = 0; file < Board::width; file++) {
      out << (board.is_set(Board::index(file, rank)) ? '@' : '.') << ' ';
    }
    out << '\n';
  }
  out << "    ";
  for (int file = 0; file < Board::width; file++) out << "--";
  out << "\t as decimal: " << std::dec << board.board;
  out << "\n    ";
  for (int file = 0; file < Board::width; file++)
    out << Board::file_name(file) << ' ';
  out << "\t as hex:     0x" << std::hex << board.board << std::dec
      << std::endl;

  return out;
}

}  // namespace Dagor::BitBoard
