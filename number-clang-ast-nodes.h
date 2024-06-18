// number-clang-ast-nodes.h
// Assign numeric IDs to clang AST nodes.

#ifndef NUMBER_CLANG_AST_NODES_H
#define NUMBER_CLANG_AST_NODES_H

#include "expose-template-common.h"              // clang::FunctionTemplateDecl_CommonBase
#include "pca-util.h"                            // NULLABLE

#include "clang/AST/ASTContext.h"                // clang::ASTContext
#include "clang/AST/ASTFwd.h"                    // clang::FunctionDecl [n]
#include "clang/AST/Attr.h"                      // clang::Attr [n]

#include "smbase/sm-macros.h"                    // NULLABLE
#include "smbase/sm-pp-util.h"                   // SM_PP_MAP_LIST

#include <map>                                   // std::map
#include <string>                                // std::string

#include <stdint.h>                              // uint64_t


// I would like to have an accurate type here, but it is private.
namespace clang {
  struct Fake_CXXRecordDecl_DefinitionData;
}


// The set of AST node types for which we create a numbering map.
#define CLANG_AST_NODE_NUMBERING_TRACKED_TYPES ( \
  Type,                                          \
  Decl,                                          \
  Stmt,                                          \
  Attr,                                          \
  NestedNameSpecifier,                           \
  FunctionTemplateSpecializationInfo,            \
  MemberSpecializationInfo,                      \
  DependentFunctionTemplateSpecializationInfo,   \
  FunctionTemplateDecl_Common,                   \
  ClassTemplateDecl_Common,                      \
  Fake_CXXRecordDecl_DefinitionData)


// A set of maps giving IDs to clang AST nodes.
class ClangASTNodeNumbering {
public:      // types
  // Type of an assigned node ID.
  typedef uint64_t NodeID;

  // Map from 'T const *' to unique ID.
  template <class T>
  class NumberingMap {
  public:      // instance data
    // Parent container, for access to 'm_nextID'.
    ClangASTNodeNumbering &m_numberingContainer;

    // Value to use for 'nodeTypeName' unless overridden with an
    // explicit specialization
    char const *m_defaultNodeTypeName;

    // Map the node pointer to its ID.
    std::map<T const *, NodeID> m_map;

    // Inverse map, from ID to node.
    std::map<NodeID, T const *> m_inverseMap;

  public:      // methods
    NumberingMap(ClangASTNodeNumbering &numberingContainer,
                 char const *defaultNodeTypeName);
    ~NumberingMap();

    // Add a new node, asserting it is not already present.
    NodeID insertUnique(T const *node);

    // Get the ID for 'node', asserting it is present.
    NodeID getExisting(T const *node) const;

    // Get the ID of 'node', adding it if needed.
    NodeID get(T const *node);

    // The type name to use in 'getIDStr' to describe the node.
    std::string nodeTypeName(T const * NULLABLE node) const;

    // Return a string like "<nodeTypeName>_123_" for 'node', which has
    // already been numbered.  If 'node' is null, returns
    // "<nodeTypeName>_null".
    std::string getExistingIDStr(T const * NULLABLE node) const;

    // Similar, but create the numbering for 'node' if needed.
    std::string getIDStr(T const * NULLABLE node);
  };

public:      // data
  // Next ID to assign.  Starts at 1 and increases with each assignment,
  // such that 0 can be used to mean "absent".
  NodeID m_nextID;

  // Maps for each type of node we track.  The idea is to track any AST
  // node that is potentially shared by multiple other nodes.
  #define DECLARE_MAP_DATA(NodeType) \
    NumberingMap<clang::NodeType> m_##NodeType##Map;

  SM_PP_MAP_LIST(DECLARE_MAP_DATA,
    CLANG_AST_NODE_NUMBERING_TRACKED_TYPES)

  #undef DECLARE_MAP_DATA

public:
  ClangASTNodeNumbering();
  ~ClangASTNodeNumbering();

  // Get the next ID, incrementing the counter.
  NodeID getNextID();

  // Declare the methods for numbering 'NodeType'.  These just relay to
  // those in 'NumberingMap', so see the comments there for semantics.
  #define DECLARE_MAP_METHODS(NodeType)                                      \
    NodeID insertUnique##NodeType(clang::NodeType const *node);              \
    NodeID getExisting##NodeType(clang::NodeType const *node) const;         \
    NodeID get##NodeType(clang::NodeType const *node);                       \
    std::string get##NodeType##IDStr(clang::NodeType const * NULLABLE node);

  SM_PP_MAP_LIST(DECLARE_MAP_METHODS,
    CLANG_AST_NODE_NUMBERING_TRACKED_TYPES)

  #undef DECLARE_MAP_METHODS
};


// Populate 'numbering' with the nodes in 'astContext'.
void numberClangASTNodes(
  clang::ASTContext &astContext,
  ClangASTNodeNumbering &numbering);


#endif // NUMBER_CLANG_AST_NODES_H
