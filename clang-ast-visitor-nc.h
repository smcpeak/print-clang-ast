// clang-ast-visitor-nc.h
// `ClangASTVisitorNC`, which is like `ClangASTVisitor` but using
// non-`const` pointers.

#ifndef CLANG_AST_VISITOR_NC_H
#define CLANG_AST_VISITOR_NC_H

#include "clang-ast-visitor.h"         // ClangASTVisitor


// Like `ClangASTVisitor` but using non-`const` pointers.
//
// There is no particular problem with just using `const_cast` in the
// handler methods of `ClangASTVisitor`, but when a client uses this
// class instead, it is clearer to readers that AST modification is a
// possibility.
//
// However, it does add two additional virtual function calls per AST
// node.  I don't know how impactful that is in practice.
//
class ClangASTVisitorNC : public ClangASTVisitor {
public:      // methods
  #define CORE_VISITOR_CASE(Class)                                 \
    /* Override the base class method to call the non-`const` */   \
    /* handler. */                                                 \
    virtual void visit##Class(                                     \
      Visit##Class##Context context,                               \
      clang::Class const *node) override;                          \
                                                                   \
    /* Define a corresponding non-`const` derived handler.  Its */ \
    /* default behavior is to visit the children of `node`. */     \
    virtual void visit##Class##NC(                                 \
      Visit##Class##Context context,                               \
      clang::Class *node);

  CORE_VISITOR_CASE(Decl)
  CORE_VISITOR_CASE(Stmt)

  // The other core visitors either do not need a separate method
  // because they already accept objects without any `const` qualifier,
  // or are rare (`concepts::Requirement`) and follow a different
  // calling pattern.

  #undef CORE_VISITOR_CASE

  // For now, I am not going to add all of the auxiliary methods, but
  // they could be added in the future.
};


#endif // CLANG_AST_VISITOR_NC_H
