#include "9cc.h"

// Input program
char *user_input;

// Current token
Token *token;

// Reports an error and exit.
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Consumes the current token if it matches `op`.
Token *consume(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return NULL;

  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_ident()
{
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

// Ensure that the current token is `op`.
void expect(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected '%c'", *op);
  token = token->next;
}

// Ensure that the current token is TK_NUM.
int expect_number()
{
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

// Create a new token and add it as the next token of `cur`.
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

static bool startswith(char *p, char *q)
{
  return memcmp(p, q, strlen(q)) == 0;
}

static bool is_alpha(char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

static bool is_alnum(char c)
{
  return is_alpha(c) || ('0' <= c && c <= '9');
}

static bool is_str(char *p, char *expect)
{
  int len = strlen(expect);
  return strncmp(p, expect, len) == 0 && !is_alnum(p[len]);
}

static char *read_reserved(char *p)
{
  // keywords
  char *kw[] = {"return", "if"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
  {
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len]))
      return kw[i];
  }
  return NULL;
}

// Tokenize `p` and returns new tokens.
Token *tokenize()
{
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    // Skip whitespace characters.
    if (isspace(*p))
    {
      p++;
      continue;
    }

    // Multi-letter punctuator
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">="))
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // Single-letter punctuator
    if (strchr("+-*/()<>=;", *p))
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Reserved keywords
    char *keyword = read_reserved(p);
    if (keyword)
    {
      int len = strlen(keyword);
      cur = new_token(TK_RESERVED, cur, keyword, len);
      p += len;
      continue;
    }

    // Identifier
    if (is_alpha(*p))
    {
      int len = 1;
      while (is_alnum(p[len]))
        len++;

      char *name = strndup(p, len);
      cur = new_token(TK_IDENT, cur, name, len);
      p += len;
      continue;
    }

    // Integer literal
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
