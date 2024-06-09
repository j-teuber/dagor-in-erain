#include "game_state.h"

#include <sstream>
#include <stdexcept>
#include <vector>

namespace Dagor {

// no special moves: en passant and castling
BitBoards::BitBoard GameState::getMoves(Piece::t piece, Color::t color,
                                        Square::t square,
                                        BitBoards::BitBoard occupancy) const {
  auto moves = BitBoards::BitBoard();
  switch (piece) {
    case Piece::pawn: {
      Square::t offset =
          (color == Color::white) ? Square::north : Square::south;
      bool canDoubleStep = (color == Color::white) ? Square::rank(square) == 1
                                                   : Square::rank(square) == 6;
      if (canDoubleStep) {
        moves |= BitBoards::BitBoard(1UL << (square + offset * 2)) & ~occupancy;
      }
      moves |= BitBoards::BitBoard(1UL << (square + offset)) & ~occupancy;
      moves |= MoveTables::pawnAttacks[color][square] & occupancy;
    } break;
    case Piece::knight:
      moves |= MoveTables::knightMoves[square];
      break;
    case Piece::king:
      moves |= MoveTables::kingMoves[square];
      break;
    case Piece::bishop:
      moves |= MoveTables::bishopHashes[square].lookUp(occupancy);
      break;
    case Piece::rook:
      moves |= MoveTables::rookHashes[square].lookUp(occupancy);
      break;
    case Piece::queen:
      moves |= MoveTables::bishopHashes[square].lookUp(occupancy);
      moves |= MoveTables::rookHashes[square].lookUp(occupancy);
      break;

    default:
      return {};
  }
  return moves & ~colors[color];
}

BitBoards::BitBoard GameState::getMoves(Piece::t piece, Color::t color,
                                        Square::t square) const {
  auto occupancy = colors[Color::black] | colors[Color::white];
  return getMoves(piece, color, square, occupancy);
}

BitBoards::BitBoard GameState::getAttacks(Square::t square,
                                          Color::t color) const {
  auto attackers = BitBoards::BitBoard();
  for (auto piece : Piece::all) {
    attackers |= getMoves(piece, color, square) & pieces[piece];
  }
  return attackers;
}

struct MoveGenerator {
  std::uint8_t attacksOnKing;
  const Color::t myColor;
  const Color::t opponentColor;
  const Square::t kingSquare;

  const GameState &state;
  BitBoards::BitBoard intercepts;
  BitBoards::BitBoard pins;

  std::vector<Move> moves;

  MoveGenerator(const GameState &state)
      : attacksOnKing{0},
        myColor{state.next},
        opponentColor{Color::opponent(state.next)},
        kingSquare{state.bitboardFor(Piece::king, myColor).findFirstSet()},
        state{state},
        intercepts{0},
        pins{0},
        moves{} {
    handleLeaperAttacks(Piece::pawn);
    handleLeaperAttacks(Piece::knight);
    handleSliderAttacks();

    if (attacksOnKing > 1) {
      intercepts = {0};

      // We may have generated moves for pinned pieces already, but they
      // don’t matter any more, because in double check, only the king can
      // move.
      moves.clear();
    } else {
      standardNonPins();
      castling();
      if (state.enPassantSquare != Square::noSquare) {
        enPassantCaptures();
      }
    }

    generatePlainKingMoves();
  }

 private:
  void enPassantCaptures() {
    // TODO
  }

  void castling() {
    // TODO
  }

  void standardNonPins() {
    for (auto piece : Piece::nonKing) {
      auto notPinned = state.bitboardFor(piece, myColor) & ~pins;
      for (auto start : notPinned) {
        enterMoves(start, piece, state.getMoves(piece, myColor, start));
      }
    }
  }

  void generatePlainKingMoves() {
    for (auto end : state.getMoves(Piece::king, myColor, kingSquare)) {
      if (state.getAttacks(end, myColor).is_empty()) {
        moves.push_back(Move{kingSquare, end});
      }
    }
  }

  void handleSliderAttacks() {
    BitBoards::BitBoard bishopQueen =
        state.bitboardFor(Piece::bishop, opponentColor) |
        state.bitboardFor(Piece::queen, opponentColor);
    BitBoards::BitBoard rookQueen =
        state.bitboardFor(Piece::rook, opponentColor) |
        state.bitboardFor(Piece::queen, opponentColor);
    auto rookAttacks = state.getMoves(Piece::rook, myColor, kingSquare,
                                      state.colors[opponentColor]);
    auto upper = rookAttacks & BitBoards::above(Square::rank(kingSquare));
    auto left = rookAttacks & BitBoards::leftOf(Square::file(kingSquare));
    auto lower = rookAttacks & BitBoards::below(Square::rank(kingSquare));
    auto right = rookAttacks & BitBoards::rightOf(Square::file(kingSquare));
    handleSliderRay(rookQueen, upper);
    handleSliderRay(rookQueen, left);
    handleSliderRay(rookQueen, lower);
    handleSliderRay(rookQueen, right);

    auto bishopAttacks = state.getMoves(Piece::bishop, myColor, kingSquare,
                                        state.colors[opponentColor]);
    auto upperLeft = bishopAttacks &
                     BitBoards::above(Square::rank(kingSquare)) &
                     BitBoards::leftOf(Square::file(kingSquare));
    auto upperRight = bishopAttacks &
                      BitBoards::above(Square::rank(kingSquare)) &
                      BitBoards::rightOf(Square::file(kingSquare));
    auto lowerLeft = bishopAttacks &
                     BitBoards::below(Square::rank(kingSquare)) &
                     BitBoards::leftOf(Square::file(kingSquare));
    auto lowerRight = bishopAttacks &
                      BitBoards::below(Square::rank(kingSquare)) &
                      BitBoards::rightOf(Square::file(kingSquare));
    handleSliderRay(bishopQueen, upperLeft);
    handleSliderRay(bishopQueen, upperRight);
    handleSliderRay(bishopQueen, lowerLeft);
    handleSliderRay(bishopQueen, lowerRight);
  }

  void handleSliderRay(BitBoards::BitBoard opponentSliders,
                       BitBoards::BitBoard ray) {
    auto attackers = opponentSliders & ray;
    auto ourBlockers = ray & state.colors[myColor];
    if (!attackers.is_empty()) {
      if (ourBlockers.is_empty()) {
        attacksOnKing++;
        intercepts |= ray;
      } else if (ourBlockers.popcount() == 1 && attacksOnKing <= 1) {
        pins |= ourBlockers;
        unsigned square = ourBlockers.findFirstSet();
        unsigned piece = state.getPiece(square);
        auto pseudoLegal = state.getMoves(piece, myColor, square);
        enterMoves(square, piece, pseudoLegal & ray);
      }
    }
  }

  void handleLeaperAttacks(Piece::t piece) {
    auto attacks = state.getMoves(piece, myColor, kingSquare);
    if (!attacks.is_empty()) attacksOnKing++;
    intercepts |= attacks;
  }

  void enterMoves(Square::t start, Piece::t piece,
                  BitBoards::BitBoard targets) {
    for (auto end : targets) {
      if (piece == Piece::pawn &&
          ((myColor == Color::white && Square::rank(end) == 7) ||
           (myColor == Color::black && Square::rank(end) == 7))) {
        moves.push_back(Move{start, end, Piece::knight});
        moves.push_back(Move{start, end, Piece::bishop});
        moves.push_back(Move{start, end, Piece::rook});
        moves.push_back(Move{start, end, Piece::queen});
      }

      moves.push_back(Move{start, end});
    }
  }
};

std::vector<Move> GameState::generateLegalMoves() const {
  std::vector<Move> moves{};
  for (auto piece : Piece::all) {
    BitBoards::BitBoard positions = bitboardFor(piece, next);
    for (auto start : positions) {
      BitBoards::BitBoard targets = getMoves(piece, next, start);
      for (auto end : targets) {
        moves.push_back(Move{start, end, 0});
      }
    }
  }
  return moves;
}

Move::Move(std::string const &algebraic)
    : start{0}, end{0}, promotion{0}, flags{0} {
  start = Square::byName(algebraic[0], algebraic[1]);
  end = Square::byName(algebraic[2], algebraic[3]);
  if (algebraic.size() > 4) {
    promotion = Piece::byName(algebraic[4]);
  }
}

std::vector<std::string> splitFenFields(std::string const &fenString) {
  std::istringstream iss(fenString);
  std::vector<std::string> fields;
  std::string field;
  while (iss >> field) {
    fields.push_back(field);
  }
  return fields;
}

void GameState::parseFenString(const std::string &fenString) {
  std::vector<std::string> fields = splitFenFields(fenString);
  int file = 0;
  int rank = Coord::width - 1;
  for (char c : fields[0]) {
    if (c == ' ') {
      break;
    } else if ('0' < c && c <= '8') {
      file += static_cast<int>(c - '0');
    } else if (c == '/') {
      file = 0;
      rank--;
    } else {
      Color::t color = Color::pieceColorFromChar(c);
      Piece::t type = Piece::byName(c);
      if (Piece::inRange(type)) {
        pieces[type].set_bit(Square::index(file, rank));
        colors[color].set_bit(Square::index(file, rank));
        file++;
      } else {
        throw std::invalid_argument{"unknown character"};
      }
    }
  }
  next = (fields[1][0] == 'w') ? Color::white : Color::black;
  for (char c : fields[2]) {
    switch (c) {
      case 'K':
        castlingRights |= CastlingRights::whiteKingSide;
        break;
      case 'Q':
        castlingRights |= CastlingRights::whiteQueenSide;
        break;
      case 'k':
        castlingRights |= CastlingRights::blackKingSide;
        break;
      case 'q':
        castlingRights |= CastlingRights::blackQueenSide;
        break;
    }
  }
  if (fields[3][0] == '-') {
    enPassantSquare = Square::noSquare;
  } else {
    enPassantSquare = Square::byName(fields[3][0], fields[3][1]);
  }
  uneventfulHalfMoves = std::stoi(fields[4]);
  moveCounter = std::stoi(fields[5]);
}

std::ostream &operator<<(std::ostream &out, const GameState &board) {
  for (auto rank : Coord::reverseRanks) {
    out << (rank + 1) << " | ";
    for (auto file : Coord::files) {
      int index = Square::index(file, rank);
      unsigned piece = board.getPiece(index);
      unsigned color = board.getColor(index);
      out << Piece::name(piece, color);
      out << ' ';
    }
    out << '\n';
  }
  out << "    ";
  for (auto _ : Coord::files) out << "--";
  out << "\t moves: " << board.moveCounter
      << ", uneventful: " << static_cast<int>(board.uneventfulHalfMoves)
      << ", next: " << (board.next == Color::white ? "white" : "black");
  out << "\n    ";
  for (auto file : Coord::files) out << Coord::fileName(file) << ' ';
  out << "\t en passant: "
      << (board.enPassantSquare == Square::noSquare
              ? "-"
              : Square::name(board.enPassantSquare))
      << ", castling rights: " << static_cast<int>(board.castlingRights);
  out << std::endl;
  return out;
}

std::ostream &operator<<(std::ostream &out, const Move &move) {
  out << Square::name(move.start) << Square::name(move.end);
  if (move.promotion != 0) {
    out << Piece::name(move.promotion, Color::white);
  }
  return out;
}

}  // namespace Dagor
