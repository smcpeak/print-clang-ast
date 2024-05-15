// clang-decl-context-fwd.h
// Forward declaration of clang::DeclContext.

#ifndef CLANG_DECL_CONTEXT_FWD_H
#define CLANG_DECL_CONTEXT_FWD_H

namespace clang {
  // Unfortunately, this is not declared in clang/AST/ASTFwd.h.
  //
  // It is defined in clang/AST/DeclBase.h.
  class DeclContext;
}

#endif // CLANG_DECL_CONTEXT_FWD_H
