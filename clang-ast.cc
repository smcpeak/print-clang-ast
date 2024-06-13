// clang-ast.cc
// Code for `clang-ast.h`.

#include "clang-ast.h"                                     // this module

#include "clang/Basic/Diagnostic.h"                        // clang::DiagnosticsEngine
#include "clang/Basic/DiagnosticOptions.h"                 // clang::DiagnosticOptions
#include "clang/Basic/Version.h"                           // CLANG_VERSION_MAJOR
#include "clang/Frontend/ASTUnit.h"                        // clang::ASTUnit
#include "clang/Frontend/CompilerInstance.h"               // clang::CompilerInstance
#include "clang/Frontend/Utils.h"                          // clang::createInvocationFromCommandLine
#include "clang/Serialization/PCHContainerOperations.h"    // clang::PCHContainerOperations

#include "smbase/exc.h"                                    // smbase::xmessage

using namespace smbase;


// ----------------------------- ClangAST ------------------------------
ClangAST::~ClangAST()
{}


ClangAST::ClangAST()
  : m_compilerInvocation(),
    m_primarySourceFileName(),
    m_ast()
{}


ClangAST::ClangAST(std::vector<std::string> const &fnameAndArgs)
  : ClangAST()
{
  if (!parseCommandLine(fnameAndArgs)) {
    xmessage("Failed to parse command line.");
  }

  if (!parseSourceCode()) {
    xmessage("Failed to parse source code.");
  }
}


bool ClangAST::parseCommandLine(
  std::vector<std::string> const &fnameAndArgs)
{
  // Copy the arguments into a vector of char pointers since that is
  // what 'createInvocationFromCommandLine' wants.
  std::vector<char const *> commandLine;
  {
    // Path to the 'clang' binary that I am behaving like.  This path is
    // used to compute the location of compiler headers like stddef.h.
    commandLine.push_back(CLANG_LLVM_INSTALL_DIR "/bin/clang");

    for (std::string const &s : fnameAndArgs) {
      commandLine.push_back(s.c_str());
    }
  }

  // Parse the command line options.
  m_compilerInvocation =
#if CLANG_VERSION_MAJOR <= 14
    clang::createInvocationFromCommandLine(llvm::makeArrayRef(commandLine));
#else
    clang::createInvocation(llvm::ArrayRef(commandLine));
#endif
  if (!m_compilerInvocation) {
    // Command line parsing errors have already been printed.
    return false;
  }

  // Get the name of the primary source file.
  if (m_compilerInvocation->getFrontendOpts().Inputs.size() != 1) {
    // Unfortunately, this message is never seen because
    // createInvocation will choke first (and in the multiple input
    // case, spew a huge error message).
    cerr << "expected exactly 1 input file, not "
         << m_compilerInvocation->getFrontendOpts().Inputs.size()
         << "\n";
    return false;
  }
  m_primarySourceFileName =
    m_compilerInvocation->getFrontendOpts().Inputs[0].getFile().str();

  return true;
}


bool ClangAST::parseSourceCode()
{
  // Boilerplate.
  std::shared_ptr<clang::PCHContainerOperations> pchContainerOps(
    new clang::PCHContainerOperations());

  // Arrange to print the option names with emitted diagnostics.
  clang::DiagnosticOptions *diagnosticOptions =
    &(m_compilerInvocation->getDiagnosticOpts());
  diagnosticOptions->ShowOptionNames = true;

  // The diagnostics engine created this way will simply print all
  // warnings and errors to stderr as they occur, and also maintain a
  // count of each.
  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticsEngine(
    clang::CompilerInstance::createDiagnostics(
      // Because we are passing a `DiagnosticOptions` here, rather than
      // a `DiagnosticConsumer`, we retain ownership of it.
      diagnosticOptions));

  // Run the Clang parser to produce an AST.
  m_ast.reset(
    clang::ASTUnit::LoadFromCompilerInvocationAction(
      m_compilerInvocation,
      pchContainerOps,
      diagnosticsEngine));

  if (m_ast == nullptr) {
    // Error messages should already have been printed.
    return false;
  }

  if (diagnosticsEngine->getNumErrors() > 0) {
    // The errors should have been printed already.
    return false;
  }

  return true;
}


clang::ASTUnit *ClangAST::getASTUnit()
{
  return m_ast.get();
}


clang::ASTContext &ClangAST::getASTContext()
{
  return m_ast->getASTContext();
}


// --------------------------- ClangASTUtil ----------------------------
ClangASTUtil::~ClangASTUtil()
{}


ClangASTUtil::ClangASTUtil(std::vector<std::string> const &fnameAndArgs)
  : ClangAST(fnameAndArgs),
    ClangUtil(m_ast->getASTContext())
{}


// EOF
