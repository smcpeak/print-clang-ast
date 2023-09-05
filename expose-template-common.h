// expose-template-common.h
// Expose some protected types.

#ifndef EXPOSE_TEMPLATE_COMMON_H
#define EXPOSE_TEMPLATE_COMMON_H

#include "clang/AST/DeclTemplate.h"              // clang::FunctionTemplate, etc.


// This namespace contains classes that purport to derive from something
// that has a protected member I want to access.  No instances of these
// types are ever created.  They just publish the members I want.
namespace FakeDerived {
  class RedeclarableTemplateDeclDerived : public clang::RedeclarableTemplateDecl {
  public:
    using clang::RedeclarableTemplateDecl::CommonBase;
  };

  class FunctionTemplateDeclDerived : public clang::FunctionTemplateDecl {
  public:
    using clang::FunctionTemplateDecl::Common;
  };

  class ClassTemplateDeclDerived : public clang::ClassTemplateDecl {
  public:
    using clang::ClassTemplateDecl::Common;
  };
} // namespace FakeDerived


// Now, we declare names for the previously inaccessible types.
//
// These are put into the 'clang' namespace because I've got macros,
// etc., that are set up assuming that all clang AST nodes are in that
// namespace.
namespace clang {
  using RedeclarableTemplateDecl_CommonBase =
    FakeDerived::RedeclarableTemplateDeclDerived::CommonBase;
  using FunctionTemplateDecl_Common =
    FakeDerived::FunctionTemplateDeclDerived::Common;
  using ClassTemplateDecl_Common =
    FakeDerived::ClassTemplateDeclDerived::Common;
}


#endif // EXPOSE_TEMPLATE_COMMON_H
