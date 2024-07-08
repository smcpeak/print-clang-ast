// clang-ast-visitor-nc.cc
// Code for `clang-ast-visitor-nc` module.

#include "clang-ast-visitor-nc.h"      // this module


#define CORE_VISITOR_CASE(Class)                                \
                                                                \
  void ClangASTVisitorNC::visit##Class(                         \
    Visit##Class##Context context,                              \
    clang::Class const *node)                                   \
  {                                                             \
    /* The `const_cast` is the reason this class exists. */     \
    visit##Class##NC(context, const_cast<clang::Class*>(node)); \
  }                                                             \
                                                                \
  void ClangASTVisitorNC::visit##Class##NC(                     \
    Visit##Class##Context context,                              \
    clang::Class *node)                                         \
  {                                                             \
    /* Visit the children of `node`. */                         \
    ClangASTVisitor::visit##Class(context, node);               \
  }

CORE_VISITOR_CASE(Decl)
CORE_VISITOR_CASE(Stmt)

#undef CORE_VISITOR_CASE


// EOF
