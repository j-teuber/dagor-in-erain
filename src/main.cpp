/// @file main.cpp

#include <cstring>
#include <iostream>

#include "test.h"
#include "uci.h"

using namespace Dagor;

int main(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "uci") == 0) {
    UCI::universalChessInterface(std::cin, std::cout);
  } else if (strcmp(argv[1], "test") == 0) {
    Test::test();
  } else if (strcmp(argv[1], "run") == 0) {
    GameState s{
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0"};
    s.executeMove(Move{"e1c1"});
    s.executeMove(Move{"a6c4"});
    s.executeMove(Move{"c1b1"});
    s.executeMove(Move{"c4a2"});
    std::cerr << s;
    Test::divide(s, 1);
  }

  return 0;
}
