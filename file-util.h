// file-util.h
// File-oriented utilities.

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <string>                                // std::string


// Read 'fname' into memory, putting the result in 'contents'.  On
// error, return an error message (otherwise "").
std::string readFile(std::string /*OUT*/ &contents,
                     std::string const &fname);

// Unit tests, defined in file-util-test.cc.
void file_util_unit_tests();

#endif // FILE_UTIL_H
