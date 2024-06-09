#include "bitboard.h"

#include <ios>

namespace Dagor::BitBoards {

std::ostream &operator<<(std::ostream &out, const BitBoard &board) {
  for (auto rank : Coord::reverseRanks) {
    out << (rank + 1) << " | ";
    for (auto file : Coord::files) {
      out << (board.isSet(Square::index(file, rank)) ? '@' : '.') << ' ';
    }
    out << '\n';
  }
  out << "    ";
  for (auto _ : Coord::files) out << "--";
  out << "\t as decimal: " << std::dec << board.asUint();
  out << "\n    ";
  for (auto file : Coord::files) out << Coord::fileName(file) << ' ';
  out << "\t as hex:     0x" << std::hex << board.asUint() << std::dec
      << std::endl;

  return out;
}

}  // namespace Dagor::BitBoards
