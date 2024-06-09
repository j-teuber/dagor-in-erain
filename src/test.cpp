#include "test.h"

#include <iostream>

#include "bitboard.h"
#include "game_state.h"
#include "types.h"

namespace Dagor::Test {

static unsigned tests = 0;
static unsigned failures = 0;

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
  out << "[";
  for (auto e : v) {
    out << e << ", ";
  }
  out << "]\n";
  return out;
}

template <typename T>
void assertEquals(T actual, T expected, std::string_view name) {
  tests++;
  std::cout << name << "... ";
  if (actual == expected) {
    std::cout << "\033[1;32mCheck!\033[0m\n";
  } else {
    std::cout << "\033[1;31mFail!\033[0m\n";
    std::cout << "expected:\n" << expected << "\nbut got:\n" << actual << '\n';
    failures++;
  }
}

void header(std::string_view name) {
  std::cout << "\n\033[1;34m" << name << "\033[0m\n";
}

void bitBoards() {
  header("BitBoards");

  BitBoards::BitBoard b{0xc0000000000e1805};
  assertEquals(static_cast<int>(*(++(++b.begin()))), 11,
               "First element of iterator");
  std::vector<int> expected{0, 2, 11, 12, 17, 18, 19, 62, 63};
  std::vector<int> squares{};
  for (auto square : b) {
    squares.push_back(square);
  }
  assertEquals(squares, expected,
               "BitBoards can iterate through their set bit");
}

void pseudoLegalMoves() {
  header("Pseudo-Legal Move Generation");
  assertEquals(GameState{"8/8/8/2r1p3/3P4/8/8/8 w - - 0 1"}.getMoves(
                   Piece::pawn, Color::white, Square::d4),
               {0x1c00000000}, "pawn can capture diagonally and move forward");
  assertEquals(GameState{"8/8/8/3q4/3P4/8/8/8 w - - 0 1"}.getMoves(
                   Piece::pawn, Color::white, Square::d4),
               {0}, "pawn can't move diagonally and capture forward");

  assertEquals(GameState{"8/8/8/8/R1n5/8/1nQ5/8 b - - 0 1"}.getMoves(
                   Piece::knight, Color::black, Square::b2),
               {0x1080008},
               "knights aren't blocked and can't capture their own pieces");
  assertEquals(GameState{"8/r5N1/5r2/8/3b4/2R5/8/Q5N1 b - - 0 1"}.getMoves(
                   Piece::bishop, Color::black, Square::d4),
               {0x21400142040},
               "bishops are blocked and can't capture their own pieces");
  assertEquals(GameState{"8/3N4/8/3p4/B2r1R1Q/8/8/3b4 b - - 0 1"}.getMoves(
                   Piece::rook, Color::black, Square::d4),
               {0x37080800},
               "rooks are blocked and can't capture their own pieces");
  assertEquals(
      GameState{"3R4/6r1/1B6/4r3/b2Qb2q/3N4/1r3R2/3q2n1 w - - 0 1"}.getMoves(
          Piece::queen, Color::white, Square::d4),
      {0x8081c17140200},
      "queens are blocked and can't capture their own pieces");
}

void pieceMovement() {
  header("Movement of Single Pieces");
  using namespace MoveTables;
  assertEquals(pawnAttacks[Color::white][Square::c8], {},
               "Pawn in last row cannot move further");
  assertEquals(pawnAttacks[Color::white][Square::c3], {0xa000000},
               "Pawn in the center can attack left and right");
  assertEquals(pawnAttacks[Color::white][Square::a3], {0x2000000},
               "Pawn in the left side has only one attack");

  assertEquals(knightMoves[Square::d5], {0x14220022140000},
               "Knight in the center has the correct moves");
  assertEquals(knightMoves[Square::a1], {0x20400},
               "Knight in a corner has only two options");

  assertEquals(kingMoves[Square::b2], {0x70507}, "King has eight moves");
  assertEquals(kingMoves[Square::a1], {0x302},
               "King in a corner has only three options");

  assertEquals(bishopHashes[Square::c4].lookUp({}), {0x4020110a000a1120},
               "Unobstructed Bishop moves");
  assertEquals(bishopHashes[Square::c4].lookUp({0x840010504008018a}),
               {0x110a000a0100}, "Bishop with blocking pieces");

  assertEquals(rookHashes[Square::c4].lookUp({}), {0x4040404fb040404},
               "Unobstructed Rook moves");
  assertEquals(rookHashes[Square::c4].lookUp({0x2440000940a200}),
               {0x404040b040404}, "Rook with blocking pieces");
}

void moveClass() {
  header("The Move Class");
  assertEquals(Move{"a1a3"}, Move{Square::a1, Square::a3},
               "Moves can be constructed from algebraic notation");
  assertEquals(
      Move{"a2a1r"}, Move{Square::a2, Square::a1, Piece::rook},
      "Moves can be constructed from algebraic notation with promotion");
}

void legalMoves() {
  header("Legal Moves");
  assertEquals(GameState{}.generateLegalMoves().size(),
               static_cast<std::size_t>(20),
               "20 legal moves are available in starting position");
}

void test() {
  header("\nRun Test suits...\n");
  pieceMovement();
  pseudoLegalMoves();
  moveClass();
  bitBoards();
  legalMoves();

  if (failures == 0) {
    std::cout << "\033[1;32m";
  } else {
    std::cout << "\033[1;31m";
  }

  std::cout << "\nTests: " << tests << " (" << (tests - failures) << " passed, "
            << failures << " failed)\n"
            << "\033[0m";
}

}  // namespace Dagor::Test