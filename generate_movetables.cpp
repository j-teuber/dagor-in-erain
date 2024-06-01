/** @file generate_movetables.cpp
 *  This file generates `movetables.cpp`. It pre-calculates the possible
 *  moves for a given position to speed up move generation in the search.
 */

#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>
#include <vector>

#include "bitboard.h"
#include "board.h"
#include "constants.h"
#include "movetables.h"

using namespace Dagor;
using Dagor::BitBoards::BitBoard;
using std::vector;

/// @brief Compute the attacks that a pawn can make on a given square.
/// @param square the position of the pawn
/// @param color the color of the pawn (`enum Color`). White pawns move upwards,
/// black pawns move downwards.
/// @return a bitboard with the attacked squares set.
BitBoard pawnAttack(int square, int color) {
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  if (color == Color::white) {
    b.set_bit_if_index_valid(f - 1, r + 1);
    b.set_bit_if_index_valid(f + 1, r + 1);
  } else {
    b.set_bit_if_index_valid(f - 1, r - 1);
    b.set_bit_if_index_valid(f + 1, r - 1);
  }
  return b;
}

/// @brief writes all possible pawn attacks into a file.
/// @param f
void writePawnAttacks(std::ofstream &f) {
  f << "const BitBoard pawnAttacks[2][Board::size] = {{\n";
  for (int square = 0; square < Board::size; square++) {
    f << '{' << pawnAttack(square, Color::white).as_uint() << "UL}";
    if (square < Board::size - 1) f << ",\n";
  }
  f << "},\n{";
  for (int square = 0; square < Board::size; square++) {
    f << '{' << pawnAttack(square, Color::black).as_uint() << "UL}";
    if (square < Board::size - 1) f << ",\n";
  }
  f << "}};\n\n";
}

/// @brief Computes all moves a knight can make on a given square.
/// @param square position of the knight
/// @return a bitboard with all squares set to which the knight could move
/// (assuming the board is otherwise empty).
BitBoard knightMove(int square) {
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  b.set_bit_if_index_valid(f + 1, r + 2);
  b.set_bit_if_index_valid(f - 1, r + 2);
  b.set_bit_if_index_valid(f - 1, r - 2);
  b.set_bit_if_index_valid(f + 1, r - 2);

  b.set_bit_if_index_valid(f + 2, r - 1);
  b.set_bit_if_index_valid(f + 2, r + 1);
  b.set_bit_if_index_valid(f - 2, r - 1);
  b.set_bit_if_index_valid(f - 2, r + 1);
  return b;
}

/// @brief writes the knight moves to a file.
/// @param f
void writeKnightMoves(std::ofstream &f) {
  f << "const BitBoard knightMoves[Board::size] = {\n";
  for (int square = 0; square < Board::size; square++) {
    f << '{' << knightMove(square).as_uint() << "UL}";
    if (square < Board::size - 1) f << ",\n";
  }
  f << "};\n\n";
}

/// @brief Computes the possible moves of a king on a given square (assuming the
/// board is empty otherwise). This includes only the ‘standard’ moves, not
/// castling, which needs to be handled as a special case.
/// @param square the position of the king.
/// @return a bitboard with all the squares set, to which a king can move.
BitBoard kingMove(int square) {
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  b.set_bit_if_index_valid(f + 1, r + 1);
  b.set_bit_if_index_valid(f + 0, r + 1);
  b.set_bit_if_index_valid(f - 1, r + 1);

  b.set_bit_if_index_valid(f + 1, r);
  b.set_bit_if_index_valid(f - 1, r);

  b.set_bit_if_index_valid(f + 1, r - 1);
  b.set_bit_if_index_valid(f + 0, r - 1);
  b.set_bit_if_index_valid(f - 1, r - 1);
  return b;
}

/// @brief writes the king moves to a file.
/// @param f
void writeKingMoves(std::ofstream &f) {
  f << "const BitBoard kingMoves[Board::size] = {\n";
  for (int square = 0; square < Board::size; square++) {
    f << '{' << kingMove(square).as_uint() << "UL}";
    if (square < Board::size - 1) f << ",\n";
  }
  f << "};\n\n";
}

/// @brief Computes a mask, where all locations are set, where a blocking piece
/// could impede the further movement of a bishop. Squares on the edge are not
/// considered, because they can only be endpoints of a move anyway. For
/// example, this is the result for a bishop on d5:
///
///     8 | . . . . . . . .
///     7 | . @ . . . @ . .
///     6 | . . @ . @ . . .
///     5 | . . . . . . . .
///     4 | . . @ . @ . . .
///     3 | . @ . . . @ . .
///     2 | . . . . . . @ .
///     1 | . . . . . . . .
///         ----------------     as decimal: 9592139778506752
///         a b c d e f g h      as hex:     0x22140014224000
///
/// @param square the position of the bishop.
/// @return a bitboard with the relevant squares set.
BitBoard bishopBlockers(int square) {
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  for (int offset = 1; offset < Board::width; offset++) {
    b.set_bit_if_index_valid(f + offset, r - offset);
    b.set_bit_if_index_valid(f + offset, r + offset);
    b.set_bit_if_index_valid(f - offset, r - offset);
    b.set_bit_if_index_valid(f - offset, r + offset);
  }
  return b & ~BitBoards::edgesOnly;
}

/// @brief Computes one ray of a bishops movement.
/// @param square the position of the bishop
/// @param fileUp if `true`, the ray will travel to the right of the board,
/// otherwise to the left.
/// @param rankUp if `true`, the ray will travel to the top of the board,
/// otherwise to the bottom.
/// @param blockers the pieces blocking the bishops movement.
/// @return a bitboard with all the squares set, to which the bishop can move in
/// that rey.
BitBoard bishopMoveRay(int square, bool fileUp, bool rankUp,
                       BitBoard blockers) {
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  for (int offset = 0; offset < Board::width; offset++) {
    int currentFile = fileUp ? f + offset : f - offset;
    int currentRank = rankUp ? r + offset : r - offset;
    if (blockers.is_set(Board::index(currentFile, currentRank))) break;
    b.set_bit_if_index_valid(currentFile, currentRank);
  }
  return b;
}

/// @brief Computes the moves for a bishop.
/// @param square the position of the bishop.
/// @param blockers the pieces blocking the bishops movement.
/// @return a bitboard with all the squares set, to which the bishop can move.
BitBoard bishopMoves(int square, BitBoard blockers) {
  return bishopMoveRay(square, true, true, blockers) |
         bishopMoveRay(square, true, false, blockers) |
         bishopMoveRay(square, false, true, blockers) |
         bishopMoveRay(square, false, false, blockers);
}

/// @brief Computes a mask, where all locations are set, where a blocking piece
/// could impede the further movement of a rook. Squares on the edge are not
/// considered, because they can only be endpoints of a move anyway. For
/// example, this is the result for a rook on d5:
///
///     8 | . . . . . . . .
///     7 | . . . @ . . . .
///     6 | . . . @ . . . .
///     5 | . @ @ . @ @ @ .
///     4 | . . . @ . . . .
///     3 | . . . @ . . . .
///     2 | . . . @ . . . .
///     1 | . . . . . . . .
///         ----------------     as decimal: 2261102847592448
///         a b c d e f g h      as hex:     0x8087608080800
///
/// @param square the position of the rook.
/// @return a bitboard with the relevant squares set.
BitBoard rookBlockers(int square) {
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  for (int offset = 1; offset < Board::width; offset++) {
    if (f + offset < Board::width - 1) b.set_bit_if_index_valid(f + offset, r);
    if (f - offset > 0) b.set_bit_if_index_valid(f - offset, r);
    if (r + offset < Board::width - 1) b.set_bit_if_index_valid(f, r + offset);
    if (r - offset > 0) b.set_bit_if_index_valid(f, r - offset);
  }
  return b;
}

/// Computes one ray of a rooks movement. The pair `(addFile, addRank)`
/// describes the direction of the ray:
///
/// - `(+1, 0)` = rook goes to the right
/// - `(-1, 0)` = rook goes to the right
/// - `(0, +1)` = rook goes to the top
/// - `(0, -1)` = rook goes to the bottom
///
/// Other values should not be uses.
///
/// @param square the position of the rook
/// @param addFile If `1`, the rook will travel to the right, if `-1` the rook
/// will travel to the left, if `0` the rook won’t change its file.
/// @param addRank If `1`, the rook will travel to the top, if `-1` the rook
/// will travel to the bottom, if `0` the rook won’t change its rank.
/// @param blockers the pieces blocking the rook’s movement.
/// @return a bitboard with the moves of this ray set.
BitBoard rookMoveRay(int square, int addFile, int addRank, BitBoard blockers) {
  assert(addFile * addRank == 0 && addFile * addFile <= 1 &&
         addRank * addRank <= 1);
  BitBoard b;
  int r = Board::rank(square);
  int f = Board::file(square);
  for (int offset = 0; offset < Board::width; offset++) {
    int currentFile = f + addFile * offset;
    int currentRank = r + addRank * offset;
    if (blockers.is_set(Board::index(currentFile, currentRank))) break;
    b.set_bit_if_index_valid(currentFile, currentRank);
  }
  return b;
}

/// @brief Computes the possible move for a rook.
/// @param square the position of the rook.
/// @param blockers the pieces blocking the rook’s movement.
/// @return a bitboard with all the squares set, to which the bishop can move.
BitBoard rookMoves(int square, BitBoard blockers) {
  return rookMoveRay(square, 1, 0, blockers) |
         rookMoveRay(square, -1, 0, blockers) |
         rookMoveRay(square, 0, 1, blockers) |
         rookMoveRay(square, 0, -1, blockers);
}

/// Spreads the given bits out to cover the ones of the mask.
/// If the `n`th bit in `bitsToSpread` is set, then the `n`th
/// set square in mask will be set in the result as well. This
/// is used to generate subsets of the mask.
///
/// Example:
///
///     bitsToSpread: 0b101010101010      →     result:
///     mask:
///     8 | . . . . . . . .                     8 | . . . . . . . .
///     7 | @ . . . . . . .                     7 | . . . . . . . .
///     6 | @ . . . . . . .                     6 | @ . . . . . . .
///     5 | @ . . . . . . .                     5 | . . . . . . . .
///     4 | @ . . . . . . .                     4 | @ . . . . . . .
///     3 | @ . . . . . . .                     3 | . . . . . . . .
///     2 | @ . . . . . . .                     2 | @ . . . . . . .
///     1 | . @ @ @ @ @ @ .                     1 | . @ . @ . @ . .
///        ----------------                        ----------------
///         a b c d e f g h                         a b c d e f g h
/// @param bitsToSpread the binary data to fill the mask with.
/// @param mask the places where the binary data should
/// @return A bitboard, in which a square is set iff it’s the `n`th set
/// square of the mask and the `n`th bit of `bitsToSpread` is set.
BitBoard spreadBitsInMask(unsigned bitsToSpread, BitBoard mask) {
  int bitCount = mask.popcount();
  BitBoard result{};
  BitBoard bits{bitsToSpread};
  for (int bitIndex = 0; bitIndex < bitCount; bitIndex++) {
    int firstSet = mask.findFirstSet();
    mask.unset_bit(firstSet);
    if (bits.is_set(bitIndex)) {
      result.set_bit(firstSet);
    }
  }
  return result;
}

/// @brief Generates an uniformly distributed uint64.
/// @return the random number.
std::uint64_t randomLong() {
  static constexpr std::uint64_t seed = 0;
  static std::mt19937_64 engine{seed};
  static std::uniform_int_distribution<std::uint64_t> distribution{};
  return distribution(engine);
}

/// @brief Generates a random number with a bias towards numbers where only a
/// few bits are set.
/// @return the random number.
std::uint64_t randomFewBitsSet() {
  return randomLong() & randomLong() & randomLong();
}

/// @brief Generates all possibilities for how blocking pieces can be
/// distributed within the range of the given mask. Mathematically speaking,
/// this produces the powerset of the mask.
/// @param mask the locations where other pieces could potentially block our
/// movement
/// @return the powerset of that mask.
vector<BitBoard> generatePossibleBlockers(BitBoard mask) {
  vector<BitBoard> blockers;
  int bitCount = mask.popcount();
  unsigned noOfCombinations = 1u << bitCount;
  for (unsigned i = 0; i < noOfCombinations; i++) {
    blockers.push_back(spreadBitsInMask(i, mask));
  }
  return blockers;
}

/// @brief Finds the configuration for a perfect hash function for the
/// powerset of the given mask.
/// @param mask
/// @return An object implementing the hash function.
/// `MoveTables::BlockerHash({0}, {0}, 0, 0)` is returned if the generation has
/// failed.
MoveTables::BlockerHash findPerfectHash(BitBoard mask) {
  unsigned maxTries = 1u << 31;
  int bitCount = mask.popcount();
  unsigned noOfBlockers = 1u << bitCount;
  vector<BitBoard> blockers = generatePossibleBlockers(mask);

  for (unsigned k = 0; k < maxTries; k++) {
    MoveTables::BlockerHash candidate{mask, randomFewBitsSet(),
                                      static_cast<unsigned>(64 - bitCount), 0};
    vector<bool> hits(noOfBlockers, false);
    bool failed = false;
    for (unsigned i = 0; !failed && i < noOfBlockers; i++) {
      unsigned hash = candidate.hash(blockers[i]);
      if (hits[hash])
        failed = true;
      else
        hits[hash] = true;
    }
    if (!failed) {
      return candidate;
    }
  }
  std::cerr << "Hash generation has failed!";
  return MoveTables::BlockerHash({0}, {0}, 0, 0);
}

/// @brief Write the representation of a hash function to a file
/// @param f
/// @param hash
/// @param offset a new offset, potentially differing from the one
/// specified in the hash function.
void writeHash(std::ofstream &f, MoveTables::BlockerHash &hash,
               unsigned offset) {
  f << "{{" << hash.blockerMask.as_uint() << "UL}, "
    << "{" << hash.magic.as_uint() << "UL}, " << hash.downShift << "U, "
    << offset << "U}";
}

/// @brief Writes the hashes and moves for the leaping pieces (bishop and rook)
/// to a file.
/// @param f
void writeHashes(std::ofstream &f) {
  f << "const BlockerHash bishopHashes[Board::size] = {\n";
  unsigned offset = 0;
  vector<BitBoard> moves;
  for (int square = 0; square < Board::size; square++) {
    BitBoard mask = bishopBlockers(square);
    MoveTables::BlockerHash hash = findPerfectHash(mask);
    writeHash(f, hash, offset);
    for (auto b : generatePossibleBlockers(mask)) {
      moves.push_back(bishopMoves(square, b));
    }
    offset += 1u << mask.popcount();
    if (square < Board::size - 1) f << ",\n";
  }
  f << "};\n\n";

  f << "const BlockerHash rookHashes[Board::size] = {\n";
  for (int square = 0; square < Board::size; square++) {
    BitBoard mask = rookBlockers(square);
    MoveTables::BlockerHash hash = findPerfectHash(mask);
    writeHash(f, hash, offset);
    for (auto b : generatePossibleBlockers(mask)) {
      moves.push_back(rookMoves(square, b));
    }
    offset += 1u << mask.popcount();
    if (square < Board::size - 1) f << ",\n";
  }
  f << "};\n\n";

  f << "/* Entries: " << offset << ", " << offset * 8 / (2 << 10)
    << " KiB */\n";
  f << "extern const BitBoard slidingMoves[] = {";
  for (unsigned i = 0; i <= offset; i++) {
    f << "{" << moves[i].as_uint() << "UL}";
    if (i < offset) f << ",\n";
  }
  f << "};\n\n";
}

int main() {
  std::ofstream f;
  f.open("movetables.cpp");

  f << "/* auto generated by generate_movetables.cpp - do not modify! */ \n\n"
    << "#include \"movetables.h\" \n\n"
    << "namespace Dagor::MoveTables {\n";
  f << std::dec;

  writePawnAttacks(f);
  writeKnightMoves(f);
  writeKingMoves(f);
  writeHashes(f);

  f << "}\n";

  f.close();
  return 0;
}
