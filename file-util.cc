// file-util.cc
// Code for file-util.h.

#include "file-util.h"                           // this module

#include "util.h"                                // stringb, doubleQuote

#include <cstring>                               // std::strerror
#include <fstream>                               // std::ifstream
#include <sstream>                               // std::ostringstream

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
