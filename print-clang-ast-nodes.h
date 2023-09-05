// print-clang-ast-nodes.h
// Print the clang AST with details for every node.

#ifndef PRINT_CLANG_AST_NODES_H
#define PRINT_CLANG_AST_NODES_H

#include "clang/AST/ASTContext.h"                // clang::ASTContext
#include "clang/AST/ASTFwd.h"                    // clang::FunctionDecl [n]

#include <iosfwd>                                // std::ostream


// Configuration for AST printing.
class PrintClangASTNodesConfiguration {
public:
  // True to print entities that are not in the primary source file.
  //
  // TODO: This currently does not do anything.
  bool m_printNonPSFFileEntities = false;

  // True to print the numeric addresses of nodes.
  bool m_printAddresses = true;

  // True to print qualifiers in front of field names to clarify which
  // class declares them.
  bool m_printQualifiers = true;
};


// Print the AST in the "dump" format to 'os'.
void dumpClangAST(
  std::ostream &os,
  clang::ASTContext &astContext);

// Print the AST in the JSON format to 'os'.
void printClangAST_JSON(
  std::ostream &os,
  clang::ASTContext &astContext);


// Print the translation unit in 'astContext' to 'os'.  Unlike the
// "dump" format, this prints a paragraph-like section for each node,
// and does not use indentation to reflect the nesting relation between
// AST nodes.  The idea is, usually, to print the dump first as an
// overview and then follow it with the node details so they can be used
// together.
//
// If the AST comes from an external source
// ('astContext.getExternalSource()' is not null), printing the AST will
// also load the parts that are printed.
//
// Returns the number of invariant checks that failed.
//
int printClangASTNodes(
  std::ostream &os,
  clang::ASTContext &astContext,
  PrintClangASTNodesConfiguration const &config);


#endif // PRINT_CLANG_AST_NODES_H
