#ifndef BOARD_H
#define BOARD_H

#include <string_view>

namespace Dagor::Board {

/// @brief The width of a chess board, that is `8`.
inline constexpr int width{8};
/// @brief The number of squares of a chess board, that is `64`.
inline constexpr int size{width * width};

/// @brief Computes the rank (i. e. row) of a square from its index.
/// @param square the index of the square.
/// @return its rank.
constexpr int file(int square) { return square % width; }

/// @brief Computes the file (i. e. column) of a square from its index
/// @param square the index of a square.
/// @return its file.
constexpr int rank(int square) { return square / width; }

/// @brief Computes the index of a square from its file and rank.
/// @param file file (i. e. column)
/// @param rank rank (i. e. row)
/// @return the index of the specified square.
constexpr int index(int file, int rank) { return file + width * rank; }

/// @param file the numeric value of a file (i. e. column) form {0,...,7}.
/// @return the name of that file in algebraic chess notation from {a,...,h}.
constexpr char file_name(int file) { return static_cast<char>('a' + file); }

/// @brief The offsets to add to a square index to go in the intended direction.
enum CompassOffsets {
  north_west = +7,
  north = +8,
  north_east = +9,
  west = -1,
  east = 1,
  south_west = -9,
  south = -8,
  south_east = -7
};

/// @brief The names of the squares in algebraic notation. The indices of
/// squares are counted sequentially with `a1` being equal to `0` and `h8` being
/// equal to `63`. A special `no_square` constant denotes an absent value.
enum Square {
  a1,
  b1,
  c1,
  d1,
  e1,
  f1,
  g1,
  h1,
  a2,
  b2,
  c2,
  d2,
  e2,
  f2,
  g2,
  h2,
  a3,
  b3,
  c3,
  d3,
  e3,
  f3,
  g3,
  h3,
  a4,
  b4,
  c4,
  d4,
  e4,
  f4,
  g4,
  h4,
  a5,
  b5,
  c5,
  d5,
  e5,
  f5,
  g5,
  h5,
  a6,
  b6,
  c6,
  d6,
  e6,
  f6,
  g6,
  h6,
  a7,
  b7,
  c7,
  d7,
  e7,
  f7,
  g7,
  h7,
  a8,
  b8,
  c8,
  d8,
  e8,
  f8,
  g8,
  h8,
  no_square
};

}  // namespace Dagor::Board

#endif