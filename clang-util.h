// clang-util.h
// ClangUtil, a class with various Clang utilities.

#ifndef HEADER_ANALYSIS_CLANG_UTIL_H
#define HEADER_ANALYSIS_CLANG_UTIL_H

#include "util.h"                                          // NULLABLE

#include "clang/AST/ASTContext.h"                          // clang::ASTContext::{getLangOpts, getSourceManager, getTranslationUnitDecl}
#include "clang/AST/ASTFwd.h"                              // clang::{Type, Stmt, Decl} [n]
#include "clang/AST/Decl.h"                                // clang::NamedDecl
#include "clang/AST/DeclTemplate.h"                        // clang::TemplateParameterList
#include "clang/AST/ExternalASTSource.h"                   // clang::ExternalASTSource
#include "clang/AST/Type.h"                                // clang::QualType
#include "clang/Basic/Lambda.h"                            // clang::LambdaCaptureDefault
#include "clang/Basic/OperatorKinds.h"                     // clang::OverloadedOperatorKind
#include "clang/Basic/SourceLocation.h"                    // clang::FileID
#include "clang/Basic/SourceManager.h"                     // clang::SourceManager
#include "clang/Basic/Specifiers.h"                        // clang::InClassInitStyle
#include "clang/Basic/Version.h"                           // CLANG_VERSION_MAJOR
#include "clang/Lex/HeaderSearchOptions.h"                 // clang::HeaderSearchOptions

#include "llvm/ADT/StringRef.h"                            // llvm::StringRef [n]
#include "llvm/Support/Casting.h"                          // llvm::dyn_cast

#include <list>                                            // std::list
#include <string>                                          // std::string
#include <vector>                                          // std::vector


// Expand to 'thenCode' if we're compiling against Clang+LLVM 17 or later.
#if CLANG_VERSION_MAJOR >= 17
  #define IF_CLANG_17(thenCode, elseCode) thenCode
#else
  #define IF_CLANG_17(thenCode, elseCode) elseCode
#endif


// The idea is to populate this with data for a single TU, and it can
// provide services (especially printing services) on top of Clang's TU
// services.  I then typically have a class that does some analysis
// inherit this one.
//
// Methods in this class are 'static' when the ASTContext is not
// actually needed, and (should be) 'const' otherwise.
class ClangUtil {
public:      // data
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
  ClangUtil(clang::ASTContext &astContext);

  // Get the LangOptions in 'm_astContext'.
  clang::LangOptions const &getLangOptions() const;

  // Get the associated external AST source, if any.
  clang::ExternalASTSource * NULLABLE getExternalSource() const;

  // Render 'loc' as a string.
  std::string locStr(clang::SourceLocation loc) const;

  // Stringify 'range'.
  std::string sourceRangeStr(clang::SourceRange range) const;

  // Get the principal location for 'decl'.  In particular, if 'decl' is
  // a definition of something that is declared elsewhere, I want the
  // location of that definition, whereas Decl::getLocation() sometimes
  // returns the declaration location.
  //
  // In general, I should be using this rather than Decl::getLocation()
  // unless there is a known, specific reason.
  //
  // Note that many Stmt subclasses have a getLocation() method that, as
  // far as I know, is fine to use.
  static clang::SourceLocation declLoc(clang::Decl const *decl);

  // Get the location of the identifier for 'decl'.
  static clang::SourceLocation getIdentifierLoc(
    clang::NamedDecl const *decl);

  // Render the location of 'decl' as a string.
  std::string declLocStr(clang::Decl const *decl) const;

  // Render 'decl' with qualifiers and signature.
  std::string namedDeclStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Stringify the declaration syntax and its location.
  std::string namedDeclAtLocStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Stringify the declaration syntax, its kind, and its location.
  std::string namedDeclAndKindAtLocStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Like 'namedDeclAtLocStr' but include the address.
  std::string namedDeclAddrAtLocStr(
    clang::NamedDecl const * NULLABLE namedDecl) const;

  // Print address and location of 'decl'.  I include "unnamed" in the
  // method name because this is not what should normally be used, since
  // normally we have a NamedDecl.
  std::string unnamedDeclAddrAtLocStr(
    clang::Decl const * NULLABLE decl) const;

  // Stringify 'declName' and its kind.
  static std::string declarationNameStr(
    clang::DeclarationName declName);
  static std::string declarationNameKindStr(
    clang::DeclarationName::NameKind declNameKind);
  static std::string declarationNameAndKindStr(
    clang::DeclarationName declName);

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

  // Cast 'dc' to the associated Decl pointer.  Asserts that the
  // conversion succeeds, unless 'dc' is null, in which case null is
  // returned.
  static clang::Decl const *declFromDC(
    clang::DeclContext const * NULLABLE dc);

  // Render 'stmt' as a string.
  std::string stmtStr(clang::Stmt const * NULLABLE stmt) const;

  // Get the location of 'stmt'.
  std::string stmtLocStr(clang::Stmt const * NULLABLE stmt) const;

  // Get stmt kind.
  std::string stmtKindStr(clang::Stmt const * NULLABLE stmt) const;

  // Kind and location.
  std::string stmtKindLocStr(clang::Stmt const * NULLABLE stmt) const;

  // Render 'type' as a string.
  static std::string typeStr(clang::Type const * NULLABLE type);
  static std::string qualTypeStr(clang::QualType type);

  // Print 'type' as a quoted string along with its kind name.
  static std::string typeAndKindStr(clang::Type const * NULLABLE type);
  static std::string qualTypeAndKindStr(clang::QualType type);

  // Print the parameter types of 'functionType', in parentheses.
  std::string signatureStr(
    clang::FunctionProtoType const *functionType) const;

  // Stringify 'tinfo'.
  std::string typeSourceInfoStr(
    clang::TypeSourceInfo const * NULLABLE tinfo) const;

  // Stringify 'typeLoc'.
  std::string typeLocStr(clang::TypeLoc typeLoc) const;

  // Stringify 'tlClass'.
  static std::string typeLocClassStr(
    clang::TypeLoc::TypeLocClass tlClass);

  // Stringify 'kind'.
  static std::string templatedKindStr(
    clang::FunctionDecl::TemplatedKind kind);

  // Stringify 'kind'.
  static std::string templateSpecializationKindStr(
    clang::TemplateSpecializationKind kind);

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

  // Stringify 'templateName'.
  std::string templateNameStr(
    clang::TemplateName const &templateName) const;

  // Stringify 'kind'.
  static std::string templateNameKindStr(
    clang::TemplateName::NameKind kind);

  // Get both the name and its kind.
  std::string templateNameAndKindStr(
    clang::TemplateName const &templateName) const;

  // True if 'loc' is in the main file of the translation unit.
  bool inMainFile(clang::SourceLocation loc) const;

  // Apply the appropriate desugaring to 'type' to get down to something
  // structural.
  clang::Type const *desugar(clang::Type const *type) const;
  clang::QualType desugar(clang::QualType type) const;

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

  // Turn a FileID into a string.
  std::string getFnameForFileID(clang::FileID fileID) const;

  // If 'loc' is an expansion location, get the place where the
  // expansion happened; otherwise use it as-is.  Then get the FileID
  // from it.
  clang::FileID getExpansionFileID(clang::SourceLocation loc) const;

  // Get the string representation of 'qualType' for use when printing
  // parameter types.
  std::string getParamTypeString(clang::QualType qualType) const;

  // Remove all template arguments.
  std::string removeTemplateArguments(std::string const &src);

  // Get the file name containing 'loc', as influenced by #line
  // directives, and dealing with the possibility of macro expansion.
  std::string publicPresumedFname(clang::SourceLocation loc);

  // If 'decl' is a member, return its containing class.  Otherwise,
  // return 'nullptr'.
  clang::RecordDecl *maybeGetParentClass(clang::NamedDecl *decl);

  // Get the innermost enclosing parent that has a name suitable for use
  // in a qualifier, or nullptr if there is none.
  clang::NamedDecl *maybeGetNamedParent(
    clang::NamedDecl *decl) const;

  // Find the file whose inclusion from the main source file led to
  // 'loc' being in the translation unit.  Returns an empty string if
  // the location did not arise from any include.
  std::string getTopLevelIncludeForLoc(clang::SourceLocation loc);

  // If 'decl' is something whose declaration is introduced with a
  // keyword like "class" or "namespace", return that keyword.
  // Otherwise, return "".
  std::string getDeclKeyword(clang::NamedDecl *decl);

  // Turn 'paramList' into a string like "template <class T>".
  std::string templateParameterListStr(
    clang::TemplateParameterList const *paramList) const;

  // Enclose 'args' in angle brackets, separated by commas, and with an
  // extra space before the closing angle bracket if it would otherwise
  // be adjacent to another closing angle bracket.
  //
  // If 'hasParameterPack', then add "..." just before the ">".
  std::string encloseInAngleBrackets(
    std::list<std::string> const &args,
    bool hasParameterPack) const;

  // Turn 'paramList' into a string like "<T>".
  std::string templateParameterListArgsStr(
    clang::TemplateParameterList const *paramList) const;

  // Stringify 'kind'.
  static std::string templateArgumentKindStr(
    clang::TemplateArgument::ArgKind kind);

  // Render 'arg' as a string.
  std::string templateArgumentStr(
    clang::TemplateArgument const &arg) const;

  // Print 'arg' in double quotes, followed by its kind.
  std::string templateArgumentAndKindStr(
    clang::TemplateArgument const &arg) const;

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

  // Get the canonical declaration for 'decl'.
  clang::NamedDecl *canonicalNamedDecl(clang::NamedDecl *decl);

  // Stringify 'kind'.
  static std::string apValueKindStr(clang::APValue::ValueKind kind);

  // Stringify 'apValue'.
  static std::string apValueStr(clang::APValue const * NULLABLE apValue);

  // Stringify 'vec'.
  static std::string smallVectorStr(
    llvm::SmallVectorImpl<char> const &vec);

  // Stringify 'n'.
  static std::string apsIntStr(llvm::APSInt const &n);

  // Is 'decl' the declaration of an operator?
  bool isOperatorDecl(clang::NamedDecl const *decl) const;

  // True if 'loc' is in a source file, as opposed to a macro expansion
  // or on the command line.
  bool locInSourceFile(clang::SourceLocation loc) const;

  // Return the entry corresponding to the expansion location of 'loc',
  // or null if there is none.
  clang::FileEntry const *getFileEntryForLoc(
    clang::SourceLocation loc) const;

  // Get the location of the token that precedes 'decl'.
  clang::SourceLocation getDeclPrecedingTokenLoc(
    clang::Decl const *decl) const;

  // Is 'fd' is result of instantiating a template or member of a
  // template, return the user-written declaration from which it was
  // instantiated.  Otherwise return 'fd' itself.
  static clang::FunctionDecl const *getUserWrittenFunctionDecl(
    clang::FunctionDecl const *fd);
  static clang::FunctionDecl *getUserWrittenFunctionDecl(
    clang::FunctionDecl *fd);

  // True if 'fd == getUserWrittenFunctionDecl(fd)'.
  static bool isUserWrittenFunctionDecl(clang::FunctionDecl const *fd);

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


// Get the name of the dynamic type of the argument.  We have one
// overload for each class hierarchy root.
std::string getDynamicTypeClassName(clang::Type const *type);
std::string getDynamicTypeClassName(clang::Decl const *decl);
std::string getDynamicTypeClassName(clang::Stmt const *stmt);


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
inline typename llvm::cast_retty<X, Y *>::ret_type assert_dyn_cast_impl(
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
      getDynamicTypeClassName(src), sourceFile, sourceLine);
  }

  return ret;
}


#endif // HEADER_ANALYSIS_CLANG_UTIL_H
