#include "search.h"

#include <random>

namespace Dagor {

Move search(const GameState& state) {
  auto moves = state.generateLegalMoves();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, moves.size() - 1);
  std::size_t index = dis(gen);

  return moves.at(index);
}

}  // namespace Dagor
