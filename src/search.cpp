#include "search.h"

#include <algorithm>
#include <limits>
#include <random>

#include "eval.h"

namespace Dagor::Search {

Move random(const GameState& state) {
  auto moves = state.generateLegalMoves();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, moves.size() - 1);
  std::size_t index = dis(gen);

  return moves.at(index);
}

constexpr int INF = std::numeric_limits<int>::max();

int negatedMax(GameState& state, int depth, int alpha, int beta) {
  if (depth == 0) {
    return Eval::eval(state);
  }

  auto moves = state.generateLegalMoves();
  if (moves.empty()) {
    if (state.isCheck()) {
      return -INF;
    } else {
      return 0;
    }
  }

  for (Move m : moves) {
    state.executeMove(m);
    int eval = -negatedMax(state, depth - 1, -beta, -alpha);
    state.undoMove();
    if (eval >= beta) {
      // Move is too good, opponent will have made a different choice earlier
      return beta;
    }
    alpha = std::max(alpha, eval);
  }
  return alpha;
}

Move negatedMaxSearch(GameState& state) {
  auto moves = state.generateLegalMoves();
  Move bestMove = moves.front();
  int bestScore = std::numeric_limits<int>::min();
  for (Move m : moves) {
    state.executeMove(m);
    int score = -negatedMax(state, 5, -INF, +INF);
    if (score > bestScore) {
      bestScore = score;
      bestMove = m;
    }
    state.undoMove();
  }
  return bestMove;
}

Move search(GameState& state) { return negatedMaxSearch(state); }

}  // namespace Dagor::Search
