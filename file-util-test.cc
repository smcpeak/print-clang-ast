// file-util-test.cc
// Tests for file-util.

#include "file-util.h"                           // module under test

#include <cstdlib>                               // std::getenv, std::exit
#include <iostream>                              // std::cout, etc.
#include <sstream>                               // std::ostringstream
#include <string>                                // std::string

#include <assert.h>                              // assert

using std::cerr;
using std::cout;
using std::string;


void file_util_unit_tests()
{
  string fname = "Makefile";

  // If set, this unit test acts like a crude version of 'cat'.  This
  // is mainly useful to let me exercise the error reporting manually.
  char const *altFname = std::getenv("FILE_UTIL_TEST_FNAME");
  if (altFname) {
    fname = altFname;
  }

  string contents;
  string err = readFile(contents /*OUT*/, fname);
  if (!err.empty()) {
    cerr << "error: " << err << "\n";
    std::exit(2);
  }

  if (!altFname) {
    // Get the first line.
    std::ostringstream os;
    for (char c : contents) {
      if (c == '\n') {
        break;
      }
      os << c;
    }
    assert(os.str() == "# print-clang-ast/Makefile");
  }
  else {
    // Print the entire thing and exit.
    cout << contents;
    std::exit(0);
  }
}

// EOF
