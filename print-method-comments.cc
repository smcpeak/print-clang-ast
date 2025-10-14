// print-method-comments.cc
// Code for `print-method-comments` module.

#include "print-method-comments.h"     // this module

#include "clang-util-ast-visitor.h"    // ClangUtilASTVisitor

#include "smbase/string-util.h"        // doubleQuote

#include "clang/AST/ASTContext.h"      // clang::ASTContext

#include <iostream>                    // std::ostream

using clang::dyn_cast;


class PMCVisitor : public ClangUtilASTVisitor {
public:      // data
  // Stream to print to.
  std::ostream &m_os;

public:      // methods
  PMCVisitor(std::ostream &os,
             clang::ASTContext &astContext)
    : ClangUtilASTVisitor(astContext),
      m_os(os)
  {}

  // ClangASTVisitor methods.
  virtual void visitDecl(VisitDeclContext context, clang::Decl const *decl) override;
};


void PMCVisitor::visitDecl(VisitDeclContext context, clang::Decl const *decl)
{
  if (auto *md = dyn_cast<clang::CXXMethodDecl>(decl)) {
    m_os << "Method: " << declKindAtLocStr(decl) << "\n";

    // Does this declaration have a comment?
    if (clang::RawComment const *comment =
          m_astContext.getRawCommentForDeclNoCache(md)) {
      clang::StringRef text = comment->getRawText(m_srcMgr);
      m_os << "  Comment: " << doubleQuote(text.str()) << "\n";
    }

    // Is this declaration a definition? Typically there should only be
    // one definition of each method.
    if (md->isThisDeclarationADefinition()) {
      // Search the other declarations of the same entity.
      for (clang::Decl const *other : md->redecls()) {
        m_os << "  Other: " << declKindAtLocStr(other) << "\n";

        if (other != md) {
          // Check this distinct redeclaration to see if it has any
          // comments.
          if (clang::RawComment const *comment =
                m_astContext.getRawCommentForDeclNoCache(other)) {
            clang::StringRef text = comment->getRawText(m_srcMgr);
            m_os << "    Comment: " << doubleQuote(text.str()) << "\n";
          }
        }
      }
    }
  }

  ClangUtilASTVisitor::visitDecl(context, decl);
}


void printMethodComments(
  std::ostream &os,
  clang::ASTContext &astContext)
{
  PMCVisitor v(os, astContext);
  v.scanTU();
}


// EOF
