/// @file main.cpp

#include <iostream>

#include "bitboard.h"
#include "game_state.h"
#include "movetables.h"

using namespace Dagor;

int main() {
  GameState state{
      "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"};
  std::cout << state;
  std::cout << Move("c7e8q");

  return 0;
}
