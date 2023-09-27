// trace.h
// Debug trace support.

#ifndef TRACE_H
#define TRACE_H

#include <iostream>                    // std::clog, std::endl
#include <string>                      // std::string


/* Return the active tracing level for the specified scope string.

   The tracing level is a non-negative integer where 0 means disabled
   and higher levels correspond to increased verbosity.

   If the envvar "TRACE" is not set, this returns 0.  Otherwise, it is
   split at comma boundaries, with each element of the form:

     regex[=level]

   If the scope matches any regex, then the return value is the highest
   associated level, where an unspecified level means 1.  If it does not
   match any regex then the return value is 0.

   If any regex has a syntax error, this prints an error to stderr and
   then exits the program with exit code 2.

   This function is somewhat slow because, on every call, it parses the
   TRACE envvar and re-evaluates all of the contained regexes.  The
   caller should ensure it is not called repeatedly in an inner loop.
*/
int getTraceLevel(char const *scope);


// Internal function exposed only for testing purposes.
int innerGetTraceLevel(
  std::string const &scope,
  std::string const &spec,
  std::string /*OUT*/ &errorRE,
  std::string /*OUT*/ &errorMsg);


/* Placed at file or function scope, this initializes the trace level
   that applies to that scope, shadowing any outer scope tracing. */
#define INIT_TRACE(scope)                                        \
  static char const * const traceScope = scope;                  \
  static int traceLevel = getTraceLevel(traceScope) /* user ; */


// Placed in the class scope, this declares the trace variables.
#define DECLARE_CLASS_TRACE_VARS(className) \
  static char const * const traceScope;     \
  static int traceLevel /* user ; */


// Placed at file scope, this defines those variables.
#define DEFINE_CLASS_TRACE_VARS(className)                           \
  char const * const className::traceScope = #className;             \
  int className::traceLevel = getTraceLevel(#className) /* user ; */


// Macro to conditionally print based on verbosity level.
//
// This flushes after every message in order to ensure all output is
// seen before, say, crashing; and because performance is generally not
// a concern for debug trace messages.
//
// Note: It is deliberate that 'stuff' is not enclosed in parentheses
// because it will often contain additional '<<' operators that are
// meant to chain into the 'clog' output.
#define TRACE(level, stuff)                                          \
  if (traceLevel >= (level)) {                                       \
    std::clog << "### " << traceScope << ": " << stuff << std::endl; \
  }

// Level 0 is unconditional.  I use this for temporary debug printouts
// to have a uniform syntax with the other tracing levels.
#define TRACE0(stuff) TRACE(0, stuff)

// Various levels for convenience.
#define TRACE1(stuff) TRACE(1, stuff)
#define TRACE2(stuff) TRACE(2, stuff)
#define TRACE3(stuff) TRACE(3, stuff)
#define TRACE4(stuff) TRACE(4, stuff)


// Trace-print a particular expression.
#define TRACE_EXPR(level, expr) \
  TRACE(level, #expr ": " << (expr))

#define TRACE0_EXPR(stuff) TRACE_EXPR(0, stuff)
#define TRACE1_EXPR(stuff) TRACE_EXPR(1, stuff)
#define TRACE2_EXPR(stuff) TRACE_EXPR(2, stuff)
#define TRACE3_EXPR(stuff) TRACE_EXPR(3, stuff)
#define TRACE4_EXPR(stuff) TRACE_EXPR(4, stuff)


// Run unit tests, exit non-zero on error.  Defined in trace-test.cc.
void trace_unit_tests();


#endif // TRACE_H
