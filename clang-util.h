// clang-util.h
// ClangUtil, a class with various Clang utilities.

#ifndef HEADER_ANALYSIS_CLANG_UTIL_H
#define HEADER_ANALYSIS_CLANG_UTIL_H

// this dir
#include "clang-expr-concepts-fwd.h"                       // clang::concepts::Requirement [n]
#include "pca-util.h"                                      // NULLABLE

// clang
#include "clang/AST/ASTContext.h"                          // clang::ASTContext::{getLangOpts, getSourceManager, getTranslationUnitDecl}
#include "clang/AST/ASTFwd.h"                              // clang::{Type, Stmt, Decl} [n]
#include "clang/AST/Decl.h"                                // clang::NamedDecl
#include "clang/AST/DeclTemplate.h"                        // clang::TemplateParameterList
#include "clang/AST/ExprCXX.h"                             // clang::CXXNewInitializationStyle
#include "clang/AST/ExternalASTSource.h"                   // clang::ExternalASTSource
#include "clang/AST/Type.h"                                // clang::QualType
#include "clang/Basic/Lambda.h"                            // clang::LambdaCaptureDefault
#include "clang/Basic/OperatorKinds.h"                     // clang::OverloadedOperatorKind
#include "clang/Basic/SourceLocation.h"                    // clang::FileID
#include "clang/Basic/SourceManager.h"                     // clang::SourceManager
#include "clang/Basic/Specifiers.h"                        // clang::InClassInitStyle
#include "clang/Basic/Version.h"                           // CLANG_VERSION_MAJOR
#include "clang/Lex/HeaderSearchOptions.h"                 // clang::HeaderSearchOptions

// smbase
#include "smbase/save-restore.h"                           // SetRestore
#include "smbase/sm-macros.h"                              // NULLABLE

// llvm
#include "llvm/ADT/StringRef.h"                            // llvm::StringRef [n]
#include "llvm/Support/Casting.h"                          // llvm::dyn_cast

// libc++
#include <list>                                            // std::list
#include <string>                                          // std::string
#include <vector>                                          // std::vector


// Expand to 'thenCode' if we're compiling against Clang+LLVM 17 or
// later.
//
// Beware: In some cases, I'm not sure which version introduced a
// change, so there may be cases that need adjustment, particularly when
// the change was made in an earlier version than the one indicated by
// my use of the macro.  Furthermore, I often have not tested the
// 'elseCode', I merely have reason to believe it should work.  The
// basic pattern is I insert IF_CLANG_17 when I am using Clang-17 and
// ran into a compatibility issue.
//
#if CLANG_VERSION_MAJOR >= 17
  #define IF_CLANG_17(thenCode, elseCode) thenCode
#else
  #define IF_CLANG_17(thenCode, elseCode) elseCode
#endif

// Similar for Clang-16.
#if CLANG_VERSION_MAJOR >= 16
  #define IF_CLANG_16(thenCode, elseCode) thenCode
#else
  #define IF_CLANG_16(thenCode, elseCode) elseCode
#endif


// The idea is to populate this with data for a single TU, and it can
// provide services (especially printing services) on top of Clang's TU
// services.  I then typically have a class that does some analysis
// inherit this one.
//
// Methods in this class are `static` when the ASTContext is not
// actually needed, and (should usually be) `const` otherwise.
// However, making them `static` precludes adding some kinds of debug
// statements so I'm gravitating toward not using `static` methods.
//
class ClangUtil {
public:      // class data
  // Instance for use in printing or other situations where it is
  // troublesome to explicitly pass an object (which is otherwise
  // preferred).  Some care is required when setting this since it can
  // only work with one TU at a time.  Initially `nullptr`.
  static ClangUtil const * NULLABLE s_instance;

public:      // instance data
  // Main result of parsing.  This is not 'const' so I can get a
  // non-const SourceManager.
  clang::ASTContext &m_astContext;

  // Source Manager in 'm_astContext'.  This is not 'const' because
  // clang::Rewriter requires a non-const reference.
  clang::SourceManager &m_srcMgr;

  // Main file ID in 'm_astContext'.
  clang::FileID m_mainFileID;

  // Name of the main file.
  std::string m_mainFileName;

  // Default printing policy.
  clang::PrintingPolicy m_printingPolicy;

public:      // methods
  explicit ClangUtil(clang::ASTContext &astContext);

  // Get the static instance, asserting it is not `nullptr`.
  static ClangUtil const &getInstance();

  // --------------------------- ASTContext ----------------------------
  clang::ASTContext &getASTContext() const { return m_astContext; }
  clang::SourceManager &getSourceManager() const { return m_srcMgr; }

  // Get the LangOptions in 'm_astContext'.
  clang::LangOptions const &getLangOptions() const;

  // Get the associated external AST source, if any.
  clang::ExternalASTSource * NULLABLE getExternalSource() const;

  // ------------------------- SourceLocation --------------------------
  // Render 'loc' as a string.
  std::string locStr(clang::SourceLocation loc) const;

  // Return a string like "L:C" where L is the line number and C is the
  // column number in 'loc'.
  std::string locLineColStr(clang::SourceLocation loc) const;

  // Get the line/column number only from 'loc'.
  unsigned locLine(clang::SourceLocation loc) const;
  unsigned locCol(clang::SourceLocation loc) const;

  // Return the base name (i.e., without path information) of the file
  // component of `loc`, or "<none>" if there is no associated file.
  std::string basenameOfLocFile(clang::SourceLocation loc) const;

  // True if 'loc' is in a source file, as opposed to a macro expansion
  // or on the command line.
  bool locInSourceFile(clang::SourceLocation loc) const;

  // True if 'loc' is in the main file of the translation unit.
  bool inMainFile(clang::SourceLocation loc) const;

  // Return the entry corresponding to the expansion location of 'loc',
  // or null if there is none.
  clang::FileEntry const * NULLABLE getFileEntryForLoc(
    clang::SourceLocation loc) const;

  // Return the location for a given file/line/col.
  clang::SourceLocation getLoc(clang::FileID fileID,
                               int line, int col) const;

  // Return the location for line/col in the TU main file.
  clang::SourceLocation getMainFileLoc(int line, int col) const;

  // Assuming that `loc` is the start of a token, return the location of
  // the character just past the end of that token.
  clang::SourceLocation locAfterToken(clang::SourceLocation loc) const;

  // --------------------------- SourceRange ---------------------------
  // Stringify 'range'.
  std::string sourceRangeStr(clang::SourceRange range) const;

  // ---------------------------- FileEntry ----------------------------
  // True if `entry` is the main file.
  bool isMainFileEntry(clang::FileEntry const *entry) const;

  // Return the file name in 'entry'.
  static std::string fileEntryNameStr(clang::FileEntry const *entry);

  // Write the file name to 'os' as a double-quoted string.
  static void fileEntryNameToJSON(std::ostream &os,
                                  clang::FileEntry const *entry);

  // Append to 'lines' all of the source code lines of the primary
  // source file.  Each will end with a newline character, except
  // possibly for the last.
  //
  // Return false, and leave 'lines' unchanged, if the code is not
  // available (typically because the AST was loaded from a serialized
  // AST file rather than created by parsing source code).  Otherwise
  // return true.
  //
  // This routine is not particularly efficient, but it has a simple
  // interface.
  //
  bool getPrimarySourceFileLines(std::vector<std::string> &lines);

  // ----------------------------- FileId ------------------------------
  // Turn a FileID into a string.
  std::string getFnameForFileID(clang::FileID fileID) const;

  // If 'loc' is an expansion location, get the place where the
  // expansion happened; otherwise use it as-is.  Then get the FileID
  // from it.
  clang::FileID getExpansionFileID(clang::SourceLocation loc) const;

  // ------------------------------ Decl -------------------------------
  /* Get the location of the beginning of 'decl'.  In particular, if
     'decl' is a definition of something that was previously declared
     elsewhere, I want the location of that definition, whereas
     Decl::getLocation() sometimes returns the declaration location.
     See the comments in the implementation for more details.

     In general, I should be using this rather than Decl::getLocation()
     or Decl::getBeginLoc() unless there is a known, specific reason.

     Note that many Stmt subclasses have a getLocation() method that, as
     far as I know, is fine to use.
  */
  static clang::SourceLocation declLoc(clang::Decl const *decl);

  // As a minor convenience, return an invalid location for `nullptr`,
  // and otherwise do the same as `declLoc`.
  static clang::SourceLocation declLocOpt(
    clang::Decl const * NULLABLE decl);

  // Get the location of the identifier for 'decl'.
  static clang::SourceLocation getIdentifierLoc(
    clang::NamedDecl const *decl);

  // Render the location of 'decl' as a string.
  std::string declLocStr(clang::Decl const *decl) const;

  // Return a string with the declaration kind and source location.
  std::string declKindAtLocStr(clang::Decl const * NULLABLE decl) const;

  // If 'decl' is a NamedDecl, then act like 'namedDeclAndKindAtLocStr',
  // otherwise act like 'declKindAtLocStr'.
  std::string declKindMaybeNameAtLocStr(
    clang::Decl const * NULLABLE decl) const;

  // Render `decl` with qualifiers and signature, but not quoted.
  // Returns "(null)" for `nullptr`.
  std::string namedDeclStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Stringify the quoted declaration syntax and its location.
  std::string namedDeclAtLocStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Stringify the quoted declaration syntax, its kind, and its
  // location.
  std::string namedDeclAndKindAtLocStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Like 'namedDeclAtLocStr' but include the address.
  std::string namedDeclAddrAtLocStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Print kind, location, and address of 'decl'.  I include "unnamed"
  // in the method name because this is not what should normally be
  // used, since normally we have a NamedDecl.
  std::string unnamedDeclAddrAtLocStr(
    clang::Decl const * NULLABLE decl) const;

  // Return a string that identifies the entity declared by 'namedDecl'
  // in a compact but reasonably unambiguous in the context of an
  // automated test.  See comments at the implementation for details.
  std::string namedDeclCompactIdentifier(
    clang::NamedDecl const *namedDecl) const;

  // Return the lexical parent of `decl` as a `Decl` rather than a
  // `DeclContext`.  Returns `nullptr` if it has no lexical parent
  // because it is the `TranslationUnitDecl`.
  clang::Decl const *getParentDeclOpt(clang::Decl const *decl) const;

  // Get the nearest lexical ancestor of `decl` that is a `NamedDecl`, or
  // `nullptr` if there is none.
  clang::NamedDecl const * NULLABLE getNamedParentDeclOpt(
    clang::Decl const *decl) const;

  /* Get the nearest `NamedDecl` ancestor of `decl`, except if `decl` is
     a template parameter, then Clang says the parent is the template
     body declaration, so adjust that to instead yield the
     `TemplateDecl` (which is not classified as a `DeclContext`).
  */
  clang::NamedDecl const * NULLABLE
  getNamedParentDeclOpt_templateAdjustment(clang::Decl const *decl) const;

  // True if `decl` is one of the three kinds of template parameter
  // declarations.
  static bool isTemplateParameterDecl(clang::Decl const *decl);

  // True if one of the lexical proper ancestors of `decl` is a
  // `FunctionDecl`.
  bool hasProperAncestorFunction(clang::Decl const *decl) const;

  // If 'decl' is a member, return its containing class.  Otherwise,
  // return 'nullptr'.
  clang::RecordDecl const *maybeGetParentClassC(
    clang::NamedDecl const *decl) const;
  clang::RecordDecl       *maybeGetParentClass (
    clang::NamedDecl       *decl) const;

  // Get the innermost enclosing parent that has a name suitable for use
  // in a qualifier, or nullptr if there is none.
  clang::NamedDecl const *maybeGetNamedParentC(
    clang::NamedDecl const *decl) const;
  clang::NamedDecl       *maybeGetNamedParent(
    clang::NamedDecl       *decl) const;

  // If 'decl' is something whose declaration is introduced with a
  // keyword like "class" or "namespace", return that keyword.
  // Otherwise, return "".
  std::string getDeclKeyword(clang::NamedDecl const *decl) const;

  // Get the canonical declaration for 'decl'.
  //
  // This method is basically just confirming that the canonical decl will
  // also be a NamedDecl, since the Clang API does not ensure that through
  // its declared types.
  clang::NamedDecl const *canonicalNamedDeclC(
    clang::NamedDecl const *decl) const;
  clang::NamedDecl       *canonicalNamedDecl(
    clang::NamedDecl       *decl) const;

  // Is 'decl' the declaration of an operator?
  bool isOperatorDecl(clang::NamedDecl const *decl) const;

  // True if `decl` is anonymous.
  static bool isAnonymous(clang::NamedDecl const *decl);

  // True if `decl` is among the kinds that supports the
  // `isThisDeclarationADefinition` method, and it returns true.
  static bool isThisDeclarationADefinition(clang::Decl const *decl);

  // If `decl` has a definition, get it.
  clang::NamedDecl const * NULLABLE getDefnForDeclOpt(
    clang::NamedDecl const * NULLABLE decl) const;

  // If `decl` has a separate definition, return it.  Otherwise return
  // `decl`.
  clang::NamedDecl const *getDefnOrSelfForDecl(
    clang::NamedDecl const *decl) const;

  // Get the location of the token that precedes 'decl'.
  clang::SourceLocation getDeclPrecedingTokenLoc(
    clang::Decl const *decl) const;

  // If `decl` is redeclarable, get the declaration of that same entity
  // that came before it in the translation unit.  Return `nullptr` is
  // `decl` is not redeclarable or is the first declaration of its
  // entity.
  clang::NamedDecl const * NULLABLE getRedeclarablePreviousDeclarationOpt(
    clang::NamedDecl const *decl) const;

  // True if `a` and `b` declare the same entity.
  bool sameEntity(clang::Decl const *a, clang::Decl const *b) const;

  // Render `decl` as a string by pretty-printing the syntax.
  std::string declSyntaxStr(clang::Decl const * NULLABLE decl) const;

  // Pretty-print the entire TU.
  std::string tuSyntaxStr() const;

  // --------------------------- DeclContext ---------------------------
  // Cast 'dc' to the associated Decl pointer.  Asserts that the
  // conversion succeeds, unless 'dc' is null, in which case null is
  // returned.
  static clang::Decl const *declFromDC(
    clang::DeclContext const * NULLABLE dc);

  // -------------------------- FunctionDecl ---------------------------
  // Is 'fd' is result of instantiating a template or member of a
  // template, return the user-written declaration from which it was
  // instantiated.  Otherwise return 'fd' itself.
  static clang::FunctionDecl const *getUserWrittenFunctionDecl(
    clang::FunctionDecl const *fd);
  static clang::FunctionDecl *getUserWrittenFunctionDecl(
    clang::FunctionDecl *fd);

  // True if 'fd == getUserWrittenFunctionDecl(fd)'.
  static bool isUserWrittenFunctionDecl(clang::FunctionDecl const *fd);

  // ------------------------- DeclarationName -------------------------
  // Stringify 'declName' and its kind.
  static std::string declarationNameStr(
    clang::DeclarationName declName);
  static std::string declarationNameKindStr(
    clang::DeclarationName::NameKind declNameKind);
  static std::string declarationNameAndKindStr(
    clang::DeclarationName declName);

  // ----------------------- NestedNameSpecifier -----------------------
  // Stringify 'nns' and its kind.
  std::string nestedNameSpecifierStr_nq(      // No quoting.
    clang::NestedNameSpecifier const *nns) const;
  std::string nestedNameSpecifierStr(
    clang::NestedNameSpecifier const * NULLABLE nns) const;
  std::string nestedNameSpecifierKindStr(
    clang::NestedNameSpecifier::SpecifierKind nssKind) const;
  std::string nestedNameSpecifierAndKindStr(
    clang::NestedNameSpecifier const * NULLABLE nns) const;

  // Stringify 'nnsLoc'.
  std::string nestedNameSpecifierLocStr(
    clang::NestedNameSpecifierLoc nnsLoc) const;

  // ---------------------- Various enumerations -----------------------
  // Stringify 'kind'.
  static std::string moduleOwnershipKindStr(
    clang::Decl::ModuleOwnershipKind kind);

  // Stringify 'specifier'.
  static std::string accessSpecifierStr(
    clang::AccessSpecifier specifier);

  // Stringify 'idns' as a '|'-separated sequence of flag names.  The
  // result could be "0" (but not "").
  static std::string identifierNamespaceStr(
    enum clang::Decl::IdentifierNamespace idns);

  // Stringify 'linkage'.
  static std::string linkageStr(clang::Linkage linkage);

  // Stringify 'storageClass'.
  static std::string storageClassStr(clang::StorageClass storageClass);

  // Stringify 'tscSpec'.
  static std::string threadStorageClassSpecifierStr(
    clang::ThreadStorageClassSpecifier tscSpec);

  // Stringify 'initStyle'.
  static std::string initializationStyleStr(
    clang::VarDecl::InitializationStyle initStyle);

  // Stringify 'kind'.
  static std::string exprValueKindStr(
    clang::ExprValueKind kind);

  // Stringify 'kind'.
  static std::string exprObjectKindStr(
    clang::ExprObjectKind kind);

  // Stringify 'dependence' as a '|'-separated sequence of flag names,
  // with "None" to indicate no flags.
  static std::string exprDependenceStr(
    clang::ExprDependence dependence);

  // Stringify 'reason'.
  static std::string nonOdrUseReasonStr(
    clang::NonOdrUseReason reason);

  // Stringify 'kind'.
  static std::string constexprSpecKindStr(
    clang::ConstexprSpecKind kind);

  // Stringify 'kind'.
  static std::string tagTypeKindStr(
    clang::TagTypeKind kind);

  // Stringify 'kind'.
  static std::string argPassingKindStr(
    clang::RecordDecl::ArgPassingKind kind);

  // Stringify 'lcd'.
  static std::string lambdaCaptureDefaultStr(
    clang::LambdaCaptureDefault lcd);

  // Stringify 'ldk'.
  static std::string lambdaDependencyKindStr(
    clang::CXXRecordDecl::LambdaDependencyKind ldk);

  // Stringify 'icis'.
  static std::string inClassInitStyleStr(
    clang::InClassInitStyle icis);

  // Stringify 'keyword'.
  static std::string elaboratedTypeKeywordStr(
    clang::ElaboratedTypeKeyword keyword);

  // Stringify 'est'.
  static std::string exceptionSpecificationTypeStr(
    clang::ExceptionSpecificationType est);

  // Stringify 'op'.
  static std::string overloadedOperatorKindStr(
    clang::OverloadedOperatorKind op);

  // Stringify 'op'.
  static std::string binaryOperatorKindStr(
    clang::BinaryOperatorKind op);

  // Stringify 'op'.
  static std::string unaryOperatorKindStr(
    clang::UnaryOperatorKind op);

  // Stringify 'ckind'.
  static std::string castKindStr(clang::CastKind ckind);

  // Stringify 'tlClass', e.g., "ElaboratedTypeLoc".
  static std::string typeLocClassStr(
    clang::TypeLoc::TypeLocClass tlClass);

  // Stringify 'kind'.
  static std::string templatedKindStr(
    clang::FunctionDecl::TemplatedKind kind);

  // Stringify 'kind'.
  static std::string templateSpecializationKindStr(
    clang::TemplateSpecializationKind kind);

#if CLANG_VERSION_MAJOR >= 18
  // Stringify `style`.
  static std::string cxxNewInitializationStyleStr(
    clang::CXXNewInitializationStyle style);
#endif

  // ------------------------------ Stmt -------------------------------
  // Render 'stmt' as a string by pretty-printing the syntax.
  //
  // TODO: Rename to `stmtSyntaxStr`.
  std::string stmtStr(clang::Stmt const * NULLABLE stmt) const;

  // Get the location of 'stmt'.
  std::string stmtLocStr(clang::Stmt const * NULLABLE stmt) const;

  // Get stmt kind.
  std::string stmtKindStr(clang::Stmt const * NULLABLE stmt) const;

  // Kind and location.
  std::string stmtKindLocStr(clang::Stmt const * NULLABLE stmt) const;

  // --------------------- Type, QualType, TypeLoc ---------------------
  // Render 'type' as a string.
  static std::string typeStr(clang::Type const * NULLABLE type);
  static std::string qualTypeStr(clang::QualType type);

  // Print 'type' as a quoted string along with its kind name.
  static std::string typeAndKindStr(clang::Type const * NULLABLE type);
  static std::string qualTypeAndKindStr(clang::QualType type);

  // Print the parameter types of 'functionType', in parentheses.
  std::string signatureStr(
    clang::FunctionProtoType const *functionType) const;

  // Get the string representation of 'qualType' for use when printing
  // parameter types.
  std::string getParamTypeString(clang::QualType qualType) const;

  // Stringify 'tinfo'.
  std::string typeSourceInfoStr(
    clang::TypeSourceInfo const * NULLABLE tinfo) const;

  // Stringify 'typeLoc'.
  std::string typeLocStr(clang::TypeLoc typeLoc) const;

  // Apply the appropriate desugaring to 'type' to get down to something
  // structural.
  clang::Type const *desugar(clang::Type const *type) const;
  clang::QualType desugar(clang::QualType type) const;

  // ---------------------------- Templates ----------------------------
  // Return either the parameter string or the argument string (see
  // next two methods).
  std::string templateArgsOrParamsForClassIfT(
    clang::CXXRecordDecl const *cxxRecordDecl, bool wantParams) const;

  // If 'cxxRecordDecl' is a template class, then return a
  // newline-terminated string like "template <class T>\n" that
  // introduces its template parameters.  Otherwise, return an empty
  // string.
  std::string templateParamsForClassIfT(
    clang::CXXRecordDecl const *cxxRecordDecl) const;

  // If 'cxxRecordDecl' is a template class, return a string like "<T>".
  // Otherwise return "".
  std::string templateArgsForClassIfT(
    clang::CXXRecordDecl const *cxxRecordDecl) const;

  // If 'functionDecl' is a template function, return a newline-
  // terminated template parameter list.  Otherwise, return "".
  std::string templateParamsForFunctionIfT(
    clang::FunctionDecl const *functionDecl) const;

  // If `functionDecl` is the body declaration of a function template,
  // return a string denoting its parameters like "<T>".  If it is a
  // specialization of a function template, return a string denoting
  // its arguments like "<int>".
  std::string templateArgsForFunctionIfT(
    clang::FunctionDecl const *functionDecl) const;

  // Return the template parameters of 'templateDecl' as a string in
  // argument syntax.
  static std::string templateDeclParamsAsArgsStr(
    clang::TemplateDecl const *templateDecl);

  // Stringify 'templateName'.
  std::string templateNameStr(
    clang::TemplateName const &templateName) const;

  // Stringify 'kind'.
  static std::string templateNameKindStr(
    clang::TemplateName::NameKind kind);

  // Get both the name and its kind.
  std::string templateNameAndKindStr(
    clang::TemplateName const &templateName) const;

  // Remove all template arguments.
  std::string removeTemplateArguments(std::string const &src);

  // If `namedDecl` is an instantiation of a template or explicit
  // partial specialization, or a member of one, return the declaration
  // from which it was instantiated.  That will be a declaration of the
  // same basic kind, like a `FunctionDecl` for a `FunctionDecl` or a
  // `CXXRecordDecl` for that.  It is *not* the `TemplateDecl` that
  // might surround it.  Otherwise, return `nullptr`.
  //
  // If it was originally instantiated from a non-definition
  // declaration, that is what is returned here, even if a definition of
  // the template is later added.
  //
  clang::NamedDecl const * NULLABLE getInstFromDeclOpt(
    clang::NamedDecl const *namedDecl) const;

  // Like `CXXRecordDecl::getTemplateInstantiationPattern`, but without
  // calling `getDefinition` before returning.
  clang::CXXRecordDecl const * NULLABLE
  getCXXRecordDeclTemplateInstantiationPatternOpt(
    clang::CXXRecordDecl const *decl) const;

  // ------------------------ TemplateParameter ------------------------
  // Turn 'paramList' into a string like "template <class T>".
  std::string templateParameterListStr(
    clang::TemplateParameterList const *paramList) const;

  // Enclose 'args' in angle brackets, separated by commas, and with an
  // extra space before the closing angle bracket if it would otherwise
  // be adjacent to another closing angle bracket.
  //
  // If 'hasParameterPack', then add "..." just before the ">".
  static std::string encloseInAngleBrackets(
    std::list<std::string> const &args,
    bool hasParameterPack);

  // Turn 'paramList' into a string like "<T>".
  static std::string templateParameterListArgsStr(
    clang::TemplateParameterList const *paramList);

  // ------------------------ TemplateArgument -------------------------
  // Stringify 'kind'.
  static std::string templateArgumentKindStr(
    clang::TemplateArgument::ArgKind kind);

  // Render 'arg' as a string.
  std::string templateArgumentStr(
    clang::TemplateArgument const &arg) const;

  // Print 'arg' in double quotes, followed by its kind.
  std::string templateArgumentAndKindStr(
    clang::TemplateArgument const &arg) const;

  // Render 'argLoc' as a string: quoted argument, then its kind, and
  // then its source location.
  std::string templateArgumentLocStr(
    clang::TemplateArgumentLoc const &argLoc) const;

  // Render 'args' as a string like "<int, float>".
  std::string templateArgumentListStr(
    clang::TemplateArgumentList const &args) const;

  // Print "null" if 'args' is, otherwise double-quote the
  // stringification of 'args'.
  std::string templateArgumentListOptStr(
    clang::TemplateArgumentList const * NULLABLE args) const;

  // Render 'args' as a string like "<int, float>".
  std::string astTemplateArgumentListInfoStr(
    clang::ASTTemplateArgumentListInfo const &argsInfo) const;

  // Print "null" if 'argsInfo' is.
  std::string astTemplateArgumentListInfoOptStr(
    clang::ASTTemplateArgumentListInfo const * NULLABLE argsInfo) const;

  // ----------------------------- Headers -----------------------------
  // Return a quoted or angle-quoted string that will denote 'fname',
  // given 'headerSearchOptions'.
  //
  // If 'userEntryIndex != nullptr', then '*userEntryIndex' will be set
  // to the index of 'm_headerSearchOptions.UserEntries' where the file
  // was found, or -1 if it was found relative to the current directory,
  // or -2 if I can't figure out where it came from.
  std::string getIncludeSyntax(
    clang::HeaderSearchOptions const &headerSearchOptions,
    std::string const &fname,
    int * NULLABLE userEntryIndex = nullptr);

  // Return true if 'fname' refers to a private header that should not
  // be nominated in an '#include' directive.
  static bool isPrivateHeaderName(std::string const &fname);

  // Same, but for a FileEntry.
  static bool isPrivateHeaderEntry(clang::FileEntry const *entry);

  // Find the file whose inclusion from the main source file led to
  // 'loc' being in the translation unit.  Returns an empty string if
  // the location did not arise from any include.
  std::string getTopLevelIncludeForLoc(clang::SourceLocation loc) const;

  // Get the file name containing 'loc', as influenced by #line
  // directives, and dealing with the possibility of macro expansion.
  std::string publicPresumedFname(clang::SourceLocation loc);

  // ----------------------------- APValue -----------------------------
  // Stringify 'kind'.
  static std::string apValueKindStr(clang::APValue::ValueKind kind);

  // Stringify 'apValue'.
  static std::string apValueStr(clang::APValue const * NULLABLE apValue);

  // Stringify 'vec'.
  static std::string smallVectorStr(
    llvm::SmallVectorImpl<char> const &vec);

  // Stringify 'n'.
  static std::string apIntStr(llvm::APInt const &n, bool isSigned);
  static std::string apsIntStr(llvm::APSInt const &n);
};


// This class sets `ClangUtil::s_instance` to itself.
class GlobalClangUtilInstance : public ClangUtil,
                                public SetRestore<ClangUtil const *> {
public:      // methods
  ~GlobalClangUtilInstance();
  explicit GlobalClangUtilInstance(clang::ASTContext &astContext);
};


// Deterministically compare two declarations, given pointers to them.
//
// This assumes that both declarations are part of the same TU.  When
// that is the case, the order will be stable across parser invocations.
//
// This is meant to be used as the 'Compare' argument to a container.
//
class DeclCompare {
public:
  // Strcmp-style comparison result.
  static int compare(clang::Decl const *a, clang::Decl const *b);

  // Comparison operator for use with containers.
  //
  // I'd like this to be 'static' but evidently that's not possible
  // in C++17.
  bool operator() (clang::Decl const *a, clang::Decl const *b) const
  {
    return compare(a,b) < 0;
  }
};


// Return the characters in the range [begin,end-1] as a string.
std::string stringRefRange(
  llvm::StringRef const &sr, unsigned begin, unsigned end);


// Lexicographic order on SourceRange.
int compare(clang::SourceRange const &a, clang::SourceRange const &b);
namespace clang {
  inline bool operator< (clang::SourceRange const &a, clang::SourceRange const &b)
  {
    return compare(a,b) < 0;
  }
}


// Get the name of the dynamic type of the argument.  We have one
// overload for each class hierarchy root.
std::string getDynamicTypeClassName(clang::Type const *type);
std::string getDynamicTypeClassName(clang::Decl const *decl);
std::string getDynamicTypeClassName(clang::Stmt const *stmt);
std::string getDynamicTypeClassName(clang::concepts::Requirement const *req);

// There is no overload of 'getDynamicTypeClassName' for DeclContext
// because it would be ambiguous with the overload for Decl for any
// argument that inherits both.  So we define it with a different name,
// and then specialize 'outerGetDynamicTypeClassName' for DeclContext to
// use it.
std::string getDeclContextClassName(clang::DeclContext const *dc);


template <class Y>
inline std::string outerGetDynamicTypeClassName(Y *src)
{
  return getDynamicTypeClassName(src);
}

template <>
inline std::string outerGetDynamicTypeClassName(clang::DeclContext *src)
{
  return getDeclContextClassName(src);
}

template <>
inline std::string outerGetDynamicTypeClassName(clang::DeclContext const *src)
{
  return getDeclContextClassName(src);
}


// Report an attempt to dyn_cast a null pointer.
void assert_dyn_cast_null(
  char const *destTypeName,
  char const *sourceFile,
  int sourceLine);


// Report a failed assert_dyn_cast.
void assert_dyn_cast_failed(
  char const *destTypeName,
  std::string const &srcTypeName,
  char const *sourceFile,
  int sourceLine);


// Like 'dyn_cast', but assert that it succeeds.  Unlike 'cast', this
// will also print the actual type of the source pointer.
#define assert_dyn_cast(DestType, src) \
  assert_dyn_cast_impl<DestType>(#DestType, (src), __FILE__, __LINE__)

template <class X, class Y>
inline typename llvm::cast_retty<X, Y *>::ret_type
assert_dyn_cast_impl(
  char const *destTypeName,
  Y *src,
  char const *sourceFile,
  int sourceLine)
{
  if (!src) {
    assert_dyn_cast_null(destTypeName, sourceFile, sourceLine);
  }

  typename llvm::cast_retty<X, Y *>::ret_type ret =
    llvm::dyn_cast<X>(src);

  if (!ret) {
    assert_dyn_cast_failed(destTypeName,
      outerGetDynamicTypeClassName(src), sourceFile, sourceLine);
  }

  return ret;
}


void clang_util_unit_tests();


#endif // HEADER_ANALYSIS_CLANG_UTIL_H
