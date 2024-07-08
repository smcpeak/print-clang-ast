// pca-unit-tests.cc
// Code for `pca-unit-tests.h`.

#include "pca-unit-tests.h"            // this module

#include "clang-util.h"                // clang_util_unit_tests
#include "file-util.h"                 // file_util_unit_tests
#include "pca-command-line-options.h"  // pca_command_line_options_unit_tests
#include "pca-util.h"                  // pca_util_unit_tests
#include "stringref-parse.h"           // stringref_parse_unit_tests
#include "symbolic-line-mapper.h"      // symbolic_line_mapper_unit_tests


void clang_ast_visitor_nc_unit_tests();          // clang-ast-visitor-nc.test.cc


void pca_unit_tests()
{
  clang_util_unit_tests();
  clang_ast_visitor_nc_unit_tests();
  file_util_unit_tests();
  pca_command_line_options_unit_tests();
  pca_util_unit_tests();
  stringref_parse_unit_tests();
  symbolic_line_mapper_unit_tests();
}


// EOF
