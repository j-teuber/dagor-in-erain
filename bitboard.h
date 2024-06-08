
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <iterator>
#include <ostream>

#include "board.h"

namespace Dagor::BitBoards {

static_assert(sizeof(std::uint64_t) == 8,
              "For its BitBoards, this program assumes 64 bit integers.");

/// @brief BitBoards represent some subset of the chess boards pieces,
/// such as the set of all fields occupied by whit pawns or the
/// set of all pieces to which a given piece can move on it’s next
/// turn etc.
class BitBoard {
 private:
  std::uint64_t board;

 public:
  /// @brief constructs an empty BitBoard.
  BitBoard();
  /// @brief
  /// @param bitboard a uint64 as returned by the `as_uint` function.
  BitBoard(std::uint64_t bitboard);

  /// @brief
  /// @return a uint64 where all the 1 bits indicate the set squares
  constexpr std::uint64_t as_uint() const { return board; }

  /// @brief removes all the squares that are not also present in `other`.
  /// @param other
  /// @return this bitboard after the modification
  BitBoard &operator&=(BitBoard other) {
    board &= other.board;
    return *this;
  }

  /// @brief adds all the squares of the `other` bitboard to this one.
  /// @param other
  /// @return this bitboard after the modification
  BitBoard &operator|=(BitBoard other) {
    board |= other.board;
    return *this;
  }

  /// @brief Constructs a bitboard with only a single square set.
  /// @param square the square to be set
  /// @return the bitboard
  static inline BitBoard single_square_set(int square) {
    return {static_cast<std::uint64_t>(1) << square};
  }

  /// @brief checks whether the bitboard is empty, that is whether no squares
  /// are set.
  /// @return `true`, iff no squares are set.
  constexpr bool is_empty() const { return board == 0; }

  /// @brief Checks whether a particular square is set.
  /// @param square the square to check.
  /// @return `true`, iff the square is set.
  constexpr bool is_set(int square) const {
    return (board & (static_cast<std::uint64_t>(1) << square)) != 0;
  }

  /// @brief Adds the given square to the bitboard.
  /// @param square the square to add.
  void set_bit(int square) { *this |= single_square_set(square); }

  /// Adds the given square to the bitboard, if the coordinates are
  /// valid on a chess board, that is, if `file, rank are from {0,...,7}`. If
  /// this is not the case, nothing happens. This function exists to protect
  /// against warping around the edges of the board when calculating moves etc.
  /// @param file the file (i. e. column) of the square to add.
  /// @param rank the rank (i. e. row) of the square to add.
  void set_bit_if_index_valid(int file, int rank) {
    if (0 <= file && file < Board::width && 0 <= rank && rank < Board::width) {
      set_bit(Board::index(file, rank));
    }
  }

  /// @brief Removes a given square from the bitboard.
  /// @param square the square to remove.
  void unset_bit(int square) { board &= ~single_square_set(square).board; }

  /// @brief Counts the number of set squares in the bitboard.
  /// @return the number of set squares in the bitboard.
  constexpr int popcount() const { return __builtin_popcountll(board); }

  /// @brief Finds the index of the first set square in the bitboard.
  /// Do not call this function for the empty bitboard.
  /// @return the index of the first set square.
  constexpr int findFirstSet() const { return __builtin_ctzll(board); }
  struct Iterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::uint8_t;
    using pointer = value_type *;
    using reference = value_type &;

    Iterator(std::uint64_t board, std::uint8_t index)
        : board{board}, index{index} {
      advance();
    }
    value_type operator*() const { return index; }
    Iterator &operator++() {
      index++;
      advance();
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    bool operator==(const Iterator &other) const {
      return board == other.board && index == other.index;
    }
    bool operator!=(const Iterator &other) const { return !(*this == other); }

   private:
    std::uint64_t board;
    std::uint8_t index;
    void advance() {
      std::uint64_t lower = (1UL << index) - 1;
      board &= ~lower;
      if (board == 0 || index >= Board::size) {
        board = 0;
        index = Board::size;
      } else {
        index = __builtin_ctzll(board);
      }
    }
  };

  using const_iterator = Iterator;
  Iterator begin() { return {as_uint(), 0}; }
  Iterator end() { return {as_uint(), Board::size}; }
};

inline BitBoard operator&(BitBoard a, BitBoard b) { return a &= b; }
inline BitBoard operator|(BitBoard a, BitBoard b) { return a |= b; }
inline BitBoard operator~(BitBoard a) { return BitBoard(~a.as_uint()); }

inline bool operator==(BitBoard a, BitBoard b) {
  return a.as_uint() == b.as_uint();
}
inline bool operator!=(BitBoard a, BitBoard b) {
  return a.as_uint() != b.as_uint();
}

std::ostream &operator<<(std::ostream &out, const BitBoard &printer);

/// @brief A bitboard containing all squares adjacent to one of the edges of
/// the board.
///
///     8 | @ @ @ @ @ @ @ @
///     7 | @ . . . . . . @
///     6 | @ . . . . . . @
///     5 | @ . . . . . . @
///     4 | @ . . . . . . @
///     3 | @ . . . . . . @
///     2 | @ . . . . . . @
///     1 | @ @ @ @ @ @ @ @
///         ----------------     as decimal: 18411139144890810879
///         a b c d e f g h      as hex:     0xff818181818181ff
inline const BitBoard edgesOnly{0xff818181818181ff};

}  // namespace Dagor::BitBoards

#endif