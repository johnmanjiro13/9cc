#include "9cc.h"

// Global sequence number which is used for jump labels
int label_seq = 1;

static void gen_lval(Node *node)
{
  if (node->kind != ND_LVAR)
    error("left value is not a variable");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node)
{
  switch (node->kind)
  {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_IF:
  {
    int seq = label_seq;
    label_seq++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->els)
    {
      printf("  je .Lelse%d\n", seq);
      gen(node->then);
      printf("  jmp .Lend%d\n", seq);
      printf(".Lelse%d:\n", seq);
      gen(node->els);
      printf(".Lend%d:\n", seq);
    }
    else
    {
      printf("  je .Lend%d\n", seq);
      gen(node->then);
      printf(".Lend%d:\n", seq);
    }
    return;
  }
  case ND_WHILE:
  {
    int seq = label_seq;
    label_seq++;
    printf(".Lbegin%d:\n", seq);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", seq);
    gen(node->then);
    printf("  jmp .Lbegin%d\n", seq);
    printf(".Lend%d:\n", seq);
    return;
  }
  case ND_FOR:
  {
    int seq = label_seq;
    label_seq++;
    gen(node->init);
    printf(".Lbegin%d:\n", seq);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", seq);
    gen(node->then);
    gen(node->updt);
    printf("  jmp .Lbegin%d\n", seq);
    printf(".Lend%d:\n", seq);
    return;
  }
  case ND_BLOCK:
    for (int i = 0; i < node->stmts->len; i++)
    {
      gen(node->stmts->data[i]);
      printf("  pop rax\n");
    }
    return;
  case ND_CALL:
  {
    for (int i = 0; i < node->args->len; i++)
      gen(node->args->data[i]);

    // Set arguments in reverse order
    char *arg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (int i = node->args->len - 1; i >= 0; i--)
      printf("  pop %s\n", arg[i]);

    printf("  call %s\n", node->func_name);
    printf("  push rax\n");
    return;
  }
  case ND_FUNC_DEF:
  {
    printf("%s:\n", node->func_name);

    // Prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; i < node->args->len; i++)
      gen(node->args->data[i]);

    char *arg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (int i = node->args->len - 1; i >= 0; i--)
      printf("  pop %s\n", arg[i]);
    return;
  }
  case ND_FUNC_DEF_END:
  {
    printf("  pop rax\n");

    // Epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}
