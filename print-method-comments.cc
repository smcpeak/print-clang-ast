// print-method-comments.cc
// Code for `print-method-comments` module.

#include "print-method-comments.h"     // this module

#include "clang-util-ast-visitor.h"    // ClangUtilASTVisitor

#include "smbase/sm-env.h"             // smbase::envAsBool
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

  void printBases(clang::CXXRecordDecl const *crd);

  // ClangASTVisitor methods.
  virtual void visitDecl(VisitDeclContext context, clang::Decl const *decl) override;
};


// Ad-hoc code to print some information about a class's base class
// specifiers, as part of answering
// https://stackoverflow.com/questions/79893236/recovering-typedef-type-alias-name-from-a-substituted-template-type-parameter-in
void PMCVisitor::printBases(clang::CXXRecordDecl const *crd)
{
  if (!crd->isThisDeclarationADefinition()) {
    return;
  }

  m_os << "CXXRecordDecl definition: " << namedDeclAndKindAtLocStr(crd) << "\n";

  for (clang::CXXBaseSpecifier const &baseSpec : crd->bases()) {
    clang::TypeLoc baseTL = baseSpec.getTypeSourceInfo()->getTypeLoc();

    m_os << "  Base: " << typeLocStr(baseTL) << "\n";

    // NOTE: In the Clang github dev trunk, `ElaboratedTypeLoc` has been
    // renamed to `ElaboratedNameTypeLoc` (see PR 147835).  The code
    // here was written for Clang-18, which uses the older name.
    if (auto baseETL = baseTL.getAs<clang::ElaboratedTypeLoc>()) {
      if (auto baseTSTL = baseETL.getNamedTypeLoc().getAs<clang::TemplateSpecializationTypeLoc>()) {
        unsigned numArgs = baseTSTL.getNumArgs();
        m_os << "    TSTL has " << numArgs << " args:\n";

        for (unsigned i=0; i < numArgs; ++i) {
          m_os << "      Arg " << i << ":\n";

          clang::TemplateArgumentLoc argLoc = baseTSTL.getArgLoc(i);
          if (clang::TypeSourceInfo const *argTSI = argLoc.getTypeSourceInfo()) {
            clang::TypeLoc argTL = argTSI->getTypeLoc();
            m_os << "        Arg TypeLoc: " << typeLocStr(argTL) << "\n";

            if (auto argETL = argTL.getAs<clang::ElaboratedTypeLoc>()) {
              if (auto argTTL = argETL.getNamedTypeLoc().getAs<clang::TypedefTypeLoc>()) {
                clang::TypedefType const *argTT = argTTL.getTypePtr();
                assert(argTT);
                clang::TypedefNameDecl const *argTND = argTT->getDecl();
                assert(argTND);
                m_os << "        Arg TypedefNameDecl: " << namedDeclAndKindAtLocStr(argTND) << "\n";
              }
              else {
                m_os << "        Not a TypedefTypeLoc.\n";
              }
            }
            else {
              m_os << "        Not an ElaboratedTypeLoc.\n";
            }
          }
          else {
            m_os << "        Not a type argument.\n";
          }
        }
      }
      else {
        m_os << "    Not a TemplateSpecializationTypeLoc.\n";
      }
    }
    else {
      m_os << "   Not an ElaboratedTypeLoc.\n";
    }
  }
}


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

  static bool doPrintBases = smbase::envAsBool("PRINT_BASES");
  if (doPrintBases) {
    if (auto *crd = dyn_cast<clang::CXXRecordDecl>(decl)) {
      printBases(crd);
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
