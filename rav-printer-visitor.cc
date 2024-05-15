// rav-printer-visitor.cc
// Code for rav-printer-visitor.

#include "rav-printer-visitor-private.h"         // this module

// this dir
#include "util.h"                                // SET_RESTORE

// clang
#include "clang/Basic/Version.h"                 // CLANG_VERSION_MAJOR, CLANG_VERSION_MINOR

using clang::dyn_cast;


#define INCREMENT_INDENT_LEVEL() \
  SET_RESTORE(m_indentLevel, m_indentLevel+1)


// Copy of the TRY_TO macro defined (and then undefined) in
// RecursiveASTVisitor.h.
#define TRY_TO(CALL_EXPR)                                                      \
  do {                                                                         \
    if (!getDerived().CALL_EXPR)                                               \
      return false;                                                            \
  } while (false)


std::string RAVPrinterVisitor::indentString() const
{
  std::ostringstream oss;
  for (int i=0; i < m_indentLevel; ++i) {
    oss << "  ";
  }
  return oss.str();
}


bool RAVPrinterVisitor::TraverseTemplateParameterListHelper(
    clang::TemplateParameterList *TPL) {
  if (TPL) {
    for (clang::NamedDecl *D : *TPL) {
      TRY_TO(TraverseDecl(D));
    }
    if (clang::Expr *RequiresClause = TPL->getRequiresClause()) {
      TRY_TO(TraverseStmt(RequiresClause));
    }
  }
  return true;
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

  INCREMENT_INDENT_LEVEL();

#if 0
  // Compensate for Clang bug visiting partial specializations:
  //
  //   https://github.com/llvm/llvm-project/issues/90586
  //
  // Oops, this does not work because 'TraverseCXXRecordHelper' is
  // private.  Instead of this, I have adapted priter-visitor to
  // suppress the TAW when RAV would.
  //
  if (auto ctpsd = dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl)) {
    // The body of this 'if' is a modified version of what
    // 'DEF_TRAVERSE_TMPL_PART_SPEC_DECL(Class, CXXRecord)' expands to
    // in RecursiveASTVisitor.h.

    // Parameters.
    if (clang::TemplateParameterList const *TPL = ctpsd->getTemplateParameters()) {
      for (clang::TemplateParameterList::const_iterator I = TPL->begin(), E = TPL->end();
           I != E; ++I) {
        TRY_TO(TraverseDecl(*I));
      }
    }

    // TypeAsWritten *instead of* ArgsAsWritten.  This is the fix.
    if (clang::TypeSourceInfo const *tsi = ctpsd->getTypeAsWritten()) {
      TRY_TO(TraverseTypeLoc(tsi->getTypeLoc()));
    }

    // Details in CXXRecord.
    //
    // The problem is 'TraverseCXXRecordHelper' is private!
    TRY_TO(TraverseCXXRecordHelper(ctpsd));

    return true;
  }
#endif // 0

  if (auto D = dyn_cast<clang::TemplateTemplateParmDecl>(decl)) {
    // The base class visitor uses the following block of code, which
    // (1) pointlessly visits the templated decl (it is always nullptr),
    // and (2) visits the default argument *before* the parameters that
    // characterize the template template parameter!
    //
    // Also, the comment is rather misleading, as 'D' is:
    //
    //   template <typename> class T
    //
    // not just 'T'.
    //
    #if 0    // BEGIN: original code
      // D is the "T" in something like
      //   template <template <typename> class T> class container { };
      TRY_TO(TraverseDecl(D->getTemplatedDecl()));
      if (D->hasDefaultArgument() && !D->defaultArgumentWasInherited())
        TRY_TO(TraverseTemplateArgumentLoc(D->getDefaultArgument()));
      TRY_TO(TraverseTemplateParameterListHelper(D->getTemplateParameters()));
    #endif   // END: original code

    // So I'll provide a fixed version:
    assert(D->getTemplatedDecl() == nullptr);
    TRY_TO(TraverseTemplateParameterListHelper(D->getTemplateParameters()));
    if (D->hasDefaultArgument() && !D->defaultArgumentWasInherited())
      TRY_TO(TraverseTemplateArgumentLoc(D->getDefaultArgument()));

    return true;
  }

  bool ret = BaseClass::TraverseDecl(decl);

  if (ret) {
    // Prior to Clang-18.1, RAV has a bug that causes it to miss the
    // initializer expression of a bitfield:
    //
    //   https://github.com/llvm/llvm-project/issues/64916
    //
    // Fix that bug so this printer's output agrees with
    // printer-visitor.
    if (CLANG_VERSION_MAJOR < 18 ||
        (CLANG_VERSION_MAJOR == 18 && CLANG_VERSION_MINOR < 1)) {
      if (auto fieldDecl = dyn_cast<clang::FieldDecl>(decl)) {
        if (fieldDecl->isBitField() && fieldDecl->hasInClassInitializer()) {
          // This will have been missed by the 'TraverseDecl' call
          // above.
          TraverseStmt(fieldDecl->getInClassInitializer());
        }
      }
    }
  }

  return ret;
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

  INCREMENT_INDENT_LEVEL();

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


bool RAVPrinterVisitor::TraverseTemplateArgumentLoc(clang::TemplateArgumentLoc tal)
{
  m_os << indentString() << templateArgumentLocStr(tal) << "\n";

  INCREMENT_INDENT_LEVEL();

  return BaseClass::TraverseTemplateArgumentLoc(tal);
}


bool RAVPrinterVisitor::TraverseNestedNameSpecifierLoc(
  clang::NestedNameSpecifierLoc nnsl)
{
  if (nnsl.hasQualifier()) {
    m_os << indentString() << "NNS "
         << nestedNameSpecifierLocStr(nnsl) << "\n";

    INCREMENT_INDENT_LEVEL();

    return BaseClass::TraverseNestedNameSpecifierLoc(nnsl);
  }
  else {
    return BaseClass::TraverseNestedNameSpecifierLoc(nnsl);
  }
}


void ravPrinterVisitorTU(std::ostream &os,
                         clang::ASTContext &astContext)
{
  RAVPrinterVisitor rpv(os, astContext);
  rpv.TraverseAST(astContext);
}


// EOF
