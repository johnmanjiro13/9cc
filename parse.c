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
//       | "{" stmt* "}"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//       | "return" expr ";"
static Node *stmt()
{
  Node *node = stmt_inner();
  return node;
}

void *parse_args(Node *node)
{
  node->args = new_vec();
  if (consume(")"))
    return node;

  vec_push(node->args, expr());
  while (consume(","))
    vec_push(node->args, expr());
  for (int i = 0; i < node->args->len; i++)
    printf("%d\n", *(int *)node->args->data[i]);
  expect(")");
  return node;
}

Node *expect_func_definition()
{
  Token *tok = consume_ident();
  if (!tok)
    error("function name is missing.");

  expect("(");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DEF;
  node->func_name = calloc(1, tok->len + 1);
  strncpy(node->func_name, tok->str, tok->len);

  parse_args(node);

  expect("{");

  return node;
}

bool is_top_level = true;

static Node *stmt_inner()
{
  if (is_top_level)
  {
    Node *node = expect_func_definition();
    is_top_level = false;
    return node;
  }

  // Parse "if-else" statement
  if (consume("if"))
  {
    Node *node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }

  // Parse "while" statement
  if (consume("while"))
  {
    Node *node = new_node(ND_WHILE);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  // Parse "for" statement
  if (consume("for"))
  {
    Node *node = new_node(ND_FOR);
    expect("(");
    if (!consume(";"))
    {
      node->init = expr();
      expect(";");
    }
    if (!consume(";"))
    {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")"))
    {
      node->updt = expr();
      expect(")");
    }
    node->then = stmt();
    return node;
  }

  // Parse block(compound) statement like "{}"
  if (consume("{"))
  {
    Node *node = new_node(ND_BLOCK);
    node->stmts = new_vec();
    while (!consume("}"))
      vec_push(node->stmts, stmt());
    return node;
  }
  else if (consume("}"))
  {
    Node *node = new_node(ND_FUNC_DEF_END);
    is_top_level = true;
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

// primary = "(" expr ")" | num | ident ("(" (assign ("," assign)*)? ")")?
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
    if (!consume("("))
      return new_ident(tok);

    Node *node = new_node(ND_CALL);
    node->args = new_vec();
    parse_args(node);
    node->func_name = tok->str;
    return node;
  }

  return new_num(expect_number());
}
