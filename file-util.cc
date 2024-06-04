// file-util.cc
// Code for file-util.h.

#include "file-util.h"                           // this module

// this dir
#include "pca-util.h"                            // stringb

// smbase
#include "smbase/string-util.h"                  // doubleQuote
#include "smbase/stringb.h"                      // stringb

// libc++
#include <cstring>                               // std::strerror
#include <fstream>                               // std::ifstream
#include <sstream>                               // std::ostringstream

// libc
#include <errno.h>                               // errno


std::string readFile(std::string /*OUT*/ &contents,
                     std::string const &fname)
{
  std::ifstream inFile(fname.c_str(), std::ios::binary);
  if (!inFile) {
    return stringb(doubleQuote(fname) << ": " <<
                   std::strerror(errno));
  }

  // This is not very efficient...
  std::ostringstream os;
  os << inFile.rdbuf();
  contents = os.str();

  return "";
}


// EOF
