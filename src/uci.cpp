#include "uci.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Dagor::UCI {

using namespace std::string_view_literals;

std::vector<std::string> splitOnWhitespace(const std::string &input) {
  std::istringstream stream(input);
  std::vector<std::string> result;
  std::string word;

  while (stream >> word) {
    result.push_back(word);
  }

  return result;
}

void universalChessInterface(std::istream &in, std::ostream &out) {
  while (true) {
    std::string line;
    std::getline(in, line);
    std::vector<std::string> parts = splitOnWhitespace(line);

    if (parts[0] == "quit") {
      return;
    } else if (parts[0] == "uci") {
      out << "id name Dagor-in-Erain\n";
      out << "id author Jakob Teuber\n";
      out << "uciok\n";
    } else if (parts[0] == "isready") {
      out << "readyok\n";
    } else if (parts[0] == "ucinewgame") {
      // nothing
    } else if (parts[0] == "position") {
      // complicated
    } else if (parts[0] == "go") {
      // complicated
    } else {
      std::cerr << "discarding unknown command: `" << line << "`\n";
    }
  }
}

}  // namespace Dagor::UCI
