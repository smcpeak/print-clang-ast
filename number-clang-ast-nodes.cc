// number-clang-ast-nodes.cc
// Code for number-clang-ast-nodes module.

#include "number-clang-ast-nodes-private.h"      // private decls for this module

#include "pca-util.h"                            // stringb

#include "smbase/map-util.h"                     // mapInsertUnique
#include "smbase/sm-trace.h"                     // INIT_TRACE

#include "clang/Basic/LLVM.h"                    // clang::isa


using clang::dyn_cast;
using clang::isa;

using std::string;


// Avoid having to write the qualifier in return types.
typedef ClangASTNodeNumbering::NodeID NodeID;


INIT_TRACE("number-clang-ast-nodes");


// --------------- ClangASTNodeNumbering::NumberingMap -----------------
template <class T>
ClangASTNodeNumbering::NumberingMap<T>::NumberingMap(
  ClangASTNodeNumbering &numberingContainer,
  char const *defaultNodeTypeName)
:
  m_numberingContainer(numberingContainer),
  m_defaultNodeTypeName(defaultNodeTypeName),
  m_map(),
  m_inverseMap()
{}


template <class T>
ClangASTNodeNumbering::NumberingMap<T>::~NumberingMap()
{}


template <class T>
NodeID ClangASTNodeNumbering::NumberingMap<T>::insertUnique(
  T const *node)
{
  NodeID id = m_numberingContainer.getNextID();
  TRACE2("insertUnique: " << node << " -> " << id);
  mapInsertUnique(m_map, node, id);
  mapInsertUnique(m_inverseMap, id, node);
  return id;
}


template <class T>
NodeID ClangASTNodeNumbering::NumberingMap<T>::getExisting(
  T const *node) const
{
  auto it = m_map.find(node);
  assert(it != m_map.end());
  return (*it).second;
}


template <class T>
NodeID ClangASTNodeNumbering::NumberingMap<T>::get(
  T const *node)
{
  // I could make this a little more efficient by using insert and
  // checking its return value, but that would mean initially inserting
  // by peeking the next ID and then incrementing it if the insertion
  // succeeded, which is just a little messy, and factor of two speed
  // difference is irrelevant here.

  auto it = m_map.find(node);
  if (it == m_map.end()) {
    return insertUnique(node);
  }
  else {
    return (*it).second;
  }
}


template <class T>
std::string ClangASTNodeNumbering::NumberingMap<T>::nodeTypeName(
  T const * NULLABLE node) const
{
  return m_defaultNodeTypeName;
}


template <>
std::string
ClangASTNodeNumbering::NumberingMap<clang::Type>::nodeTypeName(
  clang::Type const * NULLABLE node) const
{
  if (node) {
    return string(node->getTypeClassName()) + "Type";
  }
  else {
    return "Type";
  }
}


template <>
std::string
ClangASTNodeNumbering::NumberingMap<clang::Decl>::nodeTypeName(
  clang::Decl const * NULLABLE node) const
{
  if (node) {
    return string(node->getDeclKindName()) + "Decl";
  }
  else {
    return "Decl";
  }
}


template <>
std::string
ClangASTNodeNumbering::NumberingMap<clang::Stmt>::nodeTypeName(
  clang::Stmt const * NULLABLE node) const
{
  if (node) {
    return node->getStmtClassName();
  }
  else {
    return "Stmt";
  }
}


// Specialize to avoid printing "Fake", and shorten a bit.
template <>
std::string
ClangASTNodeNumbering::NumberingMap<
               clang::Fake_CXXRecordDecl_DefinitionData>::nodeTypeName(
  clang::Fake_CXXRecordDecl_DefinitionData const * NULLABLE node) const
{
  return "DefinitionData";
}


template <>
std::string
ClangASTNodeNumbering::NumberingMap<
               clang::FunctionTemplateDecl_Common>::nodeTypeName(
  clang::FunctionTemplateDecl_Common const * NULLABLE node) const
{
  return "FunctionTemplateDecl::Common";
}


template <>
std::string
ClangASTNodeNumbering::NumberingMap<
               clang::ClassTemplateDecl_Common>::nodeTypeName(
  clang::ClassTemplateDecl_Common const * NULLABLE node) const
{
  return "ClassTemplateDecl::Common";
}


template <class T>
std::string ClangASTNodeNumbering::NumberingMap<T>::getExistingIDStr(
  T const * NULLABLE node) const
{
  string prefix = nodeTypeName(node);

  if (node) {
    NodeID id = getExisting(node);

    // Now that I'm outputting JSON, this string will be quoted, making
    // adequately unique for search purposes.  This should also look
    // better in the diagram editor.
    return stringb(prefix << " " << id);
  }
  else {
    // Previously, I used "<prefix>_null" here, but that is needlessly
    // verbose, especially in a diagram.
    return "null";
  }
}


template <class T>
std::string ClangASTNodeNumbering::NumberingMap<T>::getOrCreateIDStr(
  T const * NULLABLE node)
{
  // Ensure it exists first.
  if (node) {
    get(node);
  }

  // Then get its string.
  return getExistingIDStr(node);
}


// ---------------------- ClangASTNodeNumbering ------------------------
ClangASTNodeNumbering::ClangASTNodeNumbering()
  : m_nextID(1)

    #define INIT_MAP_DATA(NodeType) \
      , m_##NodeType##Map(*this, #NodeType)

    SM_PP_MAP_LIST(INIT_MAP_DATA,
      CLANG_AST_NODE_NUMBERING_TRACKED_TYPES)

    #undef INIT_MAP_DATA
{}


ClangASTNodeNumbering::~ClangASTNodeNumbering()
{}


NodeID ClangASTNodeNumbering::getNextID()
{
  return m_nextID++;
}


#define DEFINE_MAP_METHODS(NodeType)                               \
  NodeID ClangASTNodeNumbering::insertUnique##NodeType(            \
    clang::NodeType const *node)                                   \
  {                                                                \
    return m_##NodeType##Map.insertUnique(node);                   \
  }                                                                \
                                                                   \
  NodeID ClangASTNodeNumbering::getExisting##NodeType(             \
    clang::NodeType const *node) const                             \
  {                                                                \
    return m_##NodeType##Map.getExisting(node);                    \
  }                                                                \
                                                                   \
  NodeID ClangASTNodeNumbering::get##NodeType(                     \
    clang::NodeType const *node)                                   \
  {                                                                \
    return m_##NodeType##Map.get(node);                            \
  }                                                                \
                                                                   \
  std::string ClangASTNodeNumbering::get##NodeType##IDStr(         \
    clang::NodeType const * NULLABLE node)                         \
  {                                                                \
    return m_##NodeType##Map.getIDStr(node);                       \
  }                                                                \
                                                                   \
  std::string ClangASTNodeNumbering::getOrCreate##NodeType##IDStr( \
    clang::NodeType const * NULLABLE node)                         \
  {                                                                \
    return m_##NodeType##Map.getOrCreateIDStr(node);               \
  }

SM_PP_MAP_LIST(DEFINE_MAP_METHODS,
  CLANG_AST_NODE_NUMBERING_TRACKED_TYPES)

#undef DEFINE_MAP_METHODS


// ----------------------- NumberClangASTNodes -------------------------
NumberClangASTNodes::NumberClangASTNodes(
  clang::ASTContext &astContext,
  ClangASTNodeNumbering &numbering)
  : ClangUtil(astContext),
    m_numbering(numbering)
{}


NumberClangASTNodes::~NumberClangASTNodes()
{}


bool NumberClangASTNodes::VisitType(clang::Type *type)
{
  m_numbering.getType(type);
  return true;
}


bool NumberClangASTNodes::VisitDecl(clang::Decl *decl)
{
  // Previously, I was using 'insertUniqueDecl' here, but on the test
  // case nodes-function-requires-requires.cc, the visitor visits the
  // declaration of the constraint variable 'x' twice.  So, just ignore
  // a repeated visit of the same decl.
  m_numbering.getDecl(decl);

  if (auto functionDecl = dyn_cast<clang::FunctionDecl>(decl)) {
    // Due to a bug in clang:
    //
    //   https://github.com/llvm/llvm-project/issues/64820
    //
    // we need to explicitly visit the parameters of a FunctionDecl too,
    // since otherwise some of them are missed.
    if (functionDecl->decls_empty()) {
      for (unsigned i=0; i < functionDecl->getNumParams(); ++i) {
        m_numbering.getDecl(functionDecl->getParamDecl(i));
      }
    }
    else {
      // The bug happens when the declarations list is empty (and that
      // might in fact be the cause of it).  So if it's not empty, we
      // don't need the extra loop, and by not doing it, I can avoid
      // disturbing the order that RAV on its own would produce.
    }

    // Visit FTSI nodes from the FunctionDecl side.
    if (clang::FunctionTemplateSpecializationInfo *ftsi =
          functionDecl->getTemplateSpecializationInfo()) {
      m_numbering.getFunctionTemplateSpecializationInfo(ftsi);
    }
  }

  // This does not work without bypassing access control, which I do not
  // want to do here.  I guess I'll have to lazily number while
  // printing.
#if 0
  // Visit FTSI nodes from the FunctionTemplateDecl side.
  if (auto templateDecl =
        dyn_cast<clang::FunctionTemplateDecl>(decl)) {
    for (clang::FunctionTemplateSpecializationInfo *ftsi :
           templateDecl->specializations()) {
      m_numbering.getFunctionTemplateSpecializationInfo(ftsi);
    }
  }
#endif // 0

  return true;
}


bool NumberClangASTNodes::VisitStmt(clang::Stmt *stmt)
{
  m_numbering.getStmt(stmt);
  return true;
}


void numberClangASTNodes(
  clang::ASTContext &astContext,
  ClangASTNodeNumbering &numbering)
{
  NumberClangASTNodes numberer(astContext, numbering);
  numberer.TraverseDecl(astContext.getTranslationUnitDecl());
}


// EOF
