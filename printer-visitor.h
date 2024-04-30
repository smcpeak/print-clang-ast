// printer-visitor.h
// PrinterVisitor class.

#ifndef PRINTER_VISITOR_H
#define PRINTER_VISITOR_H

// this dir
#include "clang-ast-visitor.h"                   // ClangASTVisitor
#include "clang-util.h"                          // ClangUtil

// libc++
#include <iosfwd>                                // std::ostream
#include <string>                                // std::string


// Implement a simple indentation-based AST printer that uses the
// visitor for traversal.
class PrinterVisitor : public ClangUtil,
                       public ClangASTVisitor {
public:      // data
  // True to print the VisitXXXContext for each node.  Initially false.
  bool m_printVisitContext;

  // True to print implicit QualType nodes.  Initially false.
  bool m_printImplicitQualTypes;

  // If true, then suppressp printing the TypeAsWritten for a
  // ClassTemplatePartialSpecializationDecl, thereby emulating a bug in
  // RecursiveASTVisitor.  Initially false.
  bool m_omit_CTPSD_TAW;

  // Number of levels of indentation to print.
  int m_indentLevel;

  // Stream to print to.
  std::ostream &m_os;

public:      // methods
  PrinterVisitor(std::ostream &os,
                 clang::ASTContext &astContext);

  // Indentation string corresponding to 'm_indentLevel'.
  std::string indentString() const;

  // ClangASTVisitor methods.
  virtual void visitDecl(VisitDeclContext context, clang::Decl const *decl) override;
  virtual void visitStmt(VisitStmtContext context, clang::Stmt const *stmt) override;
  virtual void visitTypeLoc(VisitTypeContext context, clang::TypeLoc typeLoc) override;
  virtual void visitImplicitQualType(VisitTypeContext context,
                                     clang::QualType qualType) override;
  virtual void visitNestedNameSpecifierLoc(
    VisitNestedNameSpecifierContext context,
    clang::NestedNameSpecifierLoc nnsl) override;
};


// Print the entire TU in 'astContext'.
void printerVisitorTU(std::ostream &os,
                      clang::ASTContext &astContext,
                      bool printVisitContext,
                      bool printImplicitQualTypes,
                      bool omit_CTPSD_TAW);


#endif // PRINTER_VISITOR_H
