#ifndef MOVETABLES_H
#define MOVETABLES_H

#include <array>

#include "bitboard.h"
#include "types.h"

namespace Dagor::MoveTables {

/// @brief The attacks a pawn can make on a given square.
/// Access: `pawnAttacks[color][square]`, where white is `0` and black is `1`.
extern const std::array<std::array<std::uint64_t, Square::size>, Color::size>
    _pawnAttacks;

inline BitBoards::BitBoard pawnAttacks(Color::t color, Square::t square) {
  return {_pawnAttacks[color][square]};
}

/// @brief The moves a knight can make on a given square.
extern const std::array<std::uint64_t, Square::size> _knightMoves;

inline BitBoards::BitBoard knightMoves(Square::t square) {
  return {_knightMoves[square]};
}

/// @brief The moves a king can make on a given square. For his home square
/// this does not include castling moves.
extern const std::array<std::uint64_t, Square::size> _kingMoves;

inline BitBoards::BitBoard kingMoves(Square::t square) {
  return {_kingMoves[square]};
}

/// @brief The move that a sliding piece (bishop, rook or queen) can
/// make on a given square. Access through the hash functions in
/// `bishopHashes` and `rookHashes`.
extern const std::uint64_t slidingMoves[];

/// @brief A hash function that maps a configuration of blocking
/// pieces to an index into the `slidingMoves` table, where the
/// possible moves of a rook or bishop are stored.
/// Both rooks and bishops have one separate hash function for each square.
class BlockerHash {
 public:
  /// @brief The mask singling out the blocking pieces that actually matter
  /// to the figure under consideration.
  const std::uint64_t blockerMask;
  /// @brief The magic number that yields the perfect hash function.
  const std::uint64_t magic;
  /// @brief The amount by which the hash should be shifted down.
  const unsigned downShift;
  /// @brief The offset that should be added to the hash. In `slidingMoves` all
  /// entries lie consecutively, this marks where the entries begin, that can be
  /// accessed through this hash function.
  const unsigned tableOffset;

  BlockerHash(std::uint64_t mask, std::uint64_t magic, unsigned downShift,
              unsigned tableOffset)
      : blockerMask{mask},
        magic{magic},
        downShift{downShift},
        tableOffset{tableOffset} {}

  /// @brief Computes the hash for a configuration of blocking pieces.
  /// @param blockers pieces blocking the bishop’s/rook’s movement.
  /// @return the hash.
  unsigned hash(BitBoards::BitBoard blockers) const {
    blockers &= blockerMask;
    std::uint64_t h = blockers.as_uint() * magic;
    return static_cast<unsigned>(h >> downShift) + tableOffset;
  }

  /// @brief Looks up the possible moves for a bishop/rook with the
  /// specified blocking pieces.
  /// @param blockers pieces blocking the bishop’s/rook’s movement.
  /// @return a bitboard where all squares, to which the bishop/rook can move,
  /// are set.
  BitBoards::BitBoard lookUp(BitBoards::BitBoard blockers) const {
    return {slidingMoves[hash(blockers)]};
  }
};

/// @brief the hash functions to look up bishop moves, by square.
extern const std::array<BlockerHash, Square::size> bishopHashes;
/// @brief the hash function to look up rook moves, by square.
extern const std::array<BlockerHash, Square::size> rookHashes;
}  // namespace Dagor::MoveTables

#endif