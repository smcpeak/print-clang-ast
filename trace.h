// trace.h
// Debug trace support.

#ifndef TRACE_H
#define TRACE_H

#include <iostream>                    // std::clog, std::endl

#include <cstdlib>                     // std::getenv, std::atoi


/*
  A tracing control within each including module.  Typical meanings:

    -1: Not initialized.
     0: No output.
     1: Things directly related to module output.
     2: Important control flow.
     3: Algorithm details.
     4: Data structure dumps, etc.
*/
static int s_moduleTraceLevel = -1;


// This should be invoked somewhere in the module before tracing is
// done.  It is fine to invoke it repeatedly.  'envvarName' is a string
// literal giving the name of the environment variable that sets this
// module's tracing level.
#define INIT_TRACE_LEVEL_ONCE(envvarName)          \
  if (s_moduleTraceLevel < 0) {                    \
    if (char const *s = std::getenv(envvarName)) { \
      s_moduleTraceLevel = std::atoi(s);           \
    }                                              \
    else {                                         \
      s_moduleTraceLevel = 0;                      \
    }                                              \
  }


// Macro to conditionally print based on verbosity level.
//
// This flushes after every message in order to ensure all output is
// seen before, say, crashing; and because performance is generally not
// a concern for debug trace messages.
#define TRACE(level, stuff)            \
  if (s_moduleTraceLevel >= (level)) { \
    std::clog << stuff << std::endl;   \
  }

// Level 0 is unconditional.  I use this for temporary debug printouts
// to have a uniform syntax with the other tracing levels.
#define TRACE0(stuff) TRACE(0, stuff)

// Various levels for convenience.
#define TRACE1(stuff) TRACE(1, stuff)
#define TRACE2(stuff) TRACE(2, stuff)
#define TRACE3(stuff) TRACE(3, stuff)
#define TRACE4(stuff) TRACE(4, stuff)


#endif // TRACE_H
