#ifndef INT_TYPES_H
#define INT_TYPES_H

#include <cstdint>

using u64 = std::uint64_t;

static_assert(sizeof(u64) == 8,
              "For its BitBoards, this program assumes 64 bit integers.");

#endif