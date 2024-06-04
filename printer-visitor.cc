// printer-visitor.cc
// Code for printer-visitor.h.

#include "printer-visitor.h"                     // this module

// this dir
#include "pca-util.h"                            // SET_RESTORE

// clang
#include "clang/AST/ExprCXX.h"                   // clang::CXXDefaultArgExpr

// libc++
#include <sstream>                               // std::ostringstream

using clang::dyn_cast;


PrinterVisitor::PrinterVisitor(std::ostream &os,
                               clang::ASTContext &astContext)
  : ClangUtil(astContext),
    ClangASTVisitor(),
    m_flags(F_NONE),
    m_indentLevel(0),
    m_os(os)
{}


std::string PrinterVisitor::indentString() const
{
  std::ostringstream oss;
  for (int i=0; i < m_indentLevel; ++i) {
    oss << "  ";
  }
  return oss.str();
}


#define PRINT_INDENT_AND_CONTEXT()       \
  m_os << indentString();                \
  if (m_flags & F_PRINT_VISIT_CONTEXT) { \
    m_os << toString(context) << ": ";   \
  }


#define INCREMENT_INDENT_LEVEL() \
  SET_RESTORE(m_indentLevel, m_indentLevel+1)


void PrinterVisitor::visitDecl(VisitDeclContext context,
                               clang::Decl const *decl)
{
  PRINT_INDENT_AND_CONTEXT();
  if (auto nd = dyn_cast<clang::NamedDecl>(decl)) {
    m_os << namedDeclAndKindAtLocStr(nd) << "\n";
  }
  else {
    m_os << decl->getDeclKindName()
         << "Decl at " << declLocStr(decl) << "\n";
  }

  INCREMENT_INDENT_LEVEL();

  ClangASTVisitor::visitDecl(context, decl);
}


void PrinterVisitor::visitStmt(VisitStmtContext context,
                               clang::Stmt const *stmt)
{
  PRINT_INDENT_AND_CONTEXT();
  m_os << stmtKindLocStr(stmt) << "\n";

  INCREMENT_INDENT_LEVEL();

  ClangASTVisitor::visitStmt(context, stmt);

  if (m_flags & F_PRINT_DEFAULT_ARG_EXPRS) {
    if (auto cdae = dyn_cast<clang::CXXDefaultArgExpr>(stmt)) {
      visitStmt(VSC_NONE, cdae->getExpr());
    }
  }
}


void PrinterVisitor::visitTypeLoc(VisitTypeContext context,
                                  clang::TypeLoc typeLoc)
{
  if ((m_flags & F_OMIT_CTPSD_TAW) &&
      context == VTC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL) {
    // This TypeLoc would not be visited by RAV due to a bug:
    //
    //   https://github.com/llvm/llvm-project/issues/90586
    //
    // So, skip printing the node, but print its children, so our
    // output matches that of rav-printer-visitor.
    ClangASTVisitor::visitTypeLoc(context, typeLoc);
    return;
  }

  PRINT_INDENT_AND_CONTEXT();
  m_os << typeLocStr(typeLoc) << "\n";

  INCREMENT_INDENT_LEVEL();

  ClangASTVisitor::visitTypeLoc(context, typeLoc);
}


void PrinterVisitor::visitTemplateArgumentLoc(
  VisitTemplateArgumentContext context,
  clang::TemplateArgumentLoc tal)
{
  PRINT_INDENT_AND_CONTEXT();
  m_os << templateArgumentLocStr(tal) << "\n";

  INCREMENT_INDENT_LEVEL();

  ClangASTVisitor::visitTemplateArgumentLoc(context, tal);
}


void PrinterVisitor::visitImplicitQualType(VisitTypeContext context,
                                           clang::QualType qualType)
{
  if (m_flags & F_PRINT_IMPLICIT_QUAL_TYPES) {
    m_os << indentString();
    if (m_flags & F_PRINT_VISIT_CONTEXT) {
      m_os << "implicit " << toString(context) << ": ";
    }
    m_os << qualTypeStr(qualType) << "\n";
  }
}


void PrinterVisitor::visitNestedNameSpecifierLoc(
  VisitNestedNameSpecifierContext context,
  clang::NestedNameSpecifierLoc nnsl)
{
  PRINT_INDENT_AND_CONTEXT();
  m_os << "NNS " << nestedNameSpecifierLocStr(nnsl) << "\n";

  INCREMENT_INDENT_LEVEL();

  ClangASTVisitor::visitNestedNameSpecifierLoc(context, nnsl);
}


void printerVisitorTU(std::ostream &os,
                      clang::ASTContext &astContext,
                      PrinterVisitor::Flags flags)
{
  PrinterVisitor pv(os, astContext);
  pv.m_flags = flags;
  pv.visitDecl(VDC_NONE, astContext.getTranslationUnitDecl());
}


// EOF
