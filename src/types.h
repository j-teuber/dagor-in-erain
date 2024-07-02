#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <cctype>
#include <cstdint>

namespace Dagor {

namespace Color {
using t = std::uint8_t;
enum { white, black };
constexpr t empty = 255;
constexpr std::size_t size = 2;
constexpr std::array<t, 2> all = {white, black};
constexpr t opponent(t color) { return black - color; }
constexpr t pieceColorFromChar(char name) {
  if (std::isupper(name))
    return Color::white;
  else
    return Color::black;
}
}  // namespace Color

namespace Piece {
using t = std::uint8_t;
enum { pawn, knight, bishop, rook, queen, king, empty };
constexpr t noPiece = 7;
constexpr std::array<t, 6> all = {pawn, knight, bishop, rook, queen, king};
constexpr std::array<t, 3> leapers = {king, pawn, knight};
constexpr std::array<t, 3> sliders = {bishop, rook, queen};
constexpr std::array<t, 5> nonKing = {pawn, knight, bishop, rook, queen};
constexpr std::array<std::int16_t, Piece::all.size()> worth = {100, 325, 350,
                                                               500, 900};
constexpr std::array<char, 7> names = {'p', 'n', 'b', 'r', 'q', 'k', '.'};
constexpr bool inRange(t piece) { return piece < empty; }

constexpr char name(Piece::t type, Color::t color) {
  char c = names[type];
  if (color == Color::white) c = std::toupper(c);
  return c;
}
constexpr t byName(char name) {
  switch (std::tolower(name)) {
    case 'k':
      return king;
    case 'p':
      return pawn;
    case 'n':
      return knight;
    case 'b':
      return bishop;
    case 'r':
      return rook;
    case 'q':
      return queen;
    default:
      return empty;
  }
}
}  // namespace Piece

namespace Coord {
using t = std::int8_t;
constexpr t width = 8;
enum { a, b, c, d, e, f, g, h };
constexpr std::array<t, 8> files = {a, b, c, d, e, f, g, h};
constexpr std::array<t, 8> ranks = {0, 1, 2, 3, 4, 5, 6, 7};
constexpr std::array<t, 8> reverseRanks = {7, 6, 5, 4, 3, 2, 1, 0};
constexpr bool inRange(t coord) { return 0 <= coord && coord < width; }

constexpr t rankByName(char c) { return static_cast<t>(c - '1'); }
constexpr t fileByName(char c) { return static_cast<t>(c - 'a'); }

/// @param file the numeric value of a file (i. e. column) form {0,...,7}.
/// @return the name of that file in algebraic chess notation from {a,...,h}.
constexpr char fileName(t file) { return static_cast<char>('a' + file); }
constexpr char rankName(t rank) { return static_cast<char>('1' + rank); }
}  // namespace Coord

namespace CastlingRights {
using t = std::uint8_t;
enum {
  whiteKingSide = 0b0001,
  whiteQueenSide = 0b0010,
  blackKingSide = 0b0100,
  blackQueenSide = 0b1000,
};
constexpr std::array<t, 4> all = {whiteKingSide, whiteQueenSide, blackKingSide,
                                  blackQueenSide};
constexpr t fullRights =
    whiteKingSide | whiteQueenSide | blackKingSide | blackQueenSide;
constexpr t none = 0;
}  // namespace CastlingRights

namespace MoveFlags {
using t = std::uint8_t;
enum {
  normal = 0,
  whiteKingSide,
  whiteQueenSide,
  blackKingSide,
  blackQueenSide,
  enPassant,
  promotion
};
}  // namespace MoveFlags

namespace Square {
using t = std::int8_t;
constexpr t size = Coord::width * Coord::width;
constexpr bool inRange(t square) { return 0 <= square && square < size; }

constexpr t noSquare = -1;

/// @brief Computes the file (i. e. column) of a square from its index
/// @param square the index of a square.
/// @return its file.
constexpr Coord::t file(t square) { return square % Coord::width; }

/// @brief Computes the rank (i. e. row) of a square from its index.
/// @param square the index of the square.
/// @return its rank.
constexpr Coord::t rank(t square) { return square / Coord::width; }

/// @brief Computes the index of a square from its file and rank.
/// @param file file (i. e. column)
/// @param rank rank (i. e. row)
/// @return the index of the specified square.
constexpr Square::t index(Coord::t file, Coord::t rank) {
  return file + Coord::width * rank;
}

/// @brief Flip the index of a square so that it appears as if the current
/// player plays white.
/// @param forWhite the square that would be correct for the white side
/// @param color the actual color of the current player
/// @return forWhite, if `color` is white, otherwise a flipped index, so that
/// squares in rank 8 become squares in rank 1, rank 7 becoming rank 2, etc.
constexpr Square::t reverseForColor(Square::t forWhite, Color::t color) {
  if (color == Color::white) {
    return forWhite;
  } else {
    return forWhite ^ 56;
  }
}

constexpr t byName(char file, char rank) {
  return index(Coord::fileByName(file), Coord::rankByName(rank));
}
inline std::string name(t square) {
  char fileChar = Coord::fileName(file(square));
  char rankChar = Coord::rankName(rank(square));
  return std::string(1, fileChar) + std::string(1, rankChar);
}

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
enum {
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
constexpr std::array<t, 64> all = {
    a1, b1, c1, d1, e1, f1, g1, h1, a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3, a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5, a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7, a8, b8, c8, d8, e8, f8, g8, h8};
}  // namespace Square

}  // namespace Dagor

#endif