
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <ostream>

#include "board.h"

namespace Dagor::BitBoard {

class BitBoard {
 private:
  std::uint64_t board;

 public:
  BitBoard();
  BitBoard(std::uint64_t bitboard);

  BitBoard &operator&=(BitBoard other) {
    board &= other.board;
    return *this;
  }

  BitBoard &operator|=(BitBoard other) {
    board |= other.board;
    return *this;
  }

  static inline BitBoard single_square_set(int square) {
    return BitBoard(static_cast<std::uint64_t>(1) << square);
  }

  constexpr bool is_empty() const { return board == 0; }

  constexpr bool is_set(int square) const {
    return (board & (static_cast<std::uint64_t>(1) << square)) != 0;
  }

  void set_bit(int square) { *this |= single_square_set(square); }

  void set_bit_if_index_valid(int file, int rank) {
    if (0 <= file && file < Board::width && 0 <= rank && rank < Board::width) {
      set_bit(Board::index(file, rank));
    }
  }

  void unset_bit(int square) { board &= ~single_square_set(square).board; }

  friend std::ostream &operator<<(std::ostream &out, const BitBoard &printer);
  friend inline BitBoard operator~(BitBoard a);
};

inline BitBoard operator&(BitBoard a, BitBoard b) { return a &= b; }
inline BitBoard operator|(BitBoard a, BitBoard b) { return a |= b; }
inline BitBoard operator~(BitBoard a) { return BitBoard(~a.board); }

std::ostream &operator<<(std::ostream &out, const BitBoard &printer);

/**
8 | @ @ @ @ @ @ @ @
7 | @ . . . . . . @
6 | @ . . . . . . @
5 | @ . . . . . . @
4 | @ . . . . . . @
3 | @ . . . . . . @
2 | @ . . . . . . @
1 | @ @ @ @ @ @ @ @
    ----------------     as decimal: 18411139144890810879
    a b c d e f g h      as hex:     0xff818181818181ff
*/
inline const BitBoard edgesOnly{0xff818181818181ff};

}  // namespace Dagor::BitBoard

#endif