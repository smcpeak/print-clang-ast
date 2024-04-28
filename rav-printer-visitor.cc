// rav-printer-visitor.cc
// Code for rav-printer-visitor.

#include "rav-printer-visitor-private.h"         // this module

// this dir
#include "util.h"                                // SET_RESTORE

using clang::dyn_cast;


std::string RAVPrinterVisitor::indentString() const
{
  std::ostringstream oss;
  for (int i=0; i < m_indentLevel; ++i) {
    oss << "  ";
  }
  return oss.str();
}


bool RAVPrinterVisitor::TraverseDecl(clang::Decl *decl)
{
  // I don't know why nullptr decls are passed to this function, but it
  // happens, for example with in/src/friend-decl.cc.  RAV itself checks
  // for this case explicitly, and ignores it like this, so I will too.
  if (!decl) {
    return true;
  }

  m_os << indentString();
  if (auto nd = dyn_cast<clang::NamedDecl>(decl)) {
    m_os << namedDeclAndKindAtLocStr(nd) << "\n";
  }
  else {
    m_os << decl->getDeclKindName()
         << "Decl at " << declLocStr(decl) << "\n";
  }

  SET_RESTORE(m_indentLevel, m_indentLevel+1);

  return BaseClass::TraverseDecl(decl);
}


bool RAVPrinterVisitor::dataTraverseStmtPre(clang::Stmt *stmt)
{
  m_os << indentString() << stmtKindLocStr(stmt) << "\n";

  ++m_indentLevel;

  return true;
}


bool RAVPrinterVisitor::dataTraverseStmtPost(clang::Stmt *stmt)
{
  --m_indentLevel;

  return true;
}


bool RAVPrinterVisitor::TraverseTypeLoc(clang::TypeLoc typeLoc)
{
  m_os << indentString() << typeLocStr(typeLoc) << "\n";

  SET_RESTORE(m_indentLevel, m_indentLevel+1);

  return BaseClass::TraverseTypeLoc(typeLoc);
}


bool RAVPrinterVisitor::TraverseQualifiedTypeLoc(clang::QualifiedTypeLoc TL)
{
  // The following line of code is an exact copy of the implementation
  // of this method in RecursiveASTVisitor, but its effect here is
  // different because, here, the call resolves to the derived class
  // method, not the base class method.  The consequence is that, here,
  // we visit the QualifiedTypeLoc instead of skipping it.  My reasoning
  // for visiting it here is I want this code's behavior to match what
  // clang-ast-visitor does, and it visits qualified types.
  return TraverseTypeLoc(TL.getUnqualifiedLoc());
}


void ravPrinterVisitorTU(std::ostream &os,
                         clang::ASTContext &astContext)
{
  RAVPrinterVisitor rpv(os, astContext);
  rpv.TraverseAST(astContext);
}


// EOF
