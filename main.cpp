/// @file main.cpp

#include <iostream>

#include "bitboard.h"
#include "movetables.h"

using namespace Dagor;

int main() {
  std::cout << MoveTables::rookHashes[Board::Square::a1].lookUp(
      BitBoards::BitBoard::single_square_set(Board::Square::a4));
  return 0;
}
