/// @file main.cpp

#include <iostream>

#include "bitboard.h"
#include "game_state.h"
#include "movetables.h"

using namespace Dagor;

int main() {
  GameState state{};
  std::cout << state;
  return 0;
}
