/** @file generate_movetables.cpp
 *  This file generates `movetables.cpp`. It pre-calculates the possible
 *  moves for a given position to speed up move generation in the search.
 */

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>
#include <vector>

#include "bitboard.h"
#include "movetables.h"
#include "types.h"

using namespace Dagor;
using Dagor::BitBoards::BitBoard;
using std::vector;

/// @brief Compute the attacks that a pawn can make on a given square.
/// @param square the position of the pawn
/// @param color the color of the pawn (`enum Color`). White pawns move upwards,
/// black pawns move downwards.
/// @return a bitboard with the attacked squares set.
BitBoard pawnAttack(Square::t square, Color::t color) {
  BitBoard b;
  int r = Square::rank(square);
  int f = Square::file(square);
  if (color == Color::white) {
    b.setSquareIfInRage(f - 1, r + 1);
    b.setSquareIfInRage(f + 1, r + 1);
  } else {
    b.setSquareIfInRage(f - 1, r - 1);
    b.setSquareIfInRage(f + 1, r - 1);
  }
  return b;
}

/// @brief writes all possible pawn attacks into a file.
/// @param f
void writePawnAttacks(std::ofstream &f) {
  f << " const std::array<std::array<std::uint64_t, Square::size>, "
       "Color::size> _pawnAttacks = {\n ";
  for (auto square : Square::all) {
    f << pawnAttack(square, Color::white).asUint() << "ULL,";
  }
  for (auto square : Square::all) {
    f << "" << pawnAttack(square, Color::black).asUint() << "ULL";
    if (square < Square::size - 1) f << ",\n";
  }
  f << "};\n\n";
}

/// @brief Computes all moves a knight can make on a given square.
/// @param square position of the knight
/// @return a bitboard with all squares set to which the knight could move
/// (assuming the board is otherwise empty).
BitBoard knightMove(Square::t square) {
  BitBoard b;
  int r = Square::rank(square);
  int f = Square::file(square);
  b.setSquareIfInRage(f + 1, r + 2);
  b.setSquareIfInRage(f - 1, r + 2);
  b.setSquareIfInRage(f - 1, r - 2);
  b.setSquareIfInRage(f + 1, r - 2);

  b.setSquareIfInRage(f + 2, r - 1);
  b.setSquareIfInRage(f + 2, r + 1);
  b.setSquareIfInRage(f - 2, r - 1);
  b.setSquareIfInRage(f - 2, r + 1);
  return b;
}

/// @brief writes the knight moves to a file.
/// @param f
void writeKnightMoves(std::ofstream &f) {
  f << "const std::array<std::uint64_t, Square::size> _knightMoves = {\n";
  for (auto square : Square::all) {
    f << knightMove(square).asUint() << "ULL";
    if (square < Square::size - 1) f << ",\n";
  }
  f << "};\n\n";
}

/// @brief Computes the possible moves of a king on a given square (assuming the
/// board is empty otherwise). This includes only the ‘standard’ moves, not
/// castling, which needs to be handled as a special case.
/// @param square the position of the king.
/// @return a bitboard with all the squares set, to which a king can move.
BitBoard kingMove(Square::t square) {
  BitBoard b;
  int r = Square::rank(square);
  int f = Square::file(square);
  b.setSquareIfInRage(f + 1, r + 1);
  b.setSquareIfInRage(f + 0, r + 1);
  b.setSquareIfInRage(f - 1, r + 1);

  b.setSquareIfInRage(f + 1, r);
  b.setSquareIfInRage(f - 1, r);

  b.setSquareIfInRage(f + 1, r - 1);
  b.setSquareIfInRage(f + 0, r - 1);
  b.setSquareIfInRage(f - 1, r - 1);
  return b;
}

/// @brief writes the king moves to a file.
/// @param f
void writeKingMoves(std::ofstream &f) {
  f << "const std::array<std::uint64_t, Square::size> _kingMoves = {\n";
  for (auto square : Square::all) {
    f << kingMove(square).asUint() << "ULL";
    if (square < Square::size - 1) f << ",\n";
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
BitBoard bishopBlockers(Square::t square) {
  BitBoard b;
  Coord::t r = Square::rank(square);
  Coord::t f = Square::file(square);
  for (Coord::t offset = 1; Coord::inRange(offset); offset++) {
    b.setSquareIfInRage(f + offset, r - offset);
    b.setSquareIfInRage(f + offset, r + offset);
    b.setSquareIfInRage(f - offset, r - offset);
    b.setSquareIfInRage(f - offset, r + offset);
  }
  return b & ~BitBoards::edgesOnly;
}

/// @brief Computes one ray of a bishops movement.
/// @param square the position of the bishop
/// @param fileUp if `true`, the ray will travel to the right of the board,
/// otherwise to the left.
/// @param rankUp if `true`, the ray will travel to the top of the board,
/// otherwise to the bottom.
/// @param blockers the pieces blocking the bishops movement. This function
/// assumes that all blockers are enemy pieces and can be captured.
/// @return a bitboard with all the squares set, to which the bishop can move in
/// that rey.
BitBoard bishopMoveRay(Square::t square, bool fileUp, bool rankUp,
                       BitBoard blockers) {
  BitBoard b;
  Coord::t r = Square::rank(square);
  Coord::t f = Square::file(square);
  for (Coord::t offset = 1; Coord::inRange(offset); offset++) {
    Coord::t currentFile = fileUp ? f + offset : f - offset;
    Coord::t currentRank = rankUp ? r + offset : r - offset;
    b.setSquareIfInRage(currentFile, currentRank);
    if (blockers.isSet(Square::index(currentFile, currentRank))) break;
  }
  return b;
}

/// @brief Computes the moves for a bishop.
/// @param square the position of the bishop.
/// @param blockers the pieces blocking the bishops movement. This function
/// assumes that all blockers are enemy pieces and can be captured.
/// @return a bitboard with all the squares set, to which the bishop can move.
BitBoard bishopMoves(Square::t square, BitBoard blockers) {
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
BitBoard rookBlockers(Square::t square) {
  BitBoard b;
  Coord::t r = Square::rank(square);
  Coord::t f = Square::file(square);
  for (Coord::t offset = 1; Coord::inRange(offset); offset++) {
    if (f + offset < Coord::width - 1) b.setSquareIfInRage(f + offset, r);
    if (f - offset > 0 && Coord::inRange(f - offset))
      b.setSquareIfInRage(f - offset, r);
    if (r + offset < Coord::width - 1) b.setSquareIfInRage(f, r + offset);
    if (r - offset > 0 && Coord::inRange(r - offset))
      b.setSquareIfInRage(f, r - offset);
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
/// @param blockers the pieces blocking the rook’s movement. This function
/// assumes that all blockers are enemy pieces and can be captured.
/// @return a bitboard with the moves of this ray set.
BitBoard rookMoveRay(Square::t square, Coord::t addFile, Coord::t addRank,
                     BitBoard blockers) {
  assert(addFile * addRank == 0 && addFile * addFile <= 1 &&
         addRank * addRank <= 1);
  BitBoard b;
  Coord::t r = Square::rank(square);
  Coord::t f = Square::file(square);
  for (Coord::t offset = 1; Coord::inRange(offset); offset++) {
    Coord::t currentFile = f + addFile * offset;
    Coord::t currentRank = r + addRank * offset;
    b.setSquareIfInRage(currentFile, currentRank);
    if (blockers.isSet(Square::index(currentFile, currentRank))) break;
  }
  return b;
}

/// @brief Computes the possible move for a rook.
/// @param square the position of the rook.
/// @param blockers the pieces blocking the rook’s movement. This function
/// assumes that all blockers are enemy pieces and can be captured.
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
  int bitCount = mask.populationCount();
  BitBoard result{};
  BitBoard bits{bitsToSpread};
  for (int bitIndex = 0; bitIndex < bitCount; bitIndex++) {
    int firstSet = mask.findFirstSet();
    mask.unsetSquare(firstSet);
    if (bits.isSet(bitIndex)) {
      result.setSquare(firstSet);
    }
  }
  return result;
}

/// @brief Generates an uniformly distributed uint64.
/// @return the random number.
std::uint64_t randomLong() {
  static std::mt19937_64 engine{
      0};  // we want a predictable generator, NOLINT(*-msc51-cpp)
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
  int bitCount = mask.populationCount();
  unsigned noOfCombinations = 1u << bitCount;
  for (unsigned i = 0; i < noOfCombinations; i++) {
    blockers.push_back(spreadBitsInMask(i, mask));
  }
  return blockers;
}

vector<BitBoard> generatePossibleMoves(Square::t square,
                                       const vector<BitBoard> &blockers,
                                       bool isBishop) {
  vector<BitBoard> moves;
  for (BitBoard blocker : blockers) {
    BitBoard move{isBishop ? bishopMoves(square, blocker)
                           : rookMoves(square, blocker)};
    moves.push_back(move);
  }
  return moves;
}

class SliderInfo {
 public:
  bool isBishop;
  Square::t square;
  BitBoard blockerMask;
  vector<BitBoard> blockers;
  vector<BitBoard> moves;

  SliderInfo(bool isBishop, Square::t square)
      : isBishop{isBishop},
        square{square},
        blockerMask{isBishop ? bishopBlockers(square) : rookBlockers(square)},
        blockers(generatePossibleBlockers(blockerMask)),
        moves(generatePossibleMoves(square, blockers, isBishop)) {}
};

/// @brief Finds the configuration for a perfect hash function for the
/// powerset of the given mask.
/// @param mask
/// @return An object implementing the hash function.
/// `MoveTables::BlockerHash({0}, {0}, 0, 0)` is returned if the generation has
/// failed.
MoveTables::BlockerHash findPerfectHash(SliderInfo &info) {
  unsigned maxTries = 1u << 31;
  int bitCount = info.blockerMask.populationCount();

  for (unsigned k = 0; k < maxTries; k++) {
    MoveTables::BlockerHash candidate{info.blockerMask.asUint(),
                                      randomFewBitsSet(),
                                      static_cast<unsigned>(64 - bitCount), 0};
    vector<bool> hits(info.blockers.size(), false);
    bool failed = false;
    for (unsigned i = 0; !failed && i < info.blockers.size(); i++) {
      unsigned hash = candidate.hash(info.blockers[i]);
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
  return {{0}, {0}, 0, 0};
}

/// @brief Write the representation of a hash function to a file
/// @param f
/// @param hash
/// @param offset a new offset, potentially differing from the one
/// specified in the hash function.
void writeHash(std::ostream &f, MoveTables::BlockerHash &hash,
               unsigned offset) {
  f << "{" << hash.blockerMask << "ULL, " << hash.magic << "ULL, "
    << hash.downShift << "U, " << offset << "U}";
}

void initHashFunctions(vector<SliderInfo> &squareInfo,
                       vector<MoveTables::BlockerHash> &hashFunctions,
                       unsigned &numberOfMoves, bool isBishop) {
  for (auto square : Square::all) {
    SliderInfo info(isBishop, square);
    squareInfo.push_back(info);
    MoveTables::BlockerHash hash{findPerfectHash(info)};
    hashFunctions.push_back(hash);
    numberOfMoves += info.moves.size();
  }
}

void hashMoves(vector<BitBoard> &moves, vector<SliderInfo> &squareInfo,
               vector<MoveTables::BlockerHash> &hashFunctions,
               unsigned &currentOffset) {
  for (auto square : Square::all) {
    MoveTables::BlockerHash hash{hashFunctions[square]};
    SliderInfo info{squareInfo[square]};
    for (unsigned i = 0; i < info.blockers.size(); i++) {
      unsigned h = hash.hash(info.blockers[i]) + currentOffset;
      moves[h] = info.moves[i];
    }
    currentOffset += squareInfo[square].moves.size();
  }
}

void writeSlidingPieces(std::ostream &f) {
  vector<SliderInfo> bishopInfo;
  vector<MoveTables::BlockerHash> bishopHashes;
  vector<SliderInfo> rookInfo;
  vector<MoveTables::BlockerHash> rookHashes;
  unsigned numberOfMoves = 0;
  initHashFunctions(bishopInfo, bishopHashes, numberOfMoves, true);
  initHashFunctions(rookInfo, rookHashes, numberOfMoves, false);

  vector<BitBoard> moves(numberOfMoves);
  unsigned currentOffset = 0;
  hashMoves(moves, bishopInfo, bishopHashes, currentOffset);
  hashMoves(moves, rookInfo, rookHashes, currentOffset);

  currentOffset = 0;
  f << "const std::array<BlockerHash, Square::size> bishopHashes = {{\n";
  for (auto square : Square::all) {
    writeHash(f, bishopHashes[square], currentOffset);
    currentOffset += bishopInfo[square].moves.size();
    if (square < Square::size - 1) f << ",\n";
  }
  f << "\n}};\n\n";

  f << "const std::array<BlockerHash, Square::size> rookHashes = {{\n";
  for (auto square : Square::all) {
    writeHash(f, rookHashes[square], currentOffset);
    currentOffset += rookInfo[square].moves.size();
    if (square < Square::size - 1) f << ",\n";
  }
  f << "\n}};\n\n";

  f << "const std::uint64_t slidingMoves[] = {\n";
  for (unsigned i = 0; i < moves.size(); i++) {
    f << "{" << moves[i].asUint() << "UL}";
    if (i < moves.size() - 1) f << ",\n";
  }
  f << "\n};\n\n";
}

int main() {
  std::ofstream f;
  f.open("movetables.cpp");

  f << "/* auto generated by generate_movetables.cpp - do not modify! */ \n\n"
    << "#include \"movetables.h\" \n\n"
    << "namespace Dagor::MoveTables {\n"
    << "using BitBoards::BitBoard;\n\n";
  f << std::dec;

  writePawnAttacks(f);
  writeKnightMoves(f);
  writeKingMoves(f);
  writeSlidingPieces(f);

  f << "}\n";

  f.close();
  return 0;
}
