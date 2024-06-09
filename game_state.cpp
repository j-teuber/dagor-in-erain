#include "game_state.h"

#include <sstream>
#include <stdexcept>
#include <vector>

namespace Dagor {

// no special moves: en passant and castling
BitBoards::BitBoard GameState::getMoves(unsigned piece, unsigned color,
                                        unsigned square,
                                        BitBoards::BitBoard occupancy) const {
  auto moves = BitBoards::BitBoard();
  switch (piece) {
    case Piece::pawn: {
      int offset = color == Color::white ? Board::CompassOffsets::north
                                         : Board::CompassOffsets::south;
      bool canDoubleStep = (color == Color::white) ? Board::rank(square) == 1
                                                   : Board::rank(square) == 6;
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

BitBoards::BitBoard GameState::getMoves(unsigned piece, unsigned color,
                                        unsigned square,
                                        BitBoards::BitBoard occupancy) const {
  auto occupancy = colors[Color::black] | colors[Color::white];
  getMoves(piece, color, square, occupancy);
}

BitBoards::BitBoard GameState::getAttacks(int square, int color) const {
  auto attackers = BitBoards::BitBoard();
  for (unsigned piece = 0; piece < pieces.size(); piece++) {
    attackers |= getMoves(piece, color, square) & pieces[piece];
  }
  return attackers;
}

struct MoveGenerator {
  std::uint8_t attacksOnKing;
  const std::uint8_t myColor;
  const std::uint8_t opponentColor;
  const std::uint8_t kingSquare;

  const GameState &state;
  BitBoards::BitBoard intercepts;
  BitBoards::BitBoard pins;

  std::vector<Move> moves;

  MoveGenerator(const GameState &state)
      : attacksOnKing{0},
        myColor{state.isWhiteNext ? Color::white : Color::black},
        opponentColor{state.isWhiteNext ? Color::black : Color::white},
        kingSquare{state.bitboardFor(Piece::king, myColor).findFirstSet()},
        state(state),
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
      if (state.enPassantSquare != Board::Square::no_square) {
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
    for (unsigned piece = 0; piece < Piece::noPieces; piece++) {
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
    auto upper = rookAttacks & BitBoards::above(Board::rank(kingSquare));
    auto left = rookAttacks & BitBoards::leftOf(Board::file(kingSquare));
    auto lower = rookAttacks & BitBoards::below(Board::rank(kingSquare));
    auto right = rookAttacks & BitBoards::rightOf(Board::file(kingSquare));
    handleSliderRay(rookQueen, upper);
    handleSliderRay(rookQueen, left);
    handleSliderRay(rookQueen, lower);
    handleSliderRay(rookQueen, right);

    auto bishopAttacks = state.getMoves(Piece::bishop, myColor, kingSquare,
                                        state.colors[opponentColor]);
    auto upperLeft = bishopAttacks & BitBoards::above(Board::rank(kingSquare)) &
                     BitBoards::leftOf(Board::file(kingSquare));
    auto upperRight = bishopAttacks &
                      BitBoards::above(Board::rank(kingSquare)) &
                      BitBoards::rightOf(Board::file(kingSquare));
    auto lowerLeft = bishopAttacks & BitBoards::below(Board::rank(kingSquare)) &
                     BitBoards::leftOf(Board::file(kingSquare));
    auto lowerRight = bishopAttacks &
                      BitBoards::below(Board::rank(kingSquare)) &
                      BitBoards::rightOf(Board::file(kingSquare));
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

  void handleLeaperAttacks(unsigned piece) {
    auto attacks = state.getMoves(piece, myColor, kingSquare);
    if (!attacks.is_empty()) attacksOnKing++;
    intercepts |= attacks;
  }

  void enterMoves(unsigned start, unsigned piece, BitBoards::BitBoard targets) {
    for (auto end : targets) {
      if (piece == Piece::pawn &&
          ((myColor == Color::white && Board::rank(end) == 7) ||
           (myColor == Color::black && Board::rank(end) == 7))) {
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
  unsigned myColor = isWhiteNext ? Color::white : Color::black;
  unsigned otherColor = isWhiteNext ? Color::black : Color::white;

  std::vector<Move> moves{};
  for (unsigned piece = 0; piece < pieces.size(); piece++) {
    BitBoards::BitBoard positions = bitboardFor(piece, myColor);
    for (auto start : positions) {
      BitBoards::BitBoard targets = getMoves(piece, myColor, start);
      for (auto end : targets) {
        moves.push_back(Move{start, end, 0});
      }
    }
  }
  return moves;
}

Move::Move(std::string const &algebraic)
    : start{0}, end{0}, promotion{0}, flags{0} {
  start = Board::index(algebraic[0] - 'a', algebraic[1] - '1');
  end = Board::index(algebraic[2] - 'a', algebraic[3] - '1');
  if (algebraic.size() > 4) {
    promotion = pieceTypeFromChar(algebraic[4]);
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
  int rank = Board::width - 1;
  for (char c : fields[0]) {
    if (c == ' ') {
      break;
    } else if ('0' < c && c <= '8') {
      file += static_cast<int>(c - '0');
    } else if (c == '/') {
      file = 0;
      rank--;
    } else {
      unsigned color = pieceColorFromChar(c);
      unsigned type = pieceTypeFromChar(c);
      if (type < Piece::noPieces) {
        pieces[type].set_bit(Board::index(file, rank));
        colors[color].set_bit(Board::index(file, rank));
        file++;
      } else {
        throw std::invalid_argument{"unknown character"};
      }
    }
  }
  isWhiteNext = (fields[1][0] == 'w');
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
    enPassantSquare = Board::Square::no_square;
  } else {
    int file = fields[3][0] - 'a';
    int rank = fields[3][1] - '0';
    enPassantSquare = Board::index(file, rank);
  }
  uneventfulHalfMoves = std::stoi(fields[4]);
  moveCounter = std::stoi(fields[5]);
}

std::ostream &operator<<(std::ostream &out, const GameState &board) {
  for (int rank = Board::width - 1; rank >= 0; rank--) {
    out << (rank + 1) << " | ";
    for (int file = 0; file < Board::width; file++) {
      int index = Board::index(file, rank);
      unsigned piece = board.getPiece(index);
      unsigned color = board.getColor(index);
      out << (color >= Color::noColors ? '.' : piecePrintChar(piece, color));
      out << ' ';
    }
    out << '\n';
  }
  out << "    ";
  for (int file = 0; file < Board::width; file++) out << "--";
  out << "\t moves: " << board.moveCounter
      << ", uneventful: " << static_cast<int>(board.uneventfulHalfMoves)
      << ", next: " << (board.isWhiteNext ? "white" : "black");
  out << "\n    ";
  for (int file = 0; file < Board::width; file++)
    out << Board::file_name(file) << ' ';
  out << "\t en passant: "
      << (board.enPassantSquare == Board::Square::no_square
              ? "-"
              : Board::squareName(board.enPassantSquare))
      << ", castling rights: " << static_cast<int>(board.castlingRights);
  out << std::endl;
  return out;
}

std::ostream &operator<<(std::ostream &out, const Move &move) {
  out << Board::squareName(move.start) << Board::squareName(move.end);
  if (move.promotion != 0) {
    out << piecePrintChar(move.promotion, Color::white);
  }
  return out;
}

}  // namespace Dagor
