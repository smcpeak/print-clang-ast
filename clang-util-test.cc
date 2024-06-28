// clang-util-test.cc
// Tests for `clang-util` module.

#include "clang-util.h"                // module under test

#include "clang-ast-visitor.h"         // ClangASTVisitor
#include "clang-ast.h"                 // ClangASTUtilTempFile
#include "symbolic-line-mapper.h"      // SymbolicLineMapper

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.

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
  TRACE2("GIFDVisitor::visitDecl: " << declKindAtLocStr(decl));

  if (auto namedDecl = dyn_cast<clang::NamedDecl>(decl)) {
    if (auto instFrom = getInstFromDeclOpt(namedDecl)) {
      m_actual.setInsert(GDVTuple({
        // Name and definition-ness of the instantiation.
        namedDeclStr(namedDecl),
        GDValue::makeBool(isThisDeclarationADefinition(namedDecl)),

        // Name and definition-ness of the template.
        namedDeclStr(instFrom),
        GDValue::makeBool(isThisDeclarationADefinition(instFrom)),
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
  ClangASTUtilTempFile ast(source);

  GIFDVisitor visitor(ast.getASTContext());
  visitor.scanTU(ast.getASTContext());

  EXPECT_EQ(visitor.m_actual, expect);
}


void testGetInstFromDeclOpt()
{
  GDValue const T = GDValue::makeBool(true);
  GDValue const F = GDValue::makeBool(false);

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
      GDVTuple{"f<int>()",   T, "f<T>()", T},
      GDVTuple{"f<float>()", T, "f<T>()", T},
    })
  );

  // Simple function template that is not defined.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      void f();

      void g()
      {
        f<int>();
        f<float>();
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"f<int>()",   F, "f<T>()", F},
      GDVTuple{"f<float>()", F, "f<T>()", F},
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
      GDVTuple{"S<int>",   T, "S<T>", T},
      GDVTuple{"S<float>", T, "S<T>", T},
    })
  );

  // Class template contains members that get instantiated.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {
        void m1();
        void m2() {}         // Body instantiated for `int`.
        void m3() {}         // Body instantiated for `float`.
      };

      void g()
      {
        S<int> s1;
        s1.m2();
        S<float> s2;
        s2.m3();
      }
    )",
    GDValue(GDVSet{
      // The "S" in "S::" does not have the template parameters because
      // it results from calling `NamedDecl::getQualifiedNameAsString`,
      // whereas the other "<T>" instances come from my own code.
      //
      // Note how the template definition-ness depends only on whether
      // a definition is syntactically present, while the instantiation
      // definition-ness depends on which parts get instantiated.
      GDVTuple{"S<int>",         T, "S<T>",    T},
      GDVTuple{"S<int>::m1()",   F, "S::m1()", F},
      GDVTuple{"S<int>::m2()",   T, "S::m2()", T},
      GDVTuple{"S<int>::m3()",   F, "S::m3()", T},
      GDVTuple{"S<float>",       T, "S<T>",    T},
      GDVTuple{"S<float>::m1()", F, "S::m1()", F},
      GDVTuple{"S<float>::m2()", F, "S::m2()", T},
      GDVTuple{"S<float>::m3()", T, "S::m3()", T},
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
      GDVTuple{"S<int>",              T, "S<T>",       T},
      GDVTuple{"S<int>::m1<float>()", T, "S::m1<U>()", T},
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
      GDVTuple{"S<int>",             T, "S<T>",          T},
      GDVTuple{"S<int>::Inner",      T, "S::Inner",      T},
      GDVTuple{"S<int>::Inner::m()", T, "S::Inner::m()", T},
    })
  );


  // Class template contains class template.
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      struct S {
        template <typename U>
        struct Inner {
          void m1() {}
          void m2() {}
        };
      };

      void g()
      {
        S<int>::Inner<float> i;
        i.m1();
      }
    )",
    GDValue(GDVSet{
      GDVTuple{"S<int>",                     T, "S<T>",           T},
      GDVTuple{"S<int>::Inner<float>",       T, "S::Inner<U>",    T},
      GDVTuple{"S<int>::Inner<float>::m1()", T, "S::Inner::m1()", T},
      GDVTuple{"S<int>::Inner<float>::m2()", F, "S::Inner::m2()", T},
    })
  );
}


// Visitor that calls `getDeclLoc` on every `Decl` node and accumulates
// the results.
//
// TODO: This is copy+paste of GIFDVisitor.  I should be able to factor
// their commonality.
class DeclLocVisitor : public ClangASTVisitor, public ClangUtil {
public:      // types
  using Base = ClangASTVisitor;

public:      // data
  // Enable symbolic line numbers.
  SymbolicLineMapper m_symLineMapper;

  // Actual results of the calls.
  GDValue m_actual;

public:      // methods
  DeclLocVisitor(clang::ASTContext &astContext)
    : ClangASTVisitor(),
      ClangUtil(astContext),
      m_symLineMapper(astContext),
      m_actual(GDVK_SET)
  {}

  // ClangASTVisitor methods
  virtual void visitDecl(
    VisitDeclContext context,
    clang::Decl const *decl) override;
};


void DeclLocVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  TRACE2("DeclLocVisitor::visitDecl: " << declKindAtLocStr(decl));

  // For the moment I only care about CTSDs.
  if (auto ctsd = dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)) {
    clang::SourceLocation loc = declLoc(ctsd);

    clang::NamedDecl const * NULLABLE instFromDecl =
      getInstFromDeclOpt(ctsd);

    clang::NamedDecl const * NULLABLE instFromDefn =
      getDefnForDeclOpt(instFromDecl);

    // This serves as a crude test of `getDefnOrSelfForDecl`.
    if (instFromDecl) {
      xassert(getDefnOrSelfForDecl(instFromDecl) ==
              (instFromDefn? instFromDefn : instFromDecl));
    }

    m_actual.setInsert(GDVTuple({
      namedDeclStr(ctsd),
      m_symLineMapper.symLineColStr(loc),
      m_symLineMapper.symLineColStr(declLocOpt(instFromDecl)),
      m_symLineMapper.symLineColStr(declLocOpt(instFromDefn)),
    }));
  }

  Base::visitDecl(context, decl);
}


void testOneDeclLoc(char const *fname, GDValue const &expect)
{
  ClangASTUtil ast({fname});

  DeclLocVisitor visitor(ast.getASTContext());
  visitor.scanTU(ast.getASTContext());

  EXPECT_EQ(visitor.m_actual, expect);
}


void testDeclLoc()
{
  testOneDeclLoc("in/src/use-template-via-typedef.cc", GDValue(GDVSet{
    GDVTuple{"S<int>", "S_defn:1", "S_decl:1", "S_defn:1"},
  }));
}


// Test `getLoc`, `getMainFileLoc`, `locLine`, and `locCol`.
void testGetLoc()
{
  ClangASTUtilTempFile ast(R"(// line 1
    int x;
    // line 3
)");

  clang::SourceLocation loc = ast.getMainFileLoc(2,3);
  xassert(ast.locLine(loc) == 2);
  xassert(ast.locCol(loc) == 3);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from print-clang-ast.cc.
void clang_util_unit_tests()
{
  testGetInstFromDeclOpt();
  testDeclLoc();
  testGetLoc();
}


// EOF
