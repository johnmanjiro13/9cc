#define __STDC_WANT_LIB_EXT2__ 1
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// util.c
//

typedef struct Vector Vector;
struct Vector
{
  void **data;
  int capacity;
  int len;
};

Vector *new_vec();
void vec_push(Vector *v, void *elem);

//
// token.c
//

typedef enum
{
  TK_RESERVED, // Keywords or punctuators
  TK_IDENT,    // Identifier
  TK_NUM,      // Integer literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token
{
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TK_NUM, its value
  char *str;      // Token string
  int len;        // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof();
Token *consume(char *op);
void expect(char *op);
int expect_number();
Token *consume_ident();
Token *tokenize();

extern char *user_input;
extern Token *token;

//
// parse.c
//

typedef enum
{
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, // =
  ND_LVAR,   // Local variable
  ND_RETURN, // "return"
  ND_IF,     // "if"
  ND_ELSE,   // "else"
  ND_WHILE,  // "while"
  ND_FOR,    // "for"
  ND_BLOCK,  // block like {}
  ND_NUM,    // Integer
  ND_CALL,   // Function call
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node
{
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM
  int offset;    // Used if kind == ND_LVAR

  // "if", "while"
  Node *cond;
  Node *then;
  Node *els;

  // "for"
  Node *init;
  Node *updt;

  // Block(compound) statement
  Vector *stmts;

  char *func_name;
  Vector *args;
};

// Local variable type
typedef struct LVar LVar;
struct LVar
{
  LVar *next; // Next variable or NULL
  char *name; // Variable name
  int len;    // Name length
  int offset; // Offset from RBP
};

void *program();

extern Node *code[100];

//
// codegen.c
//

void gen(Node *node);
