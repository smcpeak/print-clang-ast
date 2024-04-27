// print-clang-ast-nodes-private.h
// Private declarations for print-clang-ast-nodes module.

#ifndef PRINT_CLANG_AST_NODES_PRIVATE_H
#define PRINT_CLANG_AST_NODES_PRIVATE_H

#include "print-clang-ast-nodes.h"               // public decls for this module

#include "clang-util.h"                          // ClangUtil
#include "number-clang-ast-nodes.h"              // ClangASTNodeNumbering


/*
  Print AST node details.

  This class uses an initial seed set of nodes, created by (e.g.) a
  visitor based on RecursiveASTVisitor, and explores and prints
  everything reachable from them.  All nodes are numbered for stable
  run-to-run output (rather than relying on addresses, which can also
  optionally be printed).

  The purpose is to facilitate understanding the design of the clang AST
  by revealing (ideally) all of the data.  Since essentially all of that
  data is either 'private' or 'protected', and not always exposed via a
  public method, the implementation of this class uses some dirty tricks
  to subvert C++ access control.  As such, it's always operating at
  least somewhat in unsupported territory, both w.r.t. the clang API and
  the C++ language.

  The printing methods in this class are non-const for three reasons:

  1. Although the numbering object is currently separate, I'm
     considering folding it into this class.  The numbering changes as
     nodes are discovered.

  2. I regard the act of printing as conceptully being a side effect,
     even though the output stream is also a separate object.

  3. There are pass/fail assertion counts being maintained.
*/
class PrintClangASTNodes : public ClangUtil {
public:      // types
  typedef ClangASTNodeNumbering::NodeID NodeID;

public:      // data
  // Stream to write output to.
  std::ostream &m_os;

  // Printing options.
  PrintClangASTNodesConfiguration const &m_config;

  // Node numbering to use.
  //
  // This is non-const so I can lazily number some things that are hard
  // to traverse in advance, such as FunctionTemplateSpecializationInfo.
  //
  // This acts as a seed set for exploring the tree.  As new nodes are
  // found, the numbering is expanded.
  ClangASTNodeNumbering &m_numbering;

  // Map a stored Common to one of the FunctionTemplateDecls that has
  // it.  This is needed to interpret the contents of the Common.
  std::map<clang::FunctionTemplateDecl_Common const * /*common*/,
           clang::FunctionTemplateDecl const * /*decl*/>
    m_mapCommonToFunctionTemplateDecl;

  // Same for ClassTemplateDecls.
  std::map<clang::ClassTemplateDecl_Common const * /*common*/,
           clang::ClassTemplateDecl const * /*decl*/>
    m_mapCommonToClassTemplateDecl;

  // Number of invariant checks that passed and failed.
  int m_passedAssertions;
  int m_failedAssertions;

  // True if there is an open object in the output produced so far.
  bool m_objectIsOpen;

public:      // methods
  PrintClangASTNodes(std::ostream &os,
                     clang::ASTContext &astContext,
                     PrintClangASTNodesConfiguration const &config,
                     ClangASTNodeNumbering &numbering);

  ~PrintClangASTNodes();

  // Write the opening of a new output object.
  void openNewObject(std::string const &id);

  // Print the closing-brace of an object, if one is open.
  void closeOpenObjectIf();

  // Return 'shortForm' is 'm_config.m_printQualifiers', otherwise
  // return 'longForm'.
  std::string shortAndLongForms(
    std::string const &shortForm,
    std::string const &longForm) const;

  // Return 'qualifier' if 'm_config.m_printQualifiers'.  Otherwise,
  // return "".
  std::string ifLongForm(std::string const &qualifier) const;

  // Define a pair of methods to get the ID string for 'NodeType', the
  // second of which will add a numbering if needed.  In both cases,
  // the method here just delegates to 'm_numbering'.
  #define DEFINE_GET_IDSTR_METHODS(NodeType)                       \
    std::string get##NodeType##IDStr(                              \
      clang::NodeType const * NULLABLE node)                       \
        { return m_numbering.get##NodeType##IDStr(node); }         \
    std::string getOrCreate##NodeType##IDStr(                      \
      clang::NodeType const * NULLABLE node)                       \
        { return m_numbering.getOrCreate##NodeType##IDStr(node); }

  SM_PP_MAP_LIST(DEFINE_GET_IDSTR_METHODS,
    CLANG_AST_NODE_NUMBERING_TRACKED_TYPES)

  #undef DEFINE_GET_IDSTR_METHODS

  void printDeclContext(clang::DeclContext const *declContext);

  void printTemplateParameterList(
    std::string const &qualifier,
    std::string const &label,
    clang::TemplateParameterList const * NULLABLE params);

  void printTemplateArgumentList(
    std::string const &qualifier,
    std::string const &label,
    clang::TemplateArgumentList const &args);

  void printTemplateArgumentListInfo(
    std::string const &qualifier,
    std::string const &label,
    clang::TemplateArgumentListInfo const &args);

  void printASTTemplateArgumentListInfo(
    std::string const &qualifier,
    std::string const &label,
    clang::ASTTemplateArgumentListInfo const * NULLABLE args);

  void printTemplateArgument(
    std::string const &qualifier,
    std::string const &label,
    clang::TemplateArgument const &arg);

  void printTemplateName(
    std::string const &qualifier,
    std::string const &label,
    clang::TemplateName const &tname);

  void printTemplateArgumentLoc(
    std::string const &qualifier,
    std::string const &label,
    clang::TemplateArgumentLoc const *targLoc);

  // Print the information in 'dni.LocInfo'.
  //
  // 'dni' has two other fields, but at the places where I call this, I
  // have already printed them.  But the other fields are needed to know
  // how to interpret 'LocInfo'.
  void printDeclarationNameInfoLocInfo(
    std::string const &qualifier,
    std::string const &label,
    clang::DeclarationNameInfo dni);

  // Print all the fields.
  void printDeclarationNameInfo(
    std::string const &qualifier,
    std::string const &label,
    clang::DeclarationNameInfo dni);

  void printDeclGroupRef(
    std::string const &qualifier,
    std::string const &label,
    clang::DeclGroupRef dgr);

  void printCXXCtorInitializer(
    std::string const &qualifier,
    std::string const &label,
    clang::CXXCtorInitializer const *init);

  void printCXXBaseSpecifier(
    std::string const &qualifier,
    std::string const &label,
    clang::CXXBaseSpecifier const *bspec);

  // Print the details of 'typeLoc'.
  void printTypeLoc(
    std::string const &qualifier,
    std::string const &label,
    clang::TypeLoc typeLoc);

  // Get JSON for an object like:
  //
  //   {
  //     "ptr": "<ptrVal>",
  //     "preview": "<previewVal>"
  //   }
  //
  // The idea is the 'ptr' can be followed to get structural details,
  // while the 'preview' is shown before following the pointer, as a
  // clue to what that structure contains.
  //
  // This double-quotes and escapes 'ptrVal' and 'previewVal', so they
  // should not already be quoted/escaped.
  std::string ptrAndPreview(std::string const &ptrVal,
                            std::string const &previewVal);

  // Get JSON for pointer to 'type' along with a preview of its syntax.
  std::string typeIDSyntaxJson(clang::Type const * NULLABLE type);
  std::string qualTypeIDSyntaxJson(clang::QualType qualType);

  // Get ptr/preview for 'nns'.
  std::string nestedNameSpecifierIDSyntaxJson(
    clang::NestedNameSpecifier const * NULLABLE nns);

  // Search forward in the source code for the next token, starting at
  // the position right after 'loc'.  Return the spelling of the found
  // token, or "" if there is none.  If a token is found, update 'loc'
  // to point at its last character (such that another call will then
  // find the token after that).
  //
  // Note: This doesn't really work if there are preprocessor
  // directives, although it does skip comments.
  std::string getNextToken(
    clang::SourceLocation /*IN/OUT*/ &loc) const;

  // Attempt to search forward to the end of the "=delete" that we have
  // already determined should be there.  Returns an invalid location if
  // it cannot be found.
  clang::SourceLocation getEndOfEqDelete(
    clang::FunctionDecl const *decl);

  // Print 'ftsi'.
  void printFunctionTemplateSpecializationInfo(
    clang::FunctionTemplateSpecializationInfo const *ftsi);

  // Print 'common', given that it was associated with 'decl'.
  void printRedeclarableTemplateDecl_CommonBase(
    clang::RedeclarableTemplateDecl_CommonBase const *common,
    clang::RedeclarableTemplateDecl const *decl);

  // Print 'common'.
  void printFunctionTemplateDecl_Common(
    clang::FunctionTemplateDecl_Common const *common);
  void printClassTemplateDecl_Common(
    clang::ClassTemplateDecl_Common const *common);

  // Print 'msi'.
  void printMemberSpecializationInfo(
    clang::MemberSpecializationInfo const *msi);

  // Print 'dftsi'.
  void printDependentFunctionTemplateSpecializationInfo(
    clang::DependentFunctionTemplateSpecializationInfo const *dftsi);

  // Print a CXXRecordDecl::DefinitionData record.
  void printFake_CXXRecordDecl_DefinitionData(
    clang::Fake_CXXRecordDecl_DefinitionData const *fakeData);

  // Print the details of 'type'.
  void printType(clang::Type const *type);

  // Print all the nodes that have been collected into the maps.  As
  // they are printed, new nodes will be added, so print those too,
  // until everything reachable has been printed.
  void printAllNodes();

  // Print the 'Redeclarable' fields.
  template <class T>
  void printRedeclarable(clang::Redeclarable<T> const *decl);

  // After each of the following, there is a comment indicating the file
  // and line where it is declared.  I track this so I can easily
  // maintain this list in the same order as they are declared in the
  // clang headers.  The line numbers here come from Clang+LLVM 14.0.0.

  void printDecl(clang::Decl const *decl);                      // DeclBase.h line 83

  void printNamedDecl(clang::NamedDecl const *decl);            // Decl.h line  247
  void printValueDecl(clang::ValueDecl const *decl);            // Decl.h line  674
  void printDeclaratorDecl(clang::DeclaratorDecl const *decl);  // Decl.h line  726
  void printVarDecl(clang::VarDecl const *decl);                // Decl.h line  874
  void printParmVarDecl(clang::ParmVarDecl const *decl);        // Decl.h line 1663
  void printFunctionDecl(clang::FunctionDecl const *decl);      // Decl.h line 1855
  void printFieldDecl(clang::FieldDecl const *decl);            // Decl.h line 2838
  void printTypeDecl(clang::TypeDecl const *decl);              // Decl.h line 3141
  void printTagDecl(clang::TagDecl const *decl);                // Decl.h line 3331
  void printRecordDecl(clang::RecordDecl const *decl);          // Decl.h line 3866

  void printCXXRecordDecl(clang::CXXRecordDecl const *decl);    // DeclCXX.h line  254
  void printCXXMethodDecl(clang::CXXMethodDecl const *decl);    // DeclCXX.h line 1950
  void printCXXConstructorDecl(                                 // DeclCXX.h line 2403
    clang::CXXConstructorDecl const *decl);

  void printFriendDecl(clang::FriendDecl const *decl);          // DeclFriend.h line 53

  void printTemplateDecl(clang::TemplateDecl const *decl);      // DeclTemplate.h line  400
  void printRedeclarableTemplateDecl(                           // DeclTemplate.h line  753
    clang::RedeclarableTemplateDecl const *decl);
  void printFunctionTemplateDecl(                               // DeclTemplate.h line  979
    clang::FunctionTemplateDecl const *functionTemplateDecl);
  void printTemplateTypeParmDecl(                               // DeclTemplate.h line 1179
    clang::TemplateTypeParmDecl const *decl);
  void printClassTemplateSpecializationDecl(                    // DeclTemplate.h line 1803
    clang::ClassTemplateSpecializationDecl const *decl);
  void printClassTemplatePartialSpecializationDecl(             // DeclTemplate.h line 2075
    clang::ClassTemplatePartialSpecializationDecl const *decl);
  void printClassTemplateDecl(                                  // DeclTemplate.h line 2247
    clang::ClassTemplateDecl const *decl);
  void printClassScopeFunctionSpecializationDecl(               // DeclTemplate.h line 2604
    clang::ClassScopeFunctionSpecializationDecl const *decl);

  void printStmt(clang::Stmt const *stmt);                      // Stmt.h line   49
  void printDeclStmt(clang::DeclStmt const *stmt);              // Stmt.h line 1297
  void printCompoundStmt(clang::CompoundStmt const *stmt);      // Stmt.h line 1404
  void printValueStmt(clang::ValueStmt const *stmt);            // Stmt.h line 1785
  void printReturnStmt(clang::ReturnStmt const *stmt);          // Stmt.h line 2765

  void printExpr(clang::Expr const *expr);                      // Expr.h line  109
  void printDeclRefExpr(clang::DeclRefExpr const *expr);        // Expr.h line 1223
  void printCallExpr(clang::CallExpr const *expr);              // Expr.h line 2801
  void printMemberExpr(clang::MemberExpr const *expr);          // Expr.h line 3168
  void printCastExpr(clang::CastExpr const *expr);              // Expr.h line 3479
  void printImplicitCastExpr(                                   // Expr.h line 3624
    clang::ImplicitCastExpr const *expr);
  void printBinaryOperator(clang::BinaryOperator const *expr);  // Expr.h line 3809
  void printParenListExpr(clang::ParenListExpr const *expr);    // Expr.h line 5538

  void printCXXDefaultArgExpr(                                  // ExprCXX.h line 1241
    clang::CXXDefaultArgExpr const *expr);
  void printCXXConstructExpr(                                   // ExprCXX.h line 1460
    clang::CXXConstructExpr const *expr);
  void printCXXDependentScopeMemberExpr(                        // ExprCXX.h line 3550
    clang::CXXDependentScopeMemberExpr const *expr);

  void printAttr(clang::Attr const *attr);                      // Attr.h line 41

  void printNestedNameSpecifier(                                // NestedNameSpecifier.h line 50
    clang::NestedNameSpecifier const *nns);
};


#endif // PRINT_CLANG_AST_NODES_PRIVATE_H
