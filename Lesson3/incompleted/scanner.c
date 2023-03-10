/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"
#include "scanner.h"

extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/

void skipBlank()
{
  while ((currentChar != EOF) && (charCodes[currentChar] == CHAR_SPACE))
    readChar();
}

void skipComment()
{
  int state = 0;
  while ((currentChar != EOF) && (state < 2))
  {
    switch (charCodes[currentChar])
    {
    case CHAR_TIMES:
      state = 1;
      break;
    case CHAR_RPAR:
      if (state == 1)
        state = 2;
      else
        state = 0;
      break;
    default:
      state = 0;
    }
    readChar();
  }
  if (state != 2)
    error(ERR_ENDOFCOMMENT, lineNo, colNo);
}

Token *readIdentKeyword(void)
{
  Token *token = makeToken(TK_NONE, lineNo, colNo);
  int count = 1;

  token->string[0] = (char)currentChar;
  readChar();

  while ((currentChar != EOF) &&
         ((charCodes[currentChar] == CHAR_LETTER) || (charCodes[currentChar] == CHAR_DIGIT)))
  {
    if (count <= MAX_IDENT_LEN)
      token->string[count++] = (char)currentChar;
    readChar();
  }

  if (count > MAX_IDENT_LEN)
  {
    error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
    return token;
  }

  token->string[count] = '\0';
  token->tokenType = checkKeyword(token->string); // return TokenType if type is Keyword, if not type still NONE that means it is IDENT

  if (token->tokenType == TK_NONE)
    token->tokenType = TK_IDENT;

  return token;
}

Token *readNumber(void)
{
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);
  int count = 0;
  int numDot = 0;
  while ((currentChar != EOF) && (charCodes[currentChar] == CHAR_DIGIT || charCodes[currentChar] == CHAR_PERIOD))
  {
    if (charCodes[currentChar] == CHAR_PERIOD)
    {
      token->tokenType = TK_FLOAT;
      numDot += 1;
    }

    if (numDot > 1)
    {
      token->tokenType = TK_NONE;
      error(ERR_INVALIDSYMBOL, token->lineNo, token->colNo);
    }

    token->string[count++] = (char)currentChar;
    readChar();
  }

  token->string[count] = '\0';
  token->value = numDot == 0 ? atoi(token->string) : atof(token->string);
  return token;
}

Token *readConstChar(void)
{
  // currentChar = ' (CHAR_SINGLEQUOTE)
  Token *token = makeToken(TK_CHAR, lineNo, colNo);

  // Read the next char if the next char is EOF -> ERROR: ERR_INVALIDCHARCONSTANT
  readChar();
  if (currentChar == EOF)
  {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }
  // else -> store the char
  token->string[0] = currentChar;
  token->string[1] = '\0';

  // Read the next char if it is ' or not
  readChar();
  if (charCodes[currentChar] == CHAR_SINGLEQUOTE)
  {
    readChar();
  }
  else
  {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
  }
  return token;
}

Token *readConstString(void)
{
  // currentChar = " (CHAR_DOUBLEQUOTE)
  Token *token = makeToken(TK_STRING, lineNo, colNo);
  int ln = lineNo;
  int cn = colNo;
  // Read the next char if the next char is EOF -> ERROR: ERR_INVALIDCHARCONSTANT
  readChar();
  if (currentChar == EOF)
  {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, ln, cn);
    return token;
  }

  int len = 0;
  while (1)
  {
    token->string[len++] = (char)currentChar;
    readChar();
    if (charCodes[currentChar] == CHAR_DOUBLEQUOTE)
    {
      token->string[len] = '\0';
      readChar();
      return token;
    }

    if (charCodes[currentChar] == CHAR_SEMICOLON || currentChar == '\n')
    {
      token->tokenType = TK_NONE;
      error(ERR_ENDOFSTRING, ln, cn);
      return token;
    }

    if (len > MAX_STRING_LENGTH)
    {
      token->tokenType = TK_NONE;
      error(ERR_STRINGTOOLONG, ln, cn);
      return token;
    }
  }
}

Token *getToken(void)
{
  Token *token;

  if (currentChar == EOF)
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar])
  {
  case CHAR_SPACE:
    skipBlank();
    return getToken();
  case CHAR_LETTER:
    return readIdentKeyword();
  case CHAR_DIGIT:
    return readNumber();
  case CHAR_SINGLEQUOTE:
    return readConstChar();
  case CHAR_DOUBLEQUOTE:
    return readConstString();

  // Group 1: Symbols that have 1 case
  case CHAR_EQ: // "="
    token = makeToken(SB_EQ, lineNo, colNo);
    break;
  case CHAR_COMMA: // ","
    token = makeToken(SB_COMMA, lineNo, colNo);
    break;
  case CHAR_SEMICOLON: // ";"
    token = makeToken(SB_SEMICOLON, lineNo, colNo);
    break;
  case CHAR_RPAR: // ")"
    token = makeToken(SB_RPAR, lineNo, colNo);
    break;
  case CHAR_LBRACKET: // "["
    token = makeToken(SB_LBRACKET, lineNo, colNo);
    break;
  case CHAR_RBRACKET: // "]"
    token = makeToken(SB_RBRACKET, lineNo, colNo);
    break;
  case CHAR_PERCENT: // "%"
    token = makeToken(SB_MODUL, lineNo, colNo);
    break;

  // Group 2: Symbols that have n cases
  case CHAR_PLUS: // "+" or "+="
    token = makeToken(SB_PLUS, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // "+="
      token->tokenType = SB_ASSIGN_PLUS;
      readChar();
    }
    return token;
  case CHAR_MINUS: // "-" or "-="
    token = makeToken(SB_MINUS, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // "-="
      token->tokenType = SB_ASSIGN_SUBTRACT;
      readChar();
    }
    return token;
  case CHAR_TIMES: // "*" or "*="
    token = makeToken(SB_TIMES, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // "*="
      token->tokenType = SB_ASSIGN_TIMES;
      readChar();
    }
    return token;
  case CHAR_SLASH: // "/" or "/="
    token = makeToken(SB_SLASH, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // "/="
      token->tokenType = SB_ASSIGN_DIVIDE;
      readChar();
    }
    return token;
  case CHAR_LT: // "<" or "<="
    token = makeToken(SB_LT, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // "<="
      token->tokenType = SB_LE;
      readChar();
    }
    return token;
  case CHAR_GT: // ">" or ">="
    token = makeToken(SB_GT, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // ">="
      token->tokenType = SB_GE;
      readChar();
    }
    return token;
  case CHAR_PERIOD: // "." or ".)"
    token = makeToken(SB_PERIOD, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_RPAR)
    {
      // ".)"
      token->tokenType = SB_RSEL;
      readChar();
    }
    return token;
  case CHAR_COLON: // ":" or ":="
    token = makeToken(SB_COLON, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // ":="
      token->tokenType = SB_ASSIGN;
      readChar();
    }
    return token;
  case CHAR_EXCLAIMATION: // "!" or "!="
    token = makeToken(TK_NONE, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ)
    {
      // "!="
      token->tokenType = SB_NEQ;
      readChar();
    }
    else
    {
      // invalid "!"
      error(ERR_INVALIDSYMBOL, token->lineNo, token->colNo);
    }
    return token;
  case CHAR_LPAR: // "(" or "(." or "(*"
    token = makeToken(SB_LPAR, lineNo, colNo);
    readChar();
    switch (charCodes[currentChar])
    {
    case CHAR_PERIOD: // "(."
      token->tokenType = SB_LSEL;
      break;
    case CHAR_TIMES: // "(*"
      skipComment();
      return getToken();
    default: // "("
      return token;
    }
    break;
  default:
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo);
    break;
  }
  readChar();
  return token;
}

Token *getValidToken(void)
{
  Token *token = getToken();
  while (token->tokenType == TK_NONE)
  {
    free(token);
    token = getToken();
  }
  return token;
}

// ************************************************** //

void printToken(Token *token)
{
  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType)
  {
  case TK_NONE:
    printf("TK_NONE\n");
    break;
  case TK_IDENT:
    printf("TK_IDENT(%s)\n", token->string);
    break;
  case TK_NUMBER:
    printf("TK_NUMBER(%s)\n", token->string);
    break;
  case TK_CHAR:
    printf("TK_CHAR(\'%s\')\n", token->string);
    break;
  case TK_STRING:
    printf("TK_STRING(\"%s\")\n", token->string);
    break;
  case TK_EOF:
    printf("TK_EOF\n");
    break;
  case TK_FLOAT:
    printf("TK_FLOAT(%s)\n", token->string);
    break;

  case KW_PROGRAM:
    printf("KW_PROGRAM\n");
    break;
  case KW_CONST:
    printf("KW_CONST\n");
    break;
  case KW_TYPE:
    printf("KW_TYPE\n");
    break;
  case KW_VAR:
    printf("KW_VAR\n");
    break;
  case KW_INTEGER:
    printf("KW_INTEGER\n");
    break;
  case KW_FLOAT:
    printf("KW_FLOAT\n");
    break;
  case KW_CHAR:
    printf("KW_CHAR\n");
    break;
  case KW_ARRAY:
    printf("KW_ARRAY\n");
    break;
  case KW_OF:
    printf("KW_OF\n");
    break;
  case KW_FUNCTION:
    printf("KW_FUNCTION\n");
    break;
  case KW_PROCEDURE:
    printf("KW_PROCEDURE\n");
    break;
  case KW_BEGIN:
    printf("KW_BEGIN\n");
    break;
  case KW_END:
    printf("KW_END\n");
    break;
  case KW_CALL:
    printf("KW_CALL\n");
    break;
  case KW_IF:
    printf("KW_IF\n");
    break;
  case KW_THEN:
    printf("KW_THEN\n");
    break;
  case KW_ELSE:
    printf("KW_ELSE\n");
    break;
  case KW_WHILE:
    printf("KW_WHILE\n");
    break;
  case KW_DO:
    printf("KW_DO\n");
    break;
  case KW_FOR:
    printf("KW_FOR\n");
    break;
  case KW_TO:
    printf("KW_TO\n");
    break;

  case SB_SEMICOLON:
    printf("SB_SEMICOLON\n");
    break;
  case SB_COLON:
    printf("SB_COLON\n");
    break;
  case SB_PERIOD:
    printf("SB_PERIOD\n");
    break;
  case SB_COMMA:
    printf("SB_COMMA\n");
    break;
  case SB_ASSIGN:
    printf("SB_ASSIGN\n");
    break;
  case SB_EQ:
    printf("SB_EQ\n");
    break;
  case SB_NEQ:
    printf("SB_NEQ\n");
    break;
  case SB_LT:
    printf("SB_LT\n");
    break;
  case SB_LE:
    printf("SB_LE\n");
    break;
  case SB_GT:
    printf("SB_GT\n");
    break;
  case SB_GE:
    printf("SB_GE\n");
    break;
  case SB_PLUS:
    printf("SB_PLUS\n");
    break;
  case SB_MINUS:
    printf("SB_MINUS\n");
    break;
  case SB_TIMES:
    printf("SB_TIMES\n");
    break;
  case SB_SLASH:
    printf("SB_SLASH\n");
    break;
  case SB_LPAR:
    printf("SB_LPAR\n");
    break;
  case SB_RPAR:
    printf("SB_RPAR\n");
    break;
  case SB_LSEL:
    printf("SB_LSEL\n");
    break;
  case SB_RSEL:
    printf("SB_RSEL\n");
    break;
  case SB_ASSIGN_PLUS:
    printf("SB_ASSIGN_PLUS\n");
    break;
  case SB_ASSIGN_SUBTRACT:
    printf("SB_ASSIGN_SUBTRACT\n");
    break;
  case SB_ASSIGN_TIMES:
    printf("SB_ASSIGN_TIMES\n");
    break;
  case SB_ASSIGN_DIVIDE:
    printf("SB_ASSIGN_DIVIDE\n");
    break;
  case SB_MODUL:
    printf("SB_MOD\n");
    break;
  case SB_LBRACKET:
    printf("SB_LBRACKET\n");
    break;
  case SB_RBRACKET:
    printf("SB_RBRACKET\n");
    break;
  }
}
