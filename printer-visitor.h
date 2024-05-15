// printer-visitor.h
// PrinterVisitor class.

#ifndef PRINTER_VISITOR_H
#define PRINTER_VISITOR_H

// this dir
#include "clang-ast-visitor.h"                   // ClangASTVisitor
#include "clang-util.h"                          // ClangUtil
#include "util-macros.h"                         // ENUM_BITWISE_OPS

// libc++
#include <iosfwd>                                // std::ostream
#include <string>                                // std::string


// Implement a simple indentation-based AST printer that uses the
// visitor for traversal.
class PrinterVisitor : public ClangUtil,
                       public ClangASTVisitor {
public:      // types
  // Flags to control print behavior.
  enum Flags {
    // No flags set.
    F_NONE                             = 0x00,

    // Set to print the VisitXXXContext for each node.
    F_PRINT_VISIT_CONTEXT              = 0x01,

    // Set to print implicit QualType nodes.
    F_PRINT_IMPLICIT_QUAL_TYPES        = 0x02,

    // If set, then suppress printing the TypeAsWritten for a
    // ClassTemplatePartialSpecializationDecl, thereby emulating a bug
    // in RecursiveASTVisitor.
    F_OMIT_CTPSD_TAW                   = 0x04,

    // If set, then when printing a CXXDefaultArgExpr, print default
    // argument too.  The default visitor does not do that because the
    // default argument is not a child node, but RAV does traverse into
    // it.
    F_PRINT_DEFAULT_ARG_EXPRS          = 0x08,

    // All flags set.
    F_ALL                              = 0x0F
  };

public:      // data
  // Current printing flags.  Initially F_NONE.
  Flags m_flags;

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
  virtual void visitTemplateArgumentLoc(
    VisitTemplateArgumentContext context,
    clang::TemplateArgumentLoc tal) override;
  virtual void visitImplicitQualType(
    VisitTypeContext context,
    clang::QualType qualType) override;
  virtual void visitNestedNameSpecifierLoc(
    VisitNestedNameSpecifierContext context,
    clang::NestedNameSpecifierLoc nnsl) override;
};


ENUM_BITWISE_OPS(PrinterVisitor::Flags, PrinterVisitor::F_ALL)


// Print the entire TU in 'astContext'.
void printerVisitorTU(std::ostream &os,
                      clang::ASTContext &astContext,
                      PrinterVisitor::Flags flags);


#endif // PRINTER_VISITOR_H
