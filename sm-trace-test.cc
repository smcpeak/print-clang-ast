// sm-trace-test.cc
// Tests for `sm-trace` module.

#include "sm-trace.h"                  // module under test

#include <assert.h>                    // assert.h

using std::cout;


static void expectLevel(
  int expectLevel,
  std::string const &scope,
  std::string const &spec)
{
  std::string errorRE;
  std::string errorMsg;
  int actualLevel = innerGetTraceLevel(
    scope,
    spec,
    errorRE,
    errorMsg);

  if (actualLevel != expectLevel) {
    cout << "expectLevel: " << expectLevel << "\n"
         << "actualLevel: " << actualLevel << "\n"
         << "scope: " << scope << "\n"
         << "spec: " << spec << "\n";
    cout.flush();
  }

  assert(actualLevel == expectLevel);
  assert(errorRE.empty());
  assert(errorMsg.empty());
}


static void expectError(
  std::string const &spec,
  std::string const &expectErrorRE)
{
  std::string actualErrorRE;
  std::string errorMsg;
  int actualLevel = innerGetTraceLevel(
    "someScope",
    spec,
    actualErrorRE,
    errorMsg);

  assert(actualLevel == -1);
  assert(actualErrorRE == expectErrorRE);

  // There's not a lot of value here in checking the code, so I just
  // print the details.
  cout << "Got expected error:\n"
       << "  spec: " << spec << "\n"
       << "  errorMsg: " << errorMsg << "\n"
       ;
}


class SomeClass {
  DECLARE_CLASS_TRACE_VARS(SomeClass);

public:
  static void foo();
};

DEFINE_CLASS_TRACE_VARS(SomeClass);

void SomeClass::foo()
{
  TRACE1("hello from SomeClass");
}


void trace_unit_tests()
{
  // Enabled.
  expectLevel(1, "someMod", "someMod");

  // Not enabled.
  expectLevel(0, "someMod", "otherMod");

  // Enabled with a different level than 1.
  expectLevel(2, "someMod", "someMod=2");

  // Enabled along with something else.
  expectLevel(1, "someMod", "otherMod,someMod");

  // Enabled using a substring.
  expectLevel(1, "someMod", "otherMod,some");

  // Enabled along with an empty spec.
  expectLevel(1, "someMod", "someMod,");

  // Disabled along with an empty spec.
  expectLevel(0, "someMod", "otherMod,");

  // Enabled with a broad regex.
  expectLevel(1, "someMod", ".");

  // Enabled with a narrow regex.
  expectLevel(1, "someMod", "[sS]ome[mM]o?d");

  // ?
  expectLevel(1, "SomeClass", "Some");

  // Some erroneous regexes.
  expectError("(", "(");

  // Curiously, for this example, Clang-16 libc++ yields an error
  // message of just "regex_error", whereas it's supposed to be a string
  // that describes std::error_badrepeat.
  expectError("*", "*");

  // Check that we recognize the part that is erroneous.
  expectError("x,(", "(");

  // Code that has actual tracing flags.  The output is seen iff the
  // caller actually sets TRACE.
  SomeClass sc;
  sc.foo();

  // Test SCOPED tracing.
  INIT_TRACE("trace_unit_tests");
  {
    TRACE1_SCOPED("start of scoped section");
    TRACE1("inside scoped section");
  }
  TRACE1("after scoped section");
}


// EOF
