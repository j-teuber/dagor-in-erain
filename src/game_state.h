#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <array>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "bitboard.h"
#include "movetables.h"
#include "types.h"

namespace Dagor {

class Move {
 public:
  Square::t start;
  Square::t end;
  Piece::t promotion;
  std::uint8_t flags;

  Move(Square::t start, Square::t end, Piece::t promotion = Piece::empty)
      : start{start}, end{end}, promotion{promotion}, flags{0} {}

  explicit Move(std::string const &algebraic);
};

const Move wkCastle{Square::e1, Square::g1};
const Move wqCastle{Square::e1, Square::c1};
const Move bkCastle{Square::e8, Square::g8};
const Move bqCastle{Square::e8, Square::c8};

inline bool operator==(Move const &a, Move const &b) {
  return a.start == b.start && a.end == b.end && a.promotion == b.promotion &&
         a.flags == b.flags;
}

class GameState;

struct UndoInfo {
  Piece::t piece;
  Piece::t capture;
  Square::t start;
  Square::t end;
  Square::t enPassant;
  CastlingRights::t castlingRights;
  std::uint8_t uneventfulHalfMoves;
  /// @brief Flags marking special Moves:
  ///
  /// - `0`: a normal move
  /// - `1, 2, 3, 4`: a castle move
  /// - `5` an en passant capture
  MoveFlags::t flags;

  UndoInfo(const GameState &state, const Move &move);
};

class GameState {
 public:
  static inline const std::string startingPosition =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  std::array<Piece::t, Square::size> mailbox;
  std::array<BitBoards::BitBoard, Piece::all.size()> pieces;
  std::array<BitBoards::BitBoard, Color::size> colors;
  std::stack<UndoInfo> undoStack;
  std::uint8_t uneventfulHalfMoves;
  CastlingRights::t castlingRights;
  Square::t enPassantSquare;
  Color::t next;

  GameState()
      : mailbox(),
        pieces(),
        colors(),
        undoStack(),
        uneventfulHalfMoves{0},
        castlingRights{CastlingRights::none},
        enPassantSquare{Square::noSquare},
        next{Color::white} {
    mailbox.fill(Piece::empty);
    parseFenString(startingPosition);
  }

  explicit GameState(std::string const &fen)
      : mailbox(),
        pieces(),
        colors(),
        undoStack(),
        uneventfulHalfMoves{0},
        castlingRights{CastlingRights::none},
        enPassantSquare{Square::noSquare},
        next{Color::white} {
    mailbox.fill(Piece::empty);
    parseFenString(fen);
  }

  Piece::t getPiece(Square::t square) const { return mailbox[square]; }
  Color::t getColor(Square::t square) const {
    if (colors[Color::black].isSet(square)) return Color::black;
    if (colors[Color::white].isSet(square)) return Color::white;
    return Color::empty;
  }

  BitBoards::BitBoard forColor(Color::t color) const { return colors[color]; }
  BitBoards::BitBoard forPiece(Piece::t piece) const { return pieces[piece]; }
  BitBoards::BitBoard forPiece(Piece::t piece, Color::t color) const {
    return forPiece(piece) & forColor(color);
  }

  BitBoards::BitBoard occupancy() const {
    return colors[Color::white] | colors[Color::black];
  }

  void unset(Square::t square) {
    Piece::t piece = getPiece(square);
    mailbox[square] = Piece::empty;
    pieces[piece].unsetSquare(square);
    colors[Color::white].unsetSquare(square);
    colors[Color::black].unsetSquare(square);
  }

  void set(Square::t square, Piece::t piece, Color::t color) {
    mailbox[square] = piece;
    pieces[piece].setSquare(square);
    colors[color].setSquare(square);
  }

  inline Color::t us() const { return next; }
  inline Color::t them() const { return Color::opponent(us()); }

  BitBoards::BitBoard getMoves(Piece::t piece, Color::t color,
                               Square::t square) const;
  BitBoards::BitBoard getMoves(Piece::t piece, Color::t color, Square::t square,
                               BitBoards::BitBoard occupancy) const;
  BitBoards::BitBoard getAttacks(Square::t square, Color::t color) const;
  BitBoards::BitBoard getAttacks(Square::t square, Color::t color,
                                 BitBoards::BitBoard occupancy) const;
  bool isCheck();

  std::vector<Move> generateLegalMoves() const;

  void executeMove(Move move);
  void undoMove();
  void parseFenString(const std::string &fenString);
};

inline bool operator==(const GameState &a, const GameState &b) {
  return a.pieces == b.pieces && a.colors == b.colors &&
         a.uneventfulHalfMoves == b.uneventfulHalfMoves &&
         a.castlingRights == b.castlingRights &&
         a.enPassantSquare == b.enPassantSquare && a.next == b.next;
}

std::ostream &operator<<(std::ostream &out, const GameState &board);
std::ostream &operator<<(std::ostream &out, const Move &move);

}  // namespace Dagor

#endif
