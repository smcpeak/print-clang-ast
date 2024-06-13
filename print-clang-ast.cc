// print-clang-ast.cc
// Entry point for print-clang-ast.exe program.

#include "clang-ast.h"                                     // ClangAST
#include "clang-util.h"                                    // clang_util_unit_tests
#include "decl-implicit.h"                                 // declareImplicitThings
#include "file-util.h"                                     // file_util_unit_tests
#include "pca-command-line-options.h"                      // PCACommandLineOptions
#include "print-clang-ast-nodes.h"                         // printClangASTNodes
#include "printer-visitor.h"                               // printerVisitorTU
#include "rav-printer-visitor.h"                           // ravPrinterVisitorTU
#include "stringref-parse.h"                               // stringref_parse_unit_tests
#include "pca-util.h"                                      // pca_util_unit_tests

#include "smbase/gdvalue.h"                                // gdv::GDValue
#include "smbase/map-util.h"                               // mapInsertAll
#include "smbase/sm-trace.h"                               // INIT_TRACE
#include "smbase/string-util.h"                            // stringVectorFromPointerArray

#include <exception>                                       // std::exception
#include <string>                                          // std::string
#include <vector>                                          // std::vector

using std::cerr;
using std::clog;
using std::cout;
using std::string;


INIT_TRACE("print-clang-ast");


static void all_unit_tests()
{
  clang_util_unit_tests();
  file_util_unit_tests();
  pca_command_line_options_unit_tests();
  stringref_parse_unit_tests();
  pca_util_unit_tests();
}


static int innerMain(int argc, char const **argv)
{
  // When printing GDValues, for example as part of test failures, use
  // indentation.
  gdv::GDValue::s_defaultWriteOptions.m_enableIndentation = true;

  // Command line options parser for the options that precede those
  // intended for clang.
  PCACommandLineOptions options;
  int firstClangArg = 1;
  string err = options.parseCommandLine(firstClangArg, argc, argv);
  if (!err.empty()) {
    cerr << err << "\n";
    return 2;
  }

  if (options.m_printUsage) {
    options.printUsage(argv[0]);
    return 0;
  }

  if (options.m_runUnitTests) {
    all_unit_tests();
    cout << "unit tests passed\n";
    return 0;
  }

  ClangAST ast;
  if (!ast.parseCommandLine(
         stringVectorFromPointerArray(argc - firstClangArg,
                                      argv + firstClangArg))) {
    return 2;
  }

  // Get the name of the primary source file.
  clang::FrontendOptions const &feOpts =
    ast.m_compilerInvocation->getFrontendOpts();
  if (feOpts.Inputs.size() != 1) {
    // Unfortunately, this message is never seen because
    // createInvocation will choke first (and in the multiple input
    // case, spew a huge error message).
    cerr << "print-clang-ast: expected exactly 1 input file, not "
         << feOpts.Inputs.size()
         << "\n";
    return 2;
  }
  string primarySourceFileName = feOpts.Inputs[0].getFile().str();
  TRACE1("primarySourceFileName: " << primarySourceFileName);

  // Scan the file for additional options.
  err = options.parsePrimarySourceFile(primarySourceFileName);
  if (!err.empty()) {
    cerr << err << "\n";
    return 2;
  }
  TRACE1("options: " << options.getAsArgumentsString());

  if (!ast.parseSourceCode()) {
    return 2;
  }

  if (options.m_forceImplicit) {
    declareImplicitThings(ast.getASTUnit(), true /*defineAlso*/);
  }

  if (options.m_dumpAST) {
    dumpClangAST(cout, ast.getASTContext());
  }

  if (options.m_printAST_JSON) {
    printClangAST_JSON(cout, ast.getASTContext());
  }

  if (options.m_printerVisitor) {
    PrinterVisitor::Flags flags = PrinterVisitor::F_NONE;
    if (options.m_printVisitContext) {
      flags |= PrinterVisitor::F_PRINT_VISIT_CONTEXT;
    }
    if (options.m_printImplicitQualTypes) {
      flags |= PrinterVisitor::F_PRINT_IMPLICIT_QUAL_TYPES;
    }
    if (options.m_omit_CTPSD_TAW) {
      flags |= PrinterVisitor::F_OMIT_CTPSD_TAW;
    }
    if (options.m_printDefaultArgExprs) {
      flags |= PrinterVisitor::F_PRINT_DEFAULT_ARG_EXPRS;
    }

    printerVisitorTU(cout,
                     ast.getASTContext(),
                     flags);
  }

  if (options.m_ravPrinterVisitor) {
    ravPrinterVisitorTU(cout, ast.getASTContext());
  }

  if (options.m_printASTNodes) {
    PrintClangASTNodesConfiguration config;
    config.m_printNonPSFFileEntities = options.m_fullTU;
    config.m_printAddresses = !options.m_suppressAddresses;
    config.m_printQualifiers = !options.m_noASTFieldQualifiers;

    if (int failedAssertions =
          printClangASTNodes(cout, ast.getASTContext(), config)) {
      cerr << "Failed assertions: " << failedAssertions << "\n";
      return 2;
    }
  }

  return 0;
}


int main(int argc, char const **argv)
{
  try {
    return innerMain(argc, argv);
  }
  catch (std::exception &x) {
    std::cerr << x.what() << "\n";
    return 2;
  }
}


// EOF
