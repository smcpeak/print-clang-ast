// print-clang-ast.cc
// Entry point for print-clang-ast.exe program.

#include "decl-implicit.h"                                 // declareImplicitThings
#include "file-util.h"                                     // file_util_unit_tests
#include "map-util.h"                                      // mapInsertAll
#include "pca-command-line-options.h"                      // PCACommandLineOptions
#include "print-clang-ast-nodes.h"                         // printClangASTNodes
#include "sm-pp-util.h"                                    // test_sm_pp_util
#include "stringref-parse.h"                               // stringref_parse_unit_tests
#include "trace.h"                                         // INIT_TRACE, trace_unit_tests
#include "util.h"                                          // util_unit_tests

#include "clang/Basic/Diagnostic.h"                        // clang::DiagnosticsEngine
#include "clang/Basic/DiagnosticOptions.h"                 // clang::DiagnosticOptions
#include "clang/Basic/Version.h"                           // CLANG_VERSION_MAJOR
#include "clang/Driver/Driver.h"                           // clang::driver::Driver
#include "clang/Frontend/ASTUnit.h"                        // clang::ASTUnit
#include "clang/Frontend/CompilerInstance.h"               // clang::CompilerInstance
#include "clang/Frontend/Utils.h"                          // clang::createInvocationFromCommandLine
#include "clang/Serialization/PCHContainerOperations.h"    // clang::PCHContainerOperations

#include <string>                                          // std::string
#include <vector>                                          // std::vector

using std::cerr;
using std::clog;
using std::cout;
using std::string;


INIT_TRACE("print-clang-ast");


static void all_unit_tests()
{
  file_util_unit_tests();
  pca_command_line_options_unit_tests();
  stringref_parse_unit_tests();
  test_sm_pp_util();
  trace_unit_tests();
  util_unit_tests();
}


int main(int argc, char const **argv)
{
  // Command line options parser.
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

  // Copy the arguments into a vector of char pointers since that is
  // what 'createInvocationFromCommandLine' wants.
  std::vector<char const *> commandLine;
  {
    // Path to the 'clang' binary that I am behaving like.  This path is
    // used to compute the location of compiler headers like stddef.h.
    commandLine.push_back(CLANG_LLVM_INSTALL_DIR "/bin/clang");

    for (int i = firstClangArg; i < argc; ++i) {
      commandLine.push_back(argv[i]);
    }
  }

  // Parse the command line options.
  std::shared_ptr<clang::CompilerInvocation> compilerInvocation(
#if CLANG_VERSION_MAJOR <= 14
    clang::createInvocationFromCommandLine(llvm::makeArrayRef(commandLine)));
#else
    clang::createInvocation(llvm::ArrayRef(commandLine)));
#endif
  if (!compilerInvocation) {
    // Command line parsing errors have already been printed.
    return 2;
  }

  // Get the name of the primary source file.
  if (compilerInvocation->getFrontendOpts().Inputs.size() != 1) {
    // Unfortunately, this message is never seen because
    // createInvocation will choke first (and in the multiple input
    // case, spew a huge error message).
    cerr << "print-clang-ast: expected exactly 1 input file, not "
         << compilerInvocation->getFrontendOpts().Inputs.size()
         << "\n";
    return 2;
  }
  string primarySourceFileName =
    compilerInvocation->getFrontendOpts().Inputs[0].getFile().str();
  TRACE1("primarySourceFileName: " << primarySourceFileName);

  // Scan the file for additional options.
  err = options.parsePrimarySourceFile(primarySourceFileName);
  if (!err.empty()) {
    cerr << err << "\n";
    return 2;
  }
  TRACE1("options: " << options.getAsArgumentsString());

  // Boilerplate.
  std::shared_ptr<clang::PCHContainerOperations> pchContainerOps(
    new clang::PCHContainerOperations());

  // Arrange to print the option names with emitted diagnostics.
  clang::DiagnosticOptions *diagnosticOptions =
    &(compilerInvocation->getDiagnosticOpts());
  diagnosticOptions->ShowOptionNames = true;

  // The diagnostics engine created this way will simply print all
  // warnings and errors to stderr as they occur, and also maintain a
  // count of each.
  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticsEngine(
    clang::CompilerInstance::createDiagnostics(
      diagnosticOptions /*callee takes refcount ownership*/));

  // Run the Clang parser to produce an AST.
  std::unique_ptr<clang::ASTUnit> ast(
    clang::ASTUnit::LoadFromCompilerInvocationAction(
      compilerInvocation,
      pchContainerOps,
      diagnosticsEngine));

  if (ast == nullptr) {
    // Error messages should already have been printed, so this might
    // be redundant.
    cerr << "clang parse failed to produce an AST\n";
    return 2;
  }

  if (diagnosticsEngine->getNumErrors() > 0) {
    // The errors should have been printed already.
    cerr << "clang reported errors, stopping\n";
    return 2;
  }

  if (options.m_forceImplicit) {
    declareImplicitThings(ast.get());
  }

  if (options.m_dumpAST) {
    dumpClangAST(cout, ast->getASTContext());
  }

  if (options.m_printAST_JSON) {
    printClangAST_JSON(cout, ast->getASTContext());
  }

  if (options.m_printASTNodes) {
    PrintClangASTNodesConfiguration config;
    config.m_printNonPSFFileEntities = options.m_fullTU;
    config.m_printAddresses = !options.m_suppressAddresses;
    config.m_printQualifiers = !options.m_noASTFieldQualifiers;

    if (int failedAssertions =
          printClangASTNodes(cout, ast->getASTContext(), config)) {
      cerr << "Failed assertions: " << failedAssertions << "\n";
      return 2;
    }
  }

  return 0;
}

// EOF
