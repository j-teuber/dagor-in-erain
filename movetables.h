#ifndef MOVETABLES_H
#define MOVETABLES_H

#include <array>

#include "bitboard.h"
#include "board.h"

namespace Dagor::MoveTables {

extern const BitBoard::BitBoard pawnAttacks[2][Board::size];
extern const BitBoard::BitBoard knightMoves[Board::size];
extern const BitBoard::BitBoard kingMoves[Board::size];
extern const BitBoard::BitBoard slidingMoves[];

class BlockerHash {
 public:
  const BitBoard::BitBoard blockerMask;
  const BitBoard::BitBoard magic;
  const unsigned downShift;
  const unsigned tableOffset;

  BlockerHash(BitBoard::BitBoard mask, BitBoard::BitBoard magic,
              unsigned downShift, unsigned tableOffset)
      : blockerMask{mask},
        magic{magic},
        downShift{downShift},
        tableOffset{tableOffset} {}

  unsigned hash(BitBoard::BitBoard blockers) const {
    blockers &= blockerMask;
    std::uint64_t h = blockers.as_uint() * magic.as_uint();
    return static_cast<unsigned>(h >> downShift) + tableOffset;
  }

  BitBoard::BitBoard lookUp(BitBoard::BitBoard blockers) const {
    return slidingMoves[hash(blockers)];
  }
};

extern const BlockerHash bishopHashes[Board::size];
extern const BlockerHash rookHashes[Board::size];
}  // namespace Dagor::MoveTables

#endif