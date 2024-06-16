#include "test.h"

#include <algorithm>
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
  assertEquals(pawnAttacks(Color::white, Square::c8), {},
               "Pawn in last row cannot move further");
  assertEquals(pawnAttacks(Color::white, Square::c3), {0xa000000},
               "Pawn in the center can attack left and right");
  assertEquals(pawnAttacks(Color::white, Square::a3), {0x2000000},
               "Pawn in the left side has only one attack");

  assertEquals(knightMoves(Square::d5), {0x14220022140000},
               "Knight in the center has the correct moves");
  assertEquals(knightMoves(Square::a1), {0x20400},
               "Knight in a corner has only two options");

  assertEquals(kingMoves(Square::b2), {0x70507}, "King has eight moves");
  assertEquals(kingMoves(Square::a1), {0x302},
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

void assertMoveGen(std::string_view fen, std::vector<Move> expected,
                   std::string_view msg) {
  GameState s{std::string(fen)};
  auto moves = s.generateLegalMoves();
  auto key = [](Move& a, Move& b) {
    int a_key = (a.start << 4) + a.end;
    int b_key = (b.start << 4) + b.end;
    return a_key < b_key;
  };
  std::sort(moves.begin(), moves.end(), key);
  std::sort(expected.begin(), expected.end(), key);
  assertEquals(moves, expected, msg);
}

void legalMoves() {
  header("Legal Moves");
  assertEquals(GameState{}.generateLegalMoves().size(),
               static_cast<std::size_t>(20),
               "20 legal moves are available in starting position");

  assertMoveGen("8/8/8/8/8/8/8/K2N2r1 w - - 0 1",
                std::vector{Move{"a1a2"}, Move{"a1b2"}, Move{"a1b1"}},
                "Pinned Knight cannot move");
  assertMoveGen("8/8/8/8/8/k7/8/K1Rr4 w - - 0 1",
                std::vector{Move{"a1b1"}, Move{"c1b1"}, Move{"c1d1"}},
                "Pinned rook can capture opponents rook");
  assertMoveGen("8/8/8/8/8/1qk5/8/K7 w - - 0 1", std::vector<Move>(),
                "no moves for patt");
  assertMoveGen("8/8/8/8/8/2k5/1q6/K7 w - - 0 1", std::vector<Move>(),
                "no moves for check mate");
  assertMoveGen("8/7k/8/8/8/1n2Q3/8/K3r3 w - - 0 1",
                std::vector{Move{"a1a2"}, Move{"a1b2"}},
                "Double check means only the king can move");
  assertMoveGen("8/7k/8/8/8/1nQ5/2n5/K7 w - - 0 1",
                std::vector{Move{"a1a2"}, Move{"a1b2"}, Move{"a1b1"}},
                "Double check is recognized if both checkers are of the same "
                "type (knight)");
  assertMoveGen("8/7k/8/8/8/r1Q5/8/K1r5 w - - 0 1", std::vector{Move{"a1b2"}},
                "Double check is recognized if both checkers are of the same "
                "type (rooks)");
  assertMoveGen("8/8/8/8/4Q3/k7/8/K3r3 w - - 0 1",
                std::vector{Move{"e4b1"}, Move{"e4e1"}},
                "Single check can be solved by capture or interception");
  assertMoveGen("8/8/8/8/8/p3k2p/P6P/R3K2R w KQ - 0 1",
                std::vector{Move{"e1f1"}, Move{"e1d1"}, Move{"e1c1"},
                            Move{"e1g1"}, Move{"a1b1"}, Move{"a1c1"},
                            Move{"a1d1"}, Move{"h1g1"}, Move{"h1f1"}},
                "castling is generated");
  assertMoveGen(
      "8/8/8/8/8/p3k2p/P6P/R3K2R w - - 0 1",
      std::vector{Move{"e1f1"}, Move{"e1d1"}, Move{"a1b1"}, Move{"a1c1"},
                  Move{"a1d1"}, Move{"h1g1"}, Move{"h1f1"}},
      "no castling if we don't have the rights");
  assertMoveGen(
      "8/8/8/8/8/p3k2p/P2r3P/R3K2R w KQ - 0 1",
      std::vector{Move{"e1f1"}, Move{"e1g1"}, Move{"a1b1"}, Move{"a1c1"},
                  Move{"a1d1"}, Move{"h1g1"}, Move{"h1f1"}},
      "no castling if we pass through check");
  assertMoveGen("8/8/8/8/8/p3k2p/P3r2P/R3K2R w KQ - 0 1",
                std::vector{Move{"e1f1"}, Move{"e1d1"}},
                "no castling if we are in check");
  assertMoveGen(
      "8/8/8/6r1/8/p3k2p/P6P/R3K2R w KQ - 0 1",
      std::vector{Move{"e1f1"}, Move{"e1d1"}, Move{"e1c1"}, Move{"a1b1"},
                  Move{"a1c1"}, Move{"a1d1"}, Move{"h1g1"}, Move{"h1f1"}},
      "no castling if we would move into check");
  assertMoveGen("4k3/8/8/3pP3/8/8/2q5/4K3 w - d6 0 1",
                std::vector{Move{"e1f1"}, Move{"e5e6"}, Move{"e5d6"}},
                "Simple en passant capture");
  assertMoveGen("8/8/8/K1pP3q/8/8/8/8 w - c6 0 1",
                std::vector{Move{"d5d6"}, Move{"a5a6"}, Move{"a5b6"},
                            Move{"a5b5"}, Move{"a5a4"}},
                "En passant discovered check");
}

void assertMoveMaker(std::string_view start, std::string_view move,
                     std::string_view end, std::string_view msg) {
  GameState s{std::string{start}};
  GameState e{std::string{end}};
  Move m{std::string{move}};
  s.executeMove(m);
  assertEquals(s, e, std::string{msg} + " (make move)");
  s.undoMove();
  assertEquals(s, GameState{std::string{start}},
               std::string{msg} + " (unmake move)");
}

void makeMove() {
  header("Make Moves");
  assertMoveMaker("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                  "b1c3",
                  "rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/R1BQKBNR b KQkq - 1 0",
                  "Simple Moves");
  assertMoveMaker("rnbqkbnr/pppppppp/8/8/8/1P6/P1PPPPPP/RNBQKBNR w KQkq - 0 1",
                  "b1a3",
                  "rnbqkbnr/pppppppp/8/8/8/NP6/P1PPPPPP/R1BQKBNR b KQkq - 1 1",
                  "Simple Moves");
  assertMoveMaker(
      "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "e4d5",
      "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
      "Simple Captures");
  assertMoveMaker("rnb1kbnr/ppp1pppp/8/3q4/8/8/PPPP1P2/RNBQKBNR b KQkq - 0 1",
                  "d5h1",
                  "rnb1kbnr/ppp1pppp/8/8/8/8/PPPP1P2/RNBQKBNq w Qkq - 0 1",
                  "Capturing a rook removes castling rights");
  assertMoveMaker("rnb1kbnr/8/8/3q4/8/8/8/RNBQKBN1 b Qkq - 0 1", "a8a1",
                  "1nb1kbnr/8/8/3q4/8/8/8/rNBQKBN1 w k - 0 1",
                  "Moving a rook removes castling rights");
  assertMoveMaker("1nb1kbnr/8/8/3q4/8/8/8/rNBQKBN1 b k - 0 1", "e8d7",
                  "1nb2bnr/3k4/8/3q4/8/8/8/rNBQKBN1 w - - 1 1",
                  "Moving a king removes castling rights");
  assertMoveMaker("8/8/8/8/2Pp4/8/8/8 b - c3 0 1", "d4c3",
                  "8/8/8/8/8/2p5/8/8 w - - 0 1", "en passant capture");
  assertMoveMaker("8/8/8/8/8/8/8/R3K3 w Q - 0 1", "e1c1",
                  "8/8/8/8/8/8/8/2KR4 b - - 1 1", "white queen-side castle");
}

void perft(GameState& start, std::vector<std::uint64_t>& results, int depth) {
  auto moves = start.generateLegalMoves();

  results[results.size() - depth] += moves.size();
  if (depth <= 1) {
    return;
  }

  for (Move m : moves) {
    start.executeMove(m);
    perft(start, results, depth - 1);
    start.undoMove();
  }
}

std::uint64_t simplePerft(GameState& start, int depth) {
  if (depth <= 0) {
    return 1;
  }
  auto moves = start.generateLegalMoves();
  std::uint64_t counter = 0;
  for (Move m : moves) {
    start.executeMove(m);
    counter += simplePerft(start, depth - 1);
    start.undoMove();
  }
  return counter;
}

void divide(GameState& start, int depth) {
  auto moves = start.generateLegalMoves();
  std::uint64_t counter = 0;
  for (Move m : moves) {
    std::cerr << m << ": ";
    start.executeMove(m);
    std::uint64_t p = simplePerft(start, depth - 1);
    start.undoMove();
    counter += p;
    std::cerr << p << '\n';
  }
  std::cerr << "Total: " << counter;
}

void assertPerft(std::string_view start, std::vector<std::uint64_t> expected,
                 std::string_view msg) {
  GameState s{std::string{start}};
  std::vector<std::uint64_t> results(expected.size());
  perft(s, results, results.size());
  assertEquals(results, expected, msg);
}

void perftTest() {
  header("Perft");
  assertPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              {
                  20, 400, 8'902, 197'281, 4'865'609, 119'060'324,
                  // 3'195'901'860, 84'998'978'956, 2'439'530'234'167
              },
              "from start");
  assertPerft(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      {48, 2039, 97862, /*4085603, 193690690, 8031647685*/},
      "Kiwipete by Peter McKenzie");
  assertPerft(
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
      {6, 264, 9467, 422333, /*15833292, 706045033*/}, "pos 4");
  assertPerft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
              {44, 1486, 62379, 2103487, 89941194}, "pos 5");
  assertPerft(
      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
      "10",
      {46, 2079, 89890, 3894594, 164075551, 6923051137},
      "pos 6  Steven Edwards");
}

void test() {
  header("\nRun Test suits...\n");
  pieceMovement();
  pseudoLegalMoves();
  moveClass();
  bitBoards();
  legalMoves();
  makeMove();
  perftTest();

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