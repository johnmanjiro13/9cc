#include "9cc.h"

int main(int argc, char **argv)
{
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // Tokenize and parse.
  user_input = argv[1];
  token = tokenize();
  program();

  // Print out the first half of assembly.
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  // Generate codes from head
  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // Epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
