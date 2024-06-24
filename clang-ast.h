// clang-ast.h
// `ClangAST` class to encapsulate parsing, etc.

#ifndef PCA_CLANG_AST_H
#define PCA_CLANG_AST_H

#include "clang-frontendaction-fwd.h"            // clang::FrontendAction
#include "clang-util.h"                          // ClangUtil

#include "clang/Frontend/ASTUnit.h"              // clang::ASTUnit
#include "clang/Frontend/CompilerInvocation.h"   // clang::CompilerInvocation

#include "smbase/sm-macros.h"                    // NO_OBJECT_COPIES
#include "smbase/temporary-file.h"               // smbase::TemporaryFile

#include <memory>                                // std::{shared_ptr, unique_ptr}
#include <string>                                // std::string
#include <vector>                                // std::vector


// Encapsulate an AST, SourceManager, etc.
//
// This object is only meant for one-shot, parse and discard usage.  If
// another TU is to be parsed, make another object.
class ClangAST {
  NO_OBJECT_COPIES(ClangAST);

public:      // data
  // The invocation created by `parseCommandLine`.
  //
  // This has to be a `shared_ptr` rather than `unique_ptr` because the
  // former is what is accepted by `LoadFromCompilerInvocation`.
  std::shared_ptr<clang::CompilerInvocation> m_compilerInvocation;

  // The name of the primary source file as determined by
  // `parseCommandLine`.
  std::string m_primarySourceFileName;

  // Result of parsing the source code.
  std::unique_ptr<clang::ASTUnit> m_ast;

public:      // methods
  ~ClangAST();

  // Start with an empty AST.  The client can then call
  // `parseCommandLine` and `parseSourceCode`.
  ClangAST();

  // Parse the command line, then parse the code specified in that
  // command line.  Throw `XMessage` if there is a problem.
  ClangAST(std::vector<std::string> const &fnameAndArgs);

  // Parse the command line, which should *not* include the name of the
  // compiler, just the name of the primary source file and any other
  // compilation options.
  //
  // Return true on success.  On failure, return false after printing
  // error messages to stderr.
  bool parseCommandLine(std::vector<std::string> const &fnameAndArgs);

  // Parse the source file specified in a previous call to
  // `parseCommandLine`.  The result is placed in `m_ast`.
  //
  // Return true on success.  On failure, return false after printing
  // error messages to stderr.
  bool parseSourceCode(clang::FrontendAction *feAction = nullptr);

  // Get the `ASTUnit` after a successful parse.
  clang::ASTUnit *getASTUnit();

  // Get the `ASTContext` after a successful parse.
  clang::ASTContext &getASTContext();
};


// Combination of `ClangAST` and `ClangUtil`.
class ClangASTUtil : public ClangAST, public ClangUtil {
public:      // methods
  ~ClangASTUtil();

  // Parse the command line, then parse the code specified in that
  // command line.  Throw `XMessage` if there is a problem.
  ClangASTUtil(std::vector<std::string> const &fnameAndArgs);

  clang::ASTContext &getASTContext()
    { return ClangAST::getASTContext(); }
};


// Parse in-memory source code using a temporary file.
class ClangASTUtilTempFile : public smbase::TemporaryFile,
                             public ClangASTUtil {
public:      // methods
  ~ClangASTUtilTempFile();

  // Parse 'source'.
  ClangASTUtilTempFile(std::string const &source);
};


#endif // PCA_CLANG_AST_H
