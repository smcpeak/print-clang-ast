// clang-util-test.cc
// Tests for `clang-util` module.

#include "clang-util.h"                // module under test

#include "clang-ast-visitor.h"         // ClangASTVisitor
#include "clang-ast.h"                 // ClangASTUtil

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/temporary-file.h"     // smbase::TemporaryFile

#include "clang/AST/Decl.h"            // clang::NamedDecl

#include <sstream>                     // std::ostringstream

using namespace gdv;
using namespace smbase;

using clang::dyn_cast;


INIT_TRACE("clang-util-test");


OPEN_ANONYMOUS_NAMESPACE


// Visitor that calls `getInstFromDeclOpt` on every `Decl` node and
// accumulates the results.
class GIFDVisitor : public ClangASTVisitor, public ClangUtil {
public:      // types
  using Base = ClangASTVisitor;

public:      // data
  // Actual results of the calls.
  GDValue m_actual;

public:      // methods
  GIFDVisitor(clang::ASTContext &astContext)
    : ClangASTVisitor(),
      ClangUtil(astContext),
      m_actual(GDVK_SET)
  {}

  // ClangASTVisitor methods
  virtual void visitDecl(
    VisitDeclContext context,
    clang::Decl const *decl) override;
};


void GIFDVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  TRACE2("visitDecl: " << declKindAtLocStr(decl));

  if (auto namedDecl = dyn_cast<clang::NamedDecl>(decl)) {
    if (auto instFrom = getInstFromDeclOpt(namedDecl)) {
      m_actual.setInsert(GDVTuple({
        namedDeclStr(namedDecl),
        namedDeclStr(instFrom)
      }));
    }
    else {
      // Do not add anything if the call returns `nullptr`.
    }
  }

  Base::visitDecl(context, decl);
}


void testOneGetInstFromDeclOpt(char const *source, GDValue const &expect)
{
  // Clang has a virtual file system that could be used to avoid
  // actually hitting the disk here, but after a quick look, I decided
  // it appeared to be more trouble than I wanted to deal with.
  TemporaryFile temp("gifd", "cc", source);
  ClangASTUtil ast({temp.getFname()});

  GIFDVisitor visitor(ast.getASTContext());
  visitor.scanTU(ast.getASTContext());

  EXPECT_EQ(visitor.m_actual, expect);
}


void testGetInstFromDeclOpt()
{
  // Simple example of a function template.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      void f() {}

      void g()
      {
        f<int>();
        f<float>();
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"f<int>()",   "f<T>()"},
      GDVTuple{"f<float>()", "f<T>()"},
    })
  );

  // Simple example of a class template.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {};

      void g()
      {
        S<int> s1;
        S<float> s2;
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"S<int>",   "S<T>"},
      GDVTuple{"S<float>", "S<T>"},
    })
  );

  // Class template contains members that get instantiated.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {
        void m1();
        void m2() {}
      };

      void g()
      {
        S<int> s1;
        S<float> s2;
      }
    )",
    GDValue(GDVSet{
      // The "S" in "S::" does not have the template parameters because
      // it results from calling `NamedDecl::getQualifiedNameAsString`,
      // whereas the other "<T>" instances come from my own code.
      GDVTuple{"S<int>",         "S<T>"},
      GDVTuple{"S<int>::m1()",   "S::m1()"},
      GDVTuple{"S<int>::m2()",   "S::m2()"},
      GDVTuple{"S<float>",       "S<T>"},
      GDVTuple{"S<float>::m1()", "S::m1()"},
      GDVTuple{"S<float>::m2()", "S::m2()"},
    })
  );

  // Class template contains function template.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {
        template <typename U>
        void m1() {}
      };

      void g()
      {
        S<int> s1;
        s1.m1<float>();
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"S<int>",              "S<T>"},
      GDVTuple{"S<int>::m1<float>()", "S::m1<U>()"},
    })
  );

  // Class template contains ordinary class.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {
        struct Inner {
          void m() {}
        };
      };

      void g()
      {
        S<int>::Inner i;
        i.m();
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"S<int>",              "S<T>"},
      GDVTuple{"S<int>::Inner",       "S::Inner"},
      GDVTuple{"S<int>::Inner::m()",  "S::Inner::m()"},
    })
  );


  // Class template contains class template.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {
        template <typename U>
        struct Inner {
          void m() {}
        };
      };

      void g()
      {
        S<int>::Inner<float> i;
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"S<int>",                    "S<T>"},
      GDVTuple{"S<int>::Inner<float>",      "S::Inner<U>"},
      GDVTuple{"S<int>::Inner<float>::m()", "S::Inner::m()"},
    })
  );


  // TODO: More tests.
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from print-clang-ast.cc.
void clang_util_unit_tests()
{
  testGetInstFromDeclOpt();
}


// EOF
