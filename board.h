#ifndef BOARD_H
#define BOARD_H

#include <string_view>

#include "int_types.h"

namespace Dagor::Board {

inline constexpr int width{8};
inline constexpr int size{width * width};

constexpr int file(int square) { return square % width; }
constexpr int rank(int square) { return square / width; }
constexpr int index(int file, int rank) { return file + width * rank; }

constexpr char file_name(int file) { return static_cast<char>('a' + file); }

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
  h8
};

}  // namespace Dagor::Board

#endif