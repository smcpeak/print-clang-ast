// no-qualifiers.cc
// Test of --no-ast-field-qualifiers.

// PRINT_CLANG_AST_OPTIONS: --no-ast-field-qualifiers

int foo()
{
  return 2;
}

// EOF
