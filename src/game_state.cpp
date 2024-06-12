#include "game_state.h"

#include <sstream>
#include <stdexcept>
#include <unordered_map>
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
      moves |= MoveTables::pawnAttacks(color, square) & occupancy;
    } break;
    case Piece::knight:
      moves |= MoveTables::knightMoves(square);
      break;
    case Piece::king:
      moves |= MoveTables::kingMoves(square);
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

BitBoards::BitBoard GameState::getAttacks(Square::t square, Color::t color,
                                          BitBoards::BitBoard occupancy) const {
  auto attackers = BitBoards::BitBoard();
  for (auto piece : Piece::all) {
    attackers |= getMoves(piece, color, square, occupancy) &
                 bitboardFor(piece, Color::opponent(color));
  }
  return attackers;
}

BitBoards::BitBoard GameState::getAttacks(Square::t square,
                                          Color::t color) const {
  return getAttacks(square, color, occupancy());
}

struct MoveGenerator {
  std::uint8_t attacksOnKing;
  const Color::t myColor;
  const Color::t opponentColor;
  const Square::t kingSquare;

  const GameState &state;
  BitBoards::BitBoard targets;
  BitBoards::BitBoard pins;
  std::unordered_map<Square::t, BitBoards::BitBoard> pinRays;

  std::vector<Move> moves;

  MoveGenerator(const GameState &state)
      : attacksOnKing{0},
        myColor{state.next},
        opponentColor{Color::opponent(state.next)},
        kingSquare{state.bitboardFor(Piece::king, myColor).findFirstSet()},
        state{state},
        targets{BitBoards::all},
        pins{0},
        pinRays{},
        moves{} {
    handleLeaperAttacks(Piece::pawn);
    handleLeaperAttacks(Piece::knight);
    handleSliderAttacks();

    if (attacksOnKing <= 1) {
      standardNonPins();
      if (attacksOnKing == 0) {
        generateCastling();
      }
      if (state.enPassantSquare != Square::noSquare) {
        enPassantCaptures();
      }
    }

    generatePlainKingMoves();
  }

 private:
  void enPassantCaptures() {
    auto electablePawns =
        state.getMoves(Piece::pawn, opponentColor, state.enPassantSquare);

    Square::t pawnPush =
        (myColor == Color::white) ? Square::north : Square::south;
    Square::t capturePawn = state.enPassantSquare + pawnPush;
    if (targets.isSet(capturePawn)) {
      // The opponents pawn is already there, so we never need to
      // intercept a check by moving there. Therefore, if we have set
      // the pawn as a target, it's because we want to capture him.
      targets.setSquare(state.enPassantSquare);
    }
    if (Square::rank(kingSquare) == Square::rank(capturePawn) &&
        electablePawns.populationCount() == 1) {
      // In this case we need to reevaluate the pins, because we
      // can remove two pieces from a rank at once
      Square::t attackerSquare = electablePawns.findFirstSet();
      BitBoards::BitBoard occupancy{state.occupancy()};
      occupancy.unsetSquare(capturePawn);
      auto rays = state.getMoves(Piece::rook, myColor, kingSquare, occupancy);
      auto ray =
          rays & (kingSquare < attackerSquare ? BitBoards::rightOf(kingSquare)
                                              : BitBoards::leftOf(kingSquare));
      enterMoves(attackerSquare, Piece::pawn,
                 BitBoards::single(state.enPassantSquare) & ray);

    } else {
      for (Square::t start : electablePawns) {
        if (pins.isSet(start)) {
          enterMoves(start, Piece::pawn,
                     BitBoards::single(state.enPassantSquare) & pinRays[start]);
        } else {
          enterMoves(start, Piece::pawn,
                     BitBoards::single(state.enPassantSquare));
        }
      }
    }
  }

  void generateCastling() {
    BitBoards::BitBoard wkEmpty{0xe};
    BitBoards::BitBoard wqEmpty{0x60};
    BitBoards::BitBoard bkEmpty{0xe00000000000000};
    BitBoards::BitBoard bqEmpty{0x6000000000000000};
    BitBoards::BitBoard occupancy{state.occupancy()};
    if (myColor == Color::white) {
      bool right = state.castlingRights & CastlingRights::whiteQueenSide;
      right = right && (occupancy & wqEmpty).isEmpty();
      right = right && state.getAttacks(Square::d1, myColor).isEmpty();
      right = right && state.getAttacks(Square::c1, myColor).isEmpty();
      if (right) {
        moves.push_back(wqCastle);
      }

      right = state.castlingRights & CastlingRights::whiteKingSide;
      right = right && (occupancy & wkEmpty).isEmpty();
      right = right && state.getAttacks(Square::f1, myColor).isEmpty();
      right = right && state.getAttacks(Square::g1, myColor).isEmpty();
      if (right) {
        moves.push_back(wkCastle);
      }
    } else {
      bool right = state.castlingRights & CastlingRights::blackQueenSide;
      right = right && (occupancy & bqEmpty).isEmpty();
      right = right && state.getAttacks(Square::d8, myColor).isEmpty();
      right = right && state.getAttacks(Square::c8, myColor).isEmpty();
      if (right) {
        moves.push_back(bqCastle);
      }

      right = state.castlingRights & CastlingRights::blackKingSide;
      right = right && (occupancy & bkEmpty).isEmpty();
      right = right && state.getAttacks(Square::f8, myColor).isEmpty();
      right = right && state.getAttacks(Square::g8, myColor).isEmpty();
      if (right) {
        moves.push_back(bkCastle);
      }
    }
  }

  void standardNonPins() {
    for (auto piece : Piece::nonKing) {
      auto positions = state.bitboardFor(piece, myColor);
      auto notPinned = positions & ~pins;
      for (auto start : notPinned) {
        enterMoves(start, piece, state.getMoves(piece, myColor, start));
      }
      auto pinned = positions & pins;
      for (auto start : pinned) {
        auto ray = pinRays[start];
        if (!(ray & BitBoards::single(start)).isEmpty()) {
          enterMoves(start, piece, state.getMoves(piece, myColor, start) & ray);
        }
      }
    }
  }

  void generatePlainKingMoves() {
    BitBoards::BitBoard withoutKing{state.occupancy()};
    withoutKing.unsetSquare(kingSquare);
    for (auto end : state.getMoves(Piece::king, myColor, kingSquare)) {
      if (state.getAttacks(end, myColor, withoutKing).isEmpty()) {
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
    if (!attackers.isEmpty()) {
      if (ourBlockers.isEmpty()) {
        attacksOnKing += attackers.populationCount();
        targets &= ray;
      } else if (ourBlockers.populationCount() == 1 && attacksOnKing <= 1) {
        pins |= ourBlockers;
        Square::t pinSquare = ourBlockers.findFirstSet();
        pinRays[pinSquare] = ray;
      }
    }
  }

  void handleLeaperAttacks(Piece::t piece) {
    auto attacks = state.getMoves(piece, myColor, kingSquare) &
                   state.bitboardFor(piece, opponentColor);
    if (!attacks.isEmpty()) {
      attacksOnKing += attacks.populationCount();
      targets &= attacks;
    }
  }

  void enterMoves(Square::t start, Piece::t piece, BitBoards::BitBoard ends) {
    for (auto end : (ends & targets)) {
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
  return MoveGenerator{*this}.moves;
}

inline UndoInfo::UndoInfo(const GameState &state, const Move &move)
    : piece{state.getPiece(move.start)},
      capture{state.getPiece(move.end)},
      start{move.start},
      end{move.end},
      enPassant{state.enPassantSquare},
      castlingRights{state.castlingRights},
      uneventfulHalfMoves{state.uneventfulHalfMoves},
      flags{0} {
  if (enPassant == end && piece == Piece::pawn) {
    flags = MoveFlags::enPassant;
    capture = Piece::pawn;
  } else if (move == wkCastle) {
    flags = MoveFlags::whiteKingSide;
  } else if (move == wqCastle) {
    flags = MoveFlags::whiteQueenSide;
  } else if (move == bkCastle) {
    flags = MoveFlags::blackKingSide;
  } else if (move == bqCastle) {
    flags = MoveFlags::blackQueenSide;
  }
}

void GameState::executeMove(Move move) {
  UndoInfo info{*(this), move};
  undoStack.push(info);

  if (info.piece != Piece::pawn && info.capture == Piece::empty) {
    uneventfulHalfMoves++;
  } else {
    uneventfulHalfMoves = 0;
  }

  if (info.start == Square::e1 || info.start == Square::h1 ||
      info.end == Square::h1) {
    castlingRights &= ~CastlingRights::whiteKingSide;
  }
  if (info.start == Square::e1 || info.start == Square::a1 ||
      info.end == Square::a1) {
    castlingRights &= ~CastlingRights::whiteQueenSide;
  }
  if (info.start == Square::e8 || info.start == Square::h8 ||
      info.end == Square::h8) {
    castlingRights &= ~CastlingRights::blackKingSide;
  }
  if (info.start == Square::e8 || info.start == Square::a8 ||
      info.end == Square::a8) {
    castlingRights &= ~CastlingRights::blackQueenSide;
  }

  if (info.piece == Piece::pawn && Square::rank(info.start) == 1 &&
      Square::rank(info.end) == 3 &&
      !getMoves(Piece::pawn, us(), info.start + Square::south).isEmpty()) {
    enPassantSquare = info.start + Square::south;
  } else if (info.piece == Piece::pawn && Square::rank(info.start) == 6 &&
             Square::rank(info.end) == 4 &&
             !getMoves(Piece::pawn, us(), info.start + Square::north)
                  .isEmpty()) {
    enPassantSquare = info.start + Square::north;
  } else {
    enPassantSquare = Square::noSquare;
  }

  if (info.flags == MoveFlags::normal && info.capture != Piece::empty) {
    pieces[info.capture].unsetSquare(info.end);
    colors[them()].unsetSquare(info.end);
  }

  pieces[info.piece].move(info.start, info.end);
  colors[us()].setSquare(info.end);
  colors[us()].unsetSquare(info.start);

  next = them();
}

void GameState::undoMove() {
  UndoInfo undo{undoStack.top()};
  undoStack.pop();

  enPassantSquare = undo.enPassant;
  uneventfulHalfMoves = undo.uneventfulHalfMoves;
  castlingRights = undo.castlingRights;
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
        pieces[type].setSquare(Square::index(file, rank));
        colors[color].setSquare(Square::index(file, rank));
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
  out << "\t uneventful: " << static_cast<int>(board.uneventfulHalfMoves)
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
