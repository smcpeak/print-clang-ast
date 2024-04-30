// rav-printer-visitor-private.h
// Private decls for rav-printer-visitor.

#ifndef RAV_PRINTER_VISITOR_PRIVATE_H
#define RAV_PRINTER_VISITOR_PRIVATE_H

#include "rav-printer-visitor.h"                 // public decls for this module

// this dir
#include "clang-util.h"                          // ClangUtil

// clang
#include "clang/AST/RecursiveASTVisitor.h"       // clang::RecursiveASTVisitor

// libc++
#include <iosfwd>                                // std::ostream
#include <string>                                // std::string


// Use RAV to print AST nodes.
class RAVPrinterVisitor : public ClangUtil,
                          public clang::RecursiveASTVisitor<RAVPrinterVisitor> {
public:      // types
  // Name of the base class visitor, to make it easier to call its
  // methods from the same-named methods when overridden.
  typedef clang::RecursiveASTVisitor<RAVPrinterVisitor> BaseClass;

public:      // data
  // Number of levels of indentation to print.
  int m_indentLevel;

  // Stream to print to.
  std::ostream &m_os;

public:      // methods
  RAVPrinterVisitor(std::ostream &os, clang::ASTContext &astContext)
    : ClangUtil(astContext),
      clang::RecursiveASTVisitor<RAVPrinterVisitor>(),
      m_indentLevel(0),
      m_os(os)
  {}

  // Indentation string corresponding to 'm_indentLevel'.
  std::string indentString() const;

  // RecursiveASTVisitor customization.
  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }

  // RecursiveASTVisitor methods.
  bool TraverseDecl(clang::Decl *decl);
  bool dataTraverseStmtPre(clang::Stmt *stmt);
  bool dataTraverseStmtPost(clang::Stmt *stmt);
  bool TraverseTypeLoc(clang::TypeLoc TL);
  bool TraverseQualifiedTypeLoc(clang::QualifiedTypeLoc TL);
  bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc nnsl);
};


#endif // RAV_PRINTER_VISITOR_PRIVATE_H
