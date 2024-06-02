#ifndef CONSTANTS_H
#define CONSTANTS_H

enum Color { white, black, noColors };

enum Piece { pawn, knight, bishop, rook, queen, king, noPieces };

constexpr char piecePrintChar(unsigned type, unsigned color) {
  if (color >= Color::noColors || type >= Piece::noPieces) {
    return '?';
  }

  char c = "PNBRQK"[type];
  c += ('a' - 'A') * (color == Color::white);
  return c;
}

constexpr unsigned pieceColorFromChar(char type) {
  if (type < 'a')
    return Color::white;
  else
    return Color::black;
}

constexpr unsigned pieceTypeFromChar(char type) {
  bool isWhite = pieceColorFromChar(type) == Color::white;
  char lowercase = type + ('a' - 'A') * isWhite;
  switch (lowercase) {
    case 'p':
      return Piece::pawn;
    case 'b':
      return Piece::bishop;
    case 'n':
      return Piece::knight;
    case 'r':
      return Piece::rook;
    case 'q':
      return Piece::queen;
    case 'k':
      return Piece::king;
    default:
      return Piece::noPieces;
  }
}

enum CastlingRights {
  whiteKingSide = 1,
  whiteQueenSide = 2,
  blackKingSide = 4,
  blackQueenSide = 8,
  all = whiteKingSide | whiteQueenSide | blackKingSide | blackQueenSide
};

#endif