// printer-visitor.cc
// Code for printer-visitor.h.

#include "printer-visitor.h"                     // this module

// this dir
#include "util.h"                                // SET_RESTORE

// libc++
#include <sstream>                               // std::ostringstream

using clang::dyn_cast;


PrinterVisitor::PrinterVisitor(std::ostream &os,
                               clang::ASTContext &astContext)
  : ClangUtil(astContext),
    ClangASTVisitor(),
    m_printVisitContext(false),
    m_printImplicitQualTypes(false),
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


void PrinterVisitor::visitDecl(VisitDeclContext context,
                               clang::Decl const *decl)
{
  m_os << indentString();
  if (m_printVisitContext) {
    m_os << toString(context) << ": ";
  }
  if (auto nd = dyn_cast<clang::NamedDecl>(decl)) {
    m_os << namedDeclAndKindAtLocStr(nd) << "\n";
  }
  else {
    m_os << decl->getDeclKindName()
         << "Decl at " << declLocStr(decl) << "\n";
  }

  SET_RESTORE(m_indentLevel, m_indentLevel+1);

  ClangASTVisitor::visitDecl(context, decl);
}


void PrinterVisitor::visitStmt(VisitStmtContext context,
                               clang::Stmt const *stmt)
{
  m_os << indentString();
  if (m_printVisitContext) {
    m_os << toString(context) << ": ";
  }
  m_os << stmtKindLocStr(stmt) << "\n";

  SET_RESTORE(m_indentLevel, m_indentLevel+1);

  ClangASTVisitor::visitStmt(context, stmt);
}


void PrinterVisitor::visitTypeLoc(VisitTypeContext context,
                                  clang::TypeLoc typeLoc)
{
  m_os << indentString();
  if (m_printVisitContext) {
    m_os << toString(context) << ": ";
  }
  m_os << typeLocStr(typeLoc) << "\n";

  SET_RESTORE(m_indentLevel, m_indentLevel+1);

  ClangASTVisitor::visitTypeLoc(context, typeLoc);
}


void PrinterVisitor::visitImplicitQualType(VisitTypeContext context,
                                           clang::QualType qualType)
{
  if (m_printImplicitQualTypes) {
    m_os << indentString();
    if (m_printVisitContext) {
      m_os << "implicit " << toString(context) << ": ";
    }
    m_os << qualTypeStr(qualType) << "\n";
  }
}


void printerVisitorTU(std::ostream &os,
                      clang::ASTContext &astContext,
                      bool printVisitContext,
                      bool printImplicitQualTypes)
{
  PrinterVisitor pv(os, astContext);
  pv.m_printVisitContext = printVisitContext;
  pv.m_printImplicitQualTypes = printImplicitQualTypes;
  pv.visitDecl(VDC_NONE, astContext.getTranslationUnitDecl());
}


// EOF
