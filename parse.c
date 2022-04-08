#include "9cc.h"

// Local variables
LVar *locals;

// Codes
Node *code[100];

static LVar *new_lvar(Token *tok)
{
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  if (locals == NULL)
  {
    lvar->offset = 8;
  }
  else
  {
    lvar->offset = locals->offset + 8;
  }
  return lvar;
}

static LVar *find_lvar(Token *tok)
{
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

static Node *new_node(NodeKind kind)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_ident(Token *tok)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar)
  {
    node->offset = lvar->offset;
  }
  else
  {
    lvar = new_lvar(tok);
    node->offset = lvar->offset;
    locals = lvar;
  }
  return node;
}

static Node *new_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

void *program();
static Node *stmt();
static Node *stmt_inner();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// program = stmt*
void *program()
{
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

// stmt = expr ";"
//       | "if" "(" expr ")" stmt
//       | "return" expr ";"
static Node *stmt()
{
  Node *node = stmt_inner();
  return node;
}

static Node *stmt_inner()
{
  // Parse "if" statement
  if (consume("if"))
  {
    Node *node = new_node(ND_IF);
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  }

  // Parse "return" statement
  if (consume("return"))
  {
    Node *node = new_node(ND_RETURN);
    node->lhs = expr();
    expect(";");
    return node;
  }

  // Parse expression statement
  Node *node = expr();
  expect(";");
  return node;
}

// expr = assign
static Node *expr()
{
  return assign();
}

// assign = equality ("=" assign)?
static Node *assign()
{
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality()
{
  Node *node = relational();
  for (;;)
  {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational()
{
  Node *node = add();
  for (;;)
  {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add()
{
  Node *node = mul();
  for (;;)
  {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul()
{
  Node *node = unary();
  for (;;)
  {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? unary
//       | primary
static Node *unary()
{
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num | ident
static Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok)
  {
    return new_ident(tok);
  }

  return new_num(expect_number());
}
