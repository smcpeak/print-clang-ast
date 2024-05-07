// clang-util.cc
// Code for clang-util.h.

#include "clang-util.h"                // this module

#include "compare-util.h"              // compare
#include "enum-util.h"                 // ENUM_TABLE_LOOKUP, BITFLAGS_TABLE_LOOKUP
#include "util.h"                      // startsWith, trimWhitespace, etc.

#include "clang/AST/Decl.h"            // clang::FieldDecl::getParent
#include "clang/AST/DeclCXX.h"         // clang::CXXMethodDecl::getParent
#include "clang/AST/ExprConcepts.h"    // clang::concepts::Requirement
#include "clang/AST/Type.h"            // clang::FunctionProtoType
#include "clang/Basic/Version.h"       // CLANG_VERSION_MAJOR

#include "llvm/ADT/APSInt.h"           // llvm::APSint::toString
#include "llvm/Support/raw_ostream.h"  // llvm::raw_string_ostream


using clang::SourceLocation;
using clang::dyn_cast;
using clang::isa;

using std::string;


ClangUtil::ClangUtil(clang::ASTContext &astContext)
  : m_astContext(astContext),
    m_srcMgr(m_astContext.getSourceManager()),
    m_mainFileID(m_srcMgr.getMainFileID()),
    m_mainFileName(getFnameForFileID(m_mainFileID)),
    m_printingPolicy(getLangOptions())
{
  m_printingPolicy.Indentation = 2;
  m_printingPolicy.Bool = true;
  m_printingPolicy.Nullptr = true;
  m_printingPolicy.SplitTemplateClosers = true;
}


clang::LangOptions const &ClangUtil::getLangOptions() const
{
  return m_astContext.getLangOpts();
}


clang::ExternalASTSource * NULLABLE ClangUtil::getExternalSource() const
{
  return m_astContext.getExternalSource();
}


string ClangUtil::locStr(SourceLocation loc) const
{
  return loc.printToString(m_srcMgr);
}


std::string ClangUtil::sourceRangeStr(clang::SourceRange range) const
{
  return range.printToString(m_srcMgr);
}


/*static*/ clang::SourceLocation ClangUtil::declLoc(
  clang::Decl const *decl)
{
  // One case where 'getLocation()' does the wrong thing is if 'decl' is
  // a FunctionDecl implicitly instantiated from a member template of a
  // template class, where the member template was declared in the class
  // body but then defined outside it.  In that case, 'getLocation()'
  // points at the declaration, while 'getBeginLoc()' points at the
  // definition.
  //
  // Example: in/src/template-method-ops.h.
  return decl->getBeginLoc();
}


/*static*/ clang::SourceLocation ClangUtil::getIdentifierLoc(
  clang::NamedDecl const *decl)
{
  // At the moment, I have no better alternative than to rely on
  // 'getLocation()'.  I don't know if the problems noted above will be
  // an issue here.
  return decl->getLocation();
}


std::string ClangUtil::declLocStr(clang::Decl const *decl) const
{
  return locStr(declLoc(decl));
}


std::string ClangUtil::declKindAtLocStr(
  clang::Decl const * NULLABLE decl) const
{
  if (!decl) {
    return "null";
  }

  else {
    return stringb(decl->getDeclKindName() << "Decl " <<
                   " at " << declLocStr(decl));
  }
}


std::string ClangUtil::namedDeclStr(
  clang::NamedDecl const * NULLABLE namedDecl) const
{
  if (!namedDecl) {
    return "null";
  }

  std::ostringstream oss;

  oss << namedDecl->getQualifiedNameAsString();

  if (auto cxxRecordDecl = dyn_cast<clang::CXXRecordDecl>(namedDecl)) {
    oss << templateArgsForClassIfT(cxxRecordDecl);
  }

  if (auto valueDecl = dyn_cast<clang::ValueDecl>(namedDecl)) {
    clang::Type const *type =
      valueDecl->getType().getTypePtr()->getUnqualifiedDesugaredType();
    if (auto functionType = dyn_cast<clang::FunctionProtoType>(type)) {
      // Append type information.
      oss << signatureStr(functionType);
    }
  }

  return doubleQuote(oss.str());
}


std::string ClangUtil::namedDeclAtLocStr(
  clang::NamedDecl const * NULLABLE namedDecl) const
{
  if (namedDecl) {
    return stringb(namedDeclStr(namedDecl) <<
                   " at " << declLocStr(namedDecl));
  }
  else {
    return "null";
  }
}


std::string ClangUtil::namedDeclAndKindAtLocStr(
  clang::NamedDecl const * NULLABLE namedDecl) const
{
  if (namedDecl) {
    return stringb(namedDecl->getDeclKindName() << "Decl " <<
                   namedDeclStr(namedDecl) <<
                   " at " << declLocStr(namedDecl));
  }
  else {
    return "null";
  }
}


std::string ClangUtil::namedDeclAddrAtLocStr(
  clang::NamedDecl const * NULLABLE namedDecl) const
{
  if (namedDecl) {
    return stringb(namedDeclAtLocStr(namedDecl) <<
                   " (" << (void*)namedDecl << ")");
  }
  else {
    return "null";
  }
}


std::string ClangUtil::unnamedDeclAddrAtLocStr(
  clang::Decl const * NULLABLE decl) const
{
  if (decl) {
    return stringb(decl->getDeclKindName() << "Decl at " <<
                   declLocStr(decl) <<
                   " (" << (void*)decl << ")");
  }
  else {
    return "null";
  }
}


/*static*/ std::string ClangUtil::declarationNameStr(
  clang::DeclarationName declName)
{
  return stringb("\"" << declName.getAsString() << "\"");
}


/*static*/ std::string ClangUtil::declarationNameKindStr(
  clang::DeclarationName::NameKind declNameKind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::DeclarationName::, NameKind, declNameKind,

    Identifier,
    ObjCZeroArgSelector,
    ObjCOneArgSelector,
    CXXConstructorName,
    CXXDestructorName,
    CXXConversionFunctionName,
    CXXOperatorName,
    CXXDeductionGuideName,
    CXXLiteralOperatorName,
    CXXUsingDirective,
    ObjCMultiArgSelector,
  )
}


/*static*/ std::string ClangUtil::declarationNameAndKindStr(
  clang::DeclarationName declName)
{
  return stringb(declarationNameStr(declName) << " (" <<
                 declarationNameKindStr(declName.getNameKind()) << ")");
}


std::string ClangUtil::nestedNameSpecifierStr_nq(
  clang::NestedNameSpecifier const *nns) const
{
  string name;
  llvm::raw_string_ostream rso(name);
  nns->print(rso, m_printingPolicy);
  return name;
}


std::string ClangUtil::nestedNameSpecifierStr(
  clang::NestedNameSpecifier const * NULLABLE nns) const
{
  if (nns) {
    return doubleQuote(nestedNameSpecifierStr_nq(nns));
  }
  else {
    return "null";
  }
}


std::string ClangUtil::nestedNameSpecifierKindStr(
  clang::NestedNameSpecifier::SpecifierKind nssKind) const
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::NestedNameSpecifier::, SpecifierKind, nssKind,

    Identifier,
    Namespace,
    NamespaceAlias,
    TypeSpec,
    TypeSpecWithTemplate,
    Global,
    Super,
  )
}


std::string ClangUtil::nestedNameSpecifierAndKindStr(
  clang::NestedNameSpecifier const * NULLABLE nns) const
{
  if (nns) {
    return stringb(nestedNameSpecifierStr(nns) << " (" <<
                   nestedNameSpecifierKindStr(nns->getKind()) << ")");
  }
  else {
    return "null";
  }
}


std::string ClangUtil::nestedNameSpecifierLocStr(
  clang::NestedNameSpecifierLoc nnsLoc) const
{
  if (nnsLoc.hasQualifier()) {
    // NNSLoc has location information for each individual qualifier, as
    // well as the type of qualifiers that denote types (classes).  But
    // here I'm just printing the high-level info.
    return stringb(nestedNameSpecifierStr(nnsLoc.getNestedNameSpecifier()) <<
                   " at " << sourceRangeStr(nnsLoc.getSourceRange()));
  }
  else {
    return "AbsentNNSLoc";
  }
}


/*static*/ std::string ClangUtil::moduleOwnershipKindStr(
  clang::Decl::ModuleOwnershipKind kind)
{
  ENUM_CLASS_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::Decl::, ModuleOwnershipKind, kind,

    Unowned,
    Visible,
    VisibleWhenImported,
    ModulePrivate,
  )
}


/*static*/ std::string ClangUtil::accessSpecifierStr(
  clang::AccessSpecifier specifier)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, AccessSpecifier, specifier,

    AS_public,
    AS_protected,
    AS_private,
    AS_none,
  )
}


/*static*/ std::string ClangUtil::identifierNamespaceStr(
  enum clang::Decl::IdentifierNamespace idns)
{
  BITFLAGS_TABLE_LOOKUP(clang::Decl::, "IdentifierNamespace", "0", idns,

    IDNS_Label,
    IDNS_Tag,
    IDNS_Type,
    IDNS_Member,
    IDNS_Namespace,
    IDNS_Ordinary,
    IDNS_ObjCProtocol,
    IDNS_OrdinaryFriend,
    IDNS_TagFriend,
    IDNS_Using,
    IDNS_NonMemberOperator,
    IDNS_LocalExtern,
    IDNS_OMPReduction,
    IDNS_OMPMapper,
  )
}


/*static*/ std::string ClangUtil::linkageStr(clang::Linkage linkage)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, Linkage, linkage,

    NoLinkage,
    InternalLinkage,
    UniqueExternalLinkage,
    VisibleNoLinkage,
    #if CLANG_VERSION_MAJOR < 17
      ModuleInternalLinkage,    // Removed in clang 17, no evident replacement.
    #endif
    ModuleLinkage,
    ExternalLinkage,
  )
}


/*static*/ std::string ClangUtil::storageClassStr(
  clang::StorageClass storageClass)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, StorageClass, storageClass,

    SC_None,
    SC_Extern,
    SC_Static,
    SC_PrivateExtern,
    SC_Auto,
    SC_Register,
  )
}


/*static*/ std::string ClangUtil::threadStorageClassSpecifierStr(
  clang::ThreadStorageClassSpecifier tscSpec)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, ThreadStorageClassSpecifier, tscSpec,

    TSCS_unspecified,
    TSCS___thread,
    TSCS_thread_local,
    TSCS__Thread_local,
  )
}


/*static*/ std::string ClangUtil::initializationStyleStr(
  clang::VarDecl::InitializationStyle initStyle)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::VarDecl::, InitializationStyle, initStyle,

    CInit,
    CallInit,
    ListInit,
  )
}


/*static*/ std::string ClangUtil::exprValueKindStr(
  clang::ExprValueKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, ExprValueKind, kind,

    VK_PRValue,
    VK_LValue,
    VK_XValue,
  )
}


/*static*/ std::string ClangUtil::exprObjectKindStr(
  clang::ExprObjectKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, ExprObjectKind, kind,

    OK_Ordinary,
    OK_BitField,
    OK_VectorComponent,
    OK_ObjCProperty,
    OK_ObjCSubscript,
    OK_MatrixComponent,
  )
}


/*static*/ std::string ClangUtil::exprDependenceStr(
  clang::ExprDependence dependence)
{
  BITFLAGS_TABLE_LOOKUP(clang::ExprDependenceScope::,
    "ExprDependence", "None", dependence,

    UnexpandedPack,
    Instantiation,
    Type,
    Value,
    Error,
  )
}


/*static*/ std::string ClangUtil::nonOdrUseReasonStr(
  clang::NonOdrUseReason reason)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, NonOdrUseReason, reason,

    NOUR_None,
    NOUR_Unevaluated,
    NOUR_Constant,
    NOUR_Discarded,
  )
}


/*static*/ std::string ClangUtil::constexprSpecKindStr(
  clang::ConstexprSpecKind kind)
{
  ENUM_CLASS_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, ConstexprSpecKind, kind,

    Unspecified,
    Constexpr,
    Consteval,
    Constinit,
  )
}


/*static*/ std::string ClangUtil::tagTypeKindStr(
  clang::TagTypeKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, TagTypeKind, kind,

    TTK_Struct,
    TTK_Interface,
    TTK_Union,
    TTK_Class,
    TTK_Enum,
  )
}


/*static*/ std::string ClangUtil::argPassingKindStr(
  clang::RecordDecl::ArgPassingKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::RecordDecl::, ArgPassingKind, kind,

    APK_CanPassInRegs,
    APK_CannotPassInRegs,
    APK_CanNeverPassInRegs,
  )
}


/*static*/ std::string ClangUtil::lambdaCaptureDefaultStr(
  clang::LambdaCaptureDefault lcd)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, LambdaCaptureDefault, lcd,

    LCD_None,
    LCD_ByCopy,
    LCD_ByRef,
  )
}


/*static*/ std::string ClangUtil::lambdaDependencyKindStr(
  clang::CXXRecordDecl::LambdaDependencyKind ldk)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::CXXRecordDecl::, LambdaDependencyKind, ldk,

    LDK_Unknown,
    LDK_AlwaysDependent,
    LDK_NeverDependent,
  )
}


/*static*/ std::string ClangUtil::inClassInitStyleStr(
  clang::InClassInitStyle icis)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, InClassInitStyle, icis,

    ICIS_NoInit,
    ICIS_CopyInit,
    ICIS_ListInit,
  )
}


/*static*/ std::string ClangUtil::elaboratedTypeKeywordStr(
  clang::ElaboratedTypeKeyword keyword)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, ElaboratedTypeKeyword, keyword,

    ETK_Struct,
    ETK_Interface,
    ETK_Union,
    ETK_Class,
    ETK_Enum,
    ETK_Typename,
    ETK_None,
  )
}


/*static*/ std::string ClangUtil::exceptionSpecificationTypeStr(
  clang::ExceptionSpecificationType est)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, ExceptionSpecificationType, est,

    EST_None,
    EST_DynamicNone,
    EST_Dynamic,
    EST_MSAny,
    EST_NoThrow,
    EST_BasicNoexcept,
    EST_DependentNoexcept,
    EST_NoexceptFalse,
    EST_NoexceptTrue,
    EST_Unevaluated,
    EST_Uninstantiated,
    EST_Unparsed,
  )
}


/*static*/ std::string ClangUtil::overloadedOperatorKindStr(
  clang::OverloadedOperatorKind op)
{
  // This is almost the same as 'clang::getOperatorSpelling()', but this
  // function does not assert if 'op' is invalid.

  static struct Entry {
    clang::OverloadedOperatorKind m_op;
    char const *m_name;
  } const entries[] = {
    #define ENTRY(name) { clang::name, #name }

    ENTRY(OO_None),

    #define OVERLOADED_OPERATOR(Name,Spelling,Token,Unary,Binary,MemberOnly) \
      ENTRY(OO_##Name),
    #include "clang/Basic/OperatorKinds.def"

    #undef ENTRY
  };

  for (Entry const &e : entries) {
    if (e.m_op == op) {
      return e.m_name;
    }
  }

  return stringb("OverloadedOperatorKind(" << (int)op << ")");
}


/*static*/ std::string ClangUtil::binaryOperatorKindStr(
  clang::BinaryOperatorKind op)
{
  static struct Entry {
    clang::BinaryOperatorKind m_op;
    char const *m_name;
  } const entries[] = {
    #define ENTRY(name) { clang::name, #name }

    #define BINARY_OPERATION(Name, Spelling) \
      ENTRY(BO_##Name),
    #include "clang/AST/OperationKinds.def"

    #undef ENTRY
  };

  for (Entry const &e : entries) {
    if (e.m_op == op) {
      return e.m_name;
    }
  }

  return stringb("BinaryOperatorKind(" << (int)op << ")");
}


/*static*/ std::string ClangUtil::unaryOperatorKindStr(
  clang::UnaryOperatorKind op)
{
  static struct Entry {
    clang::UnaryOperatorKind m_op;
    char const *m_name;
  } const entries[] = {
    #define ENTRY(name) { clang::name, #name }

    #define UNARY_OPERATION(Name, Spelling) \
      ENTRY(UO_##Name),
    #include "clang/AST/OperationKinds.def"

    #undef ENTRY
  };

  for (Entry const &e : entries) {
    if (e.m_op == op) {
      return e.m_name;
    }
  }

  return stringb("UnaryOperatorKind(" << (int)op << ")");
}


/*static*/ std::string ClangUtil::castKindStr(clang::CastKind ckind)
{
  static struct Entry {
    clang::CastKind m_ckind;
    char const *m_name;
  } const entries[] = {
    #define ENTRY(name) { clang::name, #name }

    #define CAST_OPERATION(Name) ENTRY(CK_##Name),
    #include "clang/AST/OperationKinds.def"

    #undef ENTRY
  };

  for (Entry const &e : entries) {
    if (e.m_ckind == ckind) {
      return e.m_name;
    }
  }

  return stringb("CastKind(" << (int)ckind << ")");
}


/*static*/ clang::Decl const *ClangUtil::declFromDC(
  clang::DeclContext const * NULLABLE dc)
{
  if (dc) {
    clang::Decl const *d = dyn_cast<clang::Decl>(dc);
    assert(d);
    return d;
  }
  else {
    return nullptr;
  }
}


std::string ClangUtil::stmtStr(clang::Stmt const * NULLABLE stmt) const
{
  if (stmt) {
    string ret;
    llvm::raw_string_ostream rso(ret);
    stmt->printPretty(rso, nullptr /*helper*/, m_printingPolicy);
    return ret;
  }
  else {
    return "null";
  }
}


std::string ClangUtil::stmtLocStr(clang::Stmt const * NULLABLE stmt) const
{
  if (stmt) {
    return locStr(stmt->getBeginLoc());
  }
  else {
    return "(loc of null stmt)";
  }
}


std::string ClangUtil::stmtKindStr(clang::Stmt const * NULLABLE stmt) const
{
  if (stmt) {
    return stmt->getStmtClassName();
  }
  else {
    return "(null stmt)";
  }
}


std::string ClangUtil::stmtKindLocStr(clang::Stmt const * NULLABLE stmt) const
{
  if (stmt) {
    return stringb(stmtKindStr(stmt) << " at " << stmtLocStr(stmt));
  }
  else {
    return "(null stmt)";
  }
}


/*static*/ std::string ClangUtil::typeStr(
  clang::Type const * NULLABLE type)
{
  if (type) {
    return qualTypeStr(clang::QualType(type, 0 /*Quals*/));
  }
  else {
    return "null";
  }
}


/*static*/ std::string ClangUtil::qualTypeStr(clang::QualType type)
{
  return type.getAsString();
}


/*static*/ std::string ClangUtil::typeAndKindStr(
  clang::Type const * NULLABLE type)
{
  if (type) {
    return stringb(doubleQuote(typeStr(type)) <<
                   " (" << type->getTypeClassName() << "Type)");
  }
  else {
    return "null";
  }
}


/*static*/ std::string ClangUtil::qualTypeAndKindStr(clang::QualType type)
{
  return stringb(doubleQuote(qualTypeStr(type)) <<
                 " (" << type->getTypeClassName() << "Type)");
}


std::string ClangUtil::signatureStr(
  clang::FunctionProtoType const *functionType) const
{
  std::ostringstream oss;

  oss << "(";
  int ct=0;
  for (clang::QualType paramType : functionType->param_types()) {
    if (ct++ > 0) {
      oss << ", ";
    }
    oss << getParamTypeString(paramType);
  }
  if (functionType->isVariadic()) {
    if (ct++ > 0) {
      oss << ", ";
    }
    oss << "...";
  }
  oss << ")";

  return oss.str();
}


std::string ClangUtil::typeSourceInfoStr(
  clang::TypeSourceInfo const * NULLABLE tinfo) const
{
  if (tinfo) {
    // TypeSourceInfo is not really a separate conceptual piece of data,
    // rather it is a physical container for what is conceptually a
    // TypeLoc, but stored in an unusual way.  So, when printing, we
    // just skip the TSI and go straight to the TypeLoc.
    return typeLocStr(tinfo->getTypeLoc());
  }
  else {
    return "NullTSI";
  }
}


std::string ClangUtil::typeLocStr(clang::TypeLoc typeLoc) const
{
  std::ostringstream oss;

  if (typeLoc.isNull()) {
    oss << "NullTypeLoc";
  }
  else {
    // TypeLoc is conceptually a list (internally an array I believe) of
    // types and locations, where each successive type is the thing that
    // can be found by "dereferencing" (in a general sense) the
    // preceding type.  It ends with a null TypeLoc.
    oss << typeLocClassStr(typeLoc.getTypeLocClass()) << "("
        << doubleQuote(qualTypeStr(typeLoc.getType())) << ", "
        << sourceRangeStr(typeLoc.getSourceRange()) << ", "
        << typeLocStr(typeLoc.getNextTypeLoc()) << ")";
  }

  return oss.str();
}


/*static*/ std::string ClangUtil::typeLocClassStr(
  clang::TypeLoc::TypeLocClass tlClass)
{
  // I won't try to use ENUM_TABLE_LOOKUP_OR_STRINGB_CAST here because I
  // don't think I can #include something in the middle of macro
  // arguments.
  static struct Entry {
    clang::TypeLoc::TypeLocClass m_tlClass;
    char const *m_name;
  } const entries[] = {
    #define ENTRY(name) { clang::TypeLoc::name, #name }

    #define ABSTRACT_TYPE(Class, Base)
    #define TYPE(Class, Base) \
        ENTRY(Class),
    #include "clang/AST/TypeNodes.inc"

    ENTRY(Qualified),
    #undef ENTRY
  };

  for (Entry const &e : entries) {
    if (e.m_tlClass == tlClass) {
      return e.m_name;
    }
  }

  return stringb("TypeLocClass(" << (int)tlClass << ")");
}


/*static*/ std::string ClangUtil::templatedKindStr(
  clang::FunctionDecl::TemplatedKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::FunctionDecl::, TemplatedKind, kind,

    TK_NonTemplate,
    TK_FunctionTemplate,
    TK_MemberSpecialization,
    TK_FunctionTemplateSpecialization,
    TK_DependentFunctionTemplateSpecialization,
    #if CLANG_VERSION_MAJOR > 14  // I'm not sure if 15 or 16 added this.
      TK_DependentNonTemplate,
    #endif
  )
}


/*static*/ std::string ClangUtil::templateSpecializationKindStr(
  clang::TemplateSpecializationKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::, TemplateSpecializationKind, kind,

    TSK_Undeclared,
    TSK_ImplicitInstantiation,
    TSK_ExplicitSpecialization,
    TSK_ExplicitInstantiationDeclaration,
    TSK_ExplicitInstantiationDefinition,
  )
}


std::string ClangUtil::templateArgsOrParamsForClassIfT(
  clang::CXXRecordDecl const *cxxRecordDecl, bool wantParams) const
{
  if (clang::ClassTemplateDecl const *classTemplateDecl =
        cxxRecordDecl->getDescribedClassTemplate()) {
    clang::TemplateParameterList const *params =
      classTemplateDecl->getTemplateParameters();
    if (wantParams) {
      return stringb(templateParameterListStr(params) << "\n");
    }
    else {
      // Unlike in the partial spec case, there is no
      // 'ClassTemplateDecl::getTemplateArgsAsWritten' method.
      return templateParameterListArgsStr(params);
    }
  }

  else if (auto partialSpecDecl =
             dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(
               cxxRecordDecl)) {
    if (wantParams) {
      clang::TemplateParameterList const *params =
        partialSpecDecl->getTemplateParameters();
      return stringb(templateParameterListStr(params) << "\n");
    }
    else {
      // Here, if I were to use the same procedure as for total
      // specializations, I would get names like "type-parameter-0-0"
      // showing up.  Using the "AST" arguments avoids that problem.
      clang::ASTTemplateArgumentListInfo const *args =
        partialSpecDecl->getTemplateArgsAsWritten();
      return astTemplateArgumentListInfoStr(*args);
    }
  }

  else if (auto specDecl =
             dyn_cast<clang::ClassTemplateSpecializationDecl>(cxxRecordDecl)) {
    if (wantParams) {
      // For a method of a class template total specialization, one does
      // not declare a template parameter list at all.
      return "";
    }
    else {
      return templateArgumentListStr(specDecl->getTemplateArgs());
    }
  }

  else {
    return "";
  }
}


std::string ClangUtil::templateParamsForClassIfT(
  clang::CXXRecordDecl const *cxxRecordDecl) const
{
  return templateArgsOrParamsForClassIfT(cxxRecordDecl,
                                         true /*wantParams*/);
}


std::string ClangUtil::templateArgsForClassIfT(
  clang::CXXRecordDecl const *cxxRecordDecl) const
{
  return templateArgsOrParamsForClassIfT(cxxRecordDecl,
                                         false /*wantParams*/);
}


std::string ClangUtil::templateParamsForFunctionIfT(
  clang::FunctionDecl const *functionDecl) const
{
  if (clang::FunctionTemplateDecl const *functionTemplateDecl =
        functionDecl->getDescribedFunctionTemplate()) {
    clang::TemplateParameterList const *params =
      functionTemplateDecl->getTemplateParameters();
    return stringb(templateParameterListStr(params) << "\n");
  }

  else {
    return "";
  }
}


std::string ClangUtil::templateNameStr(
  clang::TemplateName const &templateName) const
{
  string ret;
  llvm::raw_string_ostream rso(ret);
  templateName.print(rso, m_printingPolicy);
  return ret;
}


/*static*/ std::string ClangUtil::templateNameKindStr(
  clang::TemplateName::NameKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::TemplateName::, NameKind, kind,

    Template,
    OverloadedTemplate,
    AssumedTemplate,
    QualifiedTemplate,
    DependentTemplate,
    SubstTemplateTemplateParm,
    SubstTemplateTemplateParmPack,
    UsingTemplate,
  )
}


std::string ClangUtil::templateNameAndKindStr(
  clang::TemplateName const &templateName) const
{
  return stringb(templateNameStr(templateName) << " (" <<
                 templateNameKindStr(templateName.getKind()) << ")");
}


bool ClangUtil::inMainFile(SourceLocation loc) const
{
  if (!loc.isValid()) {
    return false;
  }

  if (loc.isMacroID()) {
    // Use the place the macro was expanded.
    loc = m_srcMgr.getExpansionLoc(loc);
  }

  return m_srcMgr.getFileID(loc) == m_mainFileID;
}


clang::Type const *ClangUtil::desugar(clang::Type const *type) const
{
  // This one evidently does not require 'm_astContext', while the
  // QualType version does.  That seems strange.  But, I want my API
  // signatures to be consistent, so I'm making both of these methods
  // non-static, even though this one could be static.
  return type->getUnqualifiedDesugaredType();
}


clang::QualType ClangUtil::desugar(clang::QualType type) const
{
  return type.getDesugaredType(m_astContext);
}


string ClangUtil::getIncludeSyntax(
  clang::HeaderSearchOptions const &headerSearchOptions,
  string const &fname,
  int * NULLABLE userEntryIndex)
{
  // Avoid having to awkwardly check the pointer below.
  int dummy = 0;
  if (!userEntryIndex) {
    userEntryIndex = &dummy;
  }

  // Use naive ordered search.  This isn't 100% correct in certain
  // scenarios (e.g., involving -iquote), but should suffice for my
  // purposes.
  *userEntryIndex = 0;
  for (auto const &e : headerSearchOptions.UserEntries) {
    if (startsWith(fname, e.Path)) {
      // Get the name without the path prefix.  The path is assumed to
      // end with a directory separator, and that is stripped from
      // 'fname' too.
      string relative = fname.substr(e.Path.size() + 1);

      if (e.Group == clang::frontend::Angled) {
        // Perhaps ironically, paths specified with the -I option are
        // classified by clang as "angled", but conventionally the
        // names in such directories are referenced using quotes.
        return string("\"") + relative + "\"";
      }
      else {
        return string("<") + relative + ">";
      }
    }

    ++(*userEntryIndex);
  }

  if (startsWith(fname, "./")) {
    *userEntryIndex = -1;

    // Drop the "." path component.
    string trimmed = fname.substr(2);
    return string("\"") + trimmed + "\"";
  }

  // Not found among the search paths, use the absolute name with quotes.
  *userEntryIndex = -2;
  return string("\"") + fname + "\"";
}


/*static*/ bool ClangUtil::isPrivateHeaderName(string const &fname)
{
  // GNU libc/c++ private.
  if (hasSubstring(fname, "/bits/")) {
    return true;
  }

  // More GNU libc++ private?  In particular, std::uninitialized_copy
  // shows up in pstl/glue_memory_defs.h when using Clang+LLVM 16
  // unless I exclude this (in which case it is reported as being in
  // <memory>, as it should).
  if (hasSubstring(fname, "/pstl/")) {
    return true;
  }

  // Clang macro metaprogram file extensions.
  if (endsWith(fname, ".def") ||
      endsWith(fname, ".inc")) {
    return true;
  }

  // Exclude ext/alloc_traits.h (?).
  if (hasSubstring(fname, "/ext/")) {
    return true;
  }

  return false;
}


/*static*/ bool ClangUtil::isPrivateHeaderEntry(
  clang::FileEntry const *entry)
{
  return isPrivateHeaderName(entry->getName().str());
}


string ClangUtil::getFnameForFileID(clang::FileID fileID) const
{
  clang::FileEntry const *entry = m_srcMgr.getFileEntryForID(fileID);
  assert(entry);
  return entry->getName().str();
}


clang::FileID ClangUtil::getExpansionFileID(clang::SourceLocation loc) const
{
  loc = m_srcMgr.getExpansionLoc(loc);
  return m_srcMgr.getFileID(loc);
}


string ClangUtil::getParamTypeString(clang::QualType qualType) const
{
  clang::PrintingPolicy pp(m_printingPolicy);

  pp.SuppressDefaultTemplateArgs = true;
  pp.SuppressTemplateArgsInCXXConstructors = true;

  return qualType.getAsString(pp);
}


string ClangUtil::removeTemplateArguments(string const &src)
{
  string ret(src);

  // Temporarily hide the angle brackets that appear as part of operator
  // names.
  ret = replaceAll(ret, "operator<<", "operator{{");
  ret = replaceAll(ret, "operator<", "operator{");
  ret = replaceAll(ret, "operator>>", "operator}}");
  ret = replaceAll(ret, "operator>", "operator}");
  ret = replaceAll(ret, "operator->", "operator-}");

  // Current nesting depth of angle brackets.
  int angleBracketDepth = 0;

  // Start of outermost angle brackets.
  string::size_type angleStart = 0;

  for (string::size_type i = 0; i < ret.size(); ++i) {
    if (ret[i] == '<') {
      if (angleBracketDepth == 0) {
        angleStart = i;
      }
      angleBracketDepth++;
    }
    else if (ret[i] == '>') {
      if (angleBracketDepth > 0) {
        angleBracketDepth--;
        if (angleBracketDepth == 0) {
          // Remove everything between and including the angle brackets.
          string::size_type removeLen = (i+1) - angleStart;
          ret.replace(angleStart, removeLen, "");

          // Put the cursor just before where the brackets started, so
          // that after the increment, we will be looking at the
          // character that was immediately after the brackets.
          i = angleStart-1;
        }
      }
    }
  }

  // Restore the operator names.
  ret = replaceAll(ret, "operator{{", "operator<<");
  ret = replaceAll(ret, "operator{", "operator<");
  ret = replaceAll(ret, "operator}}", "operator>>");
  ret = replaceAll(ret, "operator}", "operator>");
  ret = replaceAll(ret, "operator-}", "operator->");

  return ret;
}


string ClangUtil::publicPresumedFname(SourceLocation loc)
{
  // Get the location as influenced by #line directives.
  //
  // This is especially important if 'loc' is in a macro expansion,
  // since in that case, asking for the file name in the usual way (with
  // 'SourceManager::getFilename') yields an empty string!
  clang::PresumedLoc presumedLoc = m_srcMgr.getPresumedLoc(loc);
  string fname = presumedLoc.getFilename();

  // If 'fname' is a private header, move up the #include DAG to one
  // that is not.
  if (isPrivateHeaderName(fname)) {
    // Figure out where 'fname' was included.
    SourceLocation includeLoc = presumedLoc.getIncludeLoc();
    if (includeLoc.isValid()) {
      return publicPresumedFname(includeLoc);
    }
    else {
      // This shouldn't happen since the root of the DAG is the
      // primary source file, which should not be regarded as
      // private, but if it happens, then just forget about going
      // up and report the "private" header.
    }
  }

  return fname;
}


clang::RecordDecl *ClangUtil::maybeGetParentClass(
  clang::NamedDecl *decl)
{
  if (auto methodDecl = dyn_cast<clang::CXXMethodDecl>(decl)) {
    return methodDecl->getParent();
  }
  if (auto fieldDecl = dyn_cast<clang::FieldDecl>(decl)) {
    return fieldDecl->getParent();
  }
  return nullptr;
}


clang::NamedDecl const *ClangUtil::maybeGetNamedParentC(
  clang::NamedDecl const *decl) const
{
  // This is very loosely based on NamedDecl::printNestedNameSpecifier.

  clang::DeclContext const *parent = decl->getDeclContext();
  while (parent) {
    if (parent->isFunctionOrMethod()) {
      // We cannot name 'decl' using a qualified name because it is
      // defined inside a function.
      return nullptr;
    }

    if (auto nd = dyn_cast<clang::NamedDecl>(parent)) {
      // We found a named ancestor.
      if (parent->isInlineNamespace()) {
        // This is like 'std::__cxx11', which I do not want.
      }
      else {
        return nd;
      }
    }

    // The parent does not have a suitable name, so look at its parent.
    parent = parent->getParent();
  }

  // 'decl' is in the global scope.
  return nullptr;
}


clang::NamedDecl *ClangUtil::maybeGetNamedParent(
  clang::NamedDecl *decl) const
{
  return const_cast<clang::NamedDecl*>(maybeGetNamedParentC(decl));
}


string ClangUtil::getTopLevelIncludeForLoc(
  clang::SourceLocation loc) const
{
  // This is initially invalid.
  clang::PresumedLoc prevPresumedLoc;

  if (loc.isValid()) {
    clang::PresumedLoc presumedLoc = m_srcMgr.getPresumedLoc(loc);
    assert(presumedLoc.isValid());

    while (presumedLoc.getIncludeLoc().isValid()) {
      prevPresumedLoc = presumedLoc;
      presumedLoc = m_srcMgr.getPresumedLoc(presumedLoc.getIncludeLoc());
    }
  }

  if (prevPresumedLoc.isValid()) {
    // 'prevPresumedLoc' is the last presumed location that had a valid
    // include location.  Therefore, its file will be one that was
    // directly included by the main source file.
    //
    // BUG: I would prefer to use 'getFileID' here, but sometimes that
    // is invalid even when the file name is not.
    return prevPresumedLoc.getFilename();
  }
  else {
    return string("");
  }
}


string ClangUtil::getDeclKeyword(clang::NamedDecl const *decl) const
{
  if (isa<clang::NamespaceDecl>(decl)) {
    return "namespace";
  }

  if (auto tagDecl = dyn_cast<clang::TagDecl>(decl)) {
    return tagDecl->getKindName().str();
  }

  return "";
}


string ClangUtil::templateParameterListStr(
  clang::TemplateParameterList const *paramList) const
{
  bool omitTemplateKW = false;

  string ret;
  llvm::raw_string_ostream rso(ret);

  paramList->print(rso, m_astContext, m_printingPolicy, omitTemplateKW);

  // Remove default arguments since I'm using this to generate the
  // declaration for the Names file.
  ret = removeDefaultTemplateArguments(ret);

  // Even without 'SplitTemplateClosers', the printer puts an extra
  // space after the final '>', and I don't want that.
  return trimWhitespace(ret);
}


std::string ClangUtil::encloseInAngleBrackets(
  std::list<std::string> const &args,
  bool hasParameterPack) const
{
  std::ostringstream oss;
  oss << '<';

  int ct=0;
  for (string const &arg : args) {
    if (ct++ > 0) {
      oss << ", ";
    }
    oss << arg;
  }

  if (hasParameterPack) {
    oss << "...";
  }
  else if (endsWith(oss.str(), ">")) {
    oss << ' ';
  }

  oss << '>';
  return oss.str();
}


std::string ClangUtil::templateParameterListArgsStr(
  clang::TemplateParameterList const *paramList) const
{
  std::list<string> argStrings;
  for (clang::NamedDecl *param : *paramList) {
    argStrings.push_back(param->getNameAsString());
  }

  return encloseInAngleBrackets(argStrings,
                                paramList->hasParameterPack());
}


/*static*/ std::string ClangUtil::templateArgumentKindStr(
  clang::TemplateArgument::ArgKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::TemplateArgument::, ArgKind, kind,

    Null,
    Type,
    Declaration,
    NullPtr,
    Integral,
    Template,
    TemplateExpansion,
    Expression,
    Pack,
  )
}


std::string ClangUtil::templateArgumentStr(
  clang::TemplateArgument const &arg) const
{
  string ret;
  llvm::raw_string_ostream rso(ret);

  // Based on reading the code, this flag will cause non-type arguments
  // to include an explicit indication of their type, such as an "L"
  // suffix on longs or a cast to an enumeration type.  That seems
  // potentially useful, so my initial attempt is to turn in on.
  bool const includeType = true;

  arg.print(m_printingPolicy, rso, includeType);

  return ret;
}


std::string ClangUtil::templateArgumentAndKindStr(
  clang::TemplateArgument const &arg) const
{
  return stringb(doubleQuote(templateArgumentStr(arg)) << " (ArgKind::" <<
                 templateArgumentKindStr(arg.getKind()) << ")");
}


std::string ClangUtil::templateArgumentListStr(
  clang::TemplateArgumentList const &args) const
{
  std::list<string> argStrings;
  for (clang::TemplateArgument const &arg : args.asArray()) {
    argStrings.push_back(templateArgumentStr(arg));
  }

  // TODO: Shouldn't there be a flag in the AST for this?
  bool hasParameterPack = false;

  return encloseInAngleBrackets(argStrings, hasParameterPack);
}


std::string ClangUtil::templateArgumentListOptStr(
  clang::TemplateArgumentList const * NULLABLE args) const
{
  if (args) {
    return doubleQuote(templateArgumentListStr(*args));
  }
  else {
    return "null";
  }
}


std::string ClangUtil::astTemplateArgumentListInfoStr(
  clang::ASTTemplateArgumentListInfo const &argsInfo) const
{
  std::list<string> argStrings;
  for (clang::TemplateArgumentLoc const &arg : argsInfo.arguments()) {
    argStrings.push_back(templateArgumentStr(arg.getArgument()));
  }

  // TODO: Shouldn't there be a flag in the AST for this?
  bool hasParameterPack = false;

  return encloseInAngleBrackets(argStrings, hasParameterPack);
}


std::string ClangUtil::astTemplateArgumentListInfoOptStr(
  clang::ASTTemplateArgumentListInfo const * NULLABLE argsInfo) const
{
  if (argsInfo) {
    return astTemplateArgumentListInfoStr(*argsInfo);
  }
  else {
    return "null";
  }
}


clang::NamedDecl const *ClangUtil::canonicalNamedDeclC(
  clang::NamedDecl const *decl) const
{
  assert(decl != nullptr);
  clang::NamedDecl const *canonical =
    dyn_cast<clang::NamedDecl>(decl->getCanonicalDecl());
  assert(canonical != nullptr);
  return canonical;
}


clang::NamedDecl *ClangUtil::canonicalNamedDecl(
  clang::NamedDecl *decl) const
{
  return const_cast<clang::NamedDecl*>(canonicalNamedDeclC(decl));
}


/*static*/ std::string ClangUtil::apValueKindStr(
  clang::APValue::ValueKind kind)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::APValue::, ValueKind, kind,

    None,
    Indeterminate,
    Int,
    Float,
    FixedPoint,
    ComplexInt,
    ComplexFloat,
    LValue,
    Vector,
    Array,
    Struct,
    Union,
    MemberPointer,
    AddrLabelDiff,
  )
}


/*static*/ std::string ClangUtil::apValueStr(
  clang::APValue const * NULLABLE apValue)
{
  if (apValue) {
    std::ostringstream oss;

    // APValue::dump produces a voluminous tree, and
    // APValue::prettyPrint requires a QualType.  I just want a compact
    // representation of whatever is stored.

    switch (apValue->getKind()) {
      case clang::APValue::None:
        oss << "None";
        break;

      case clang::APValue::Indeterminate:
        oss << "Indeterminate";
        break;

      case clang::APValue::Int:
        oss << apsIntStr(apValue->getInt());
        break;

      default:
        oss << "Unimplemented APValue kind: "
            << apValueKindStr(apValue->getKind());
        break;
    }

    return oss.str();
  }
  else {
    return "null";
  }
}


/*static*/ std::string ClangUtil::smallVectorStr(
  llvm::SmallVectorImpl<char> const &vec)
{
  std::ostringstream oss;

  for (char c : vec) {
    oss << c;
  }

  return oss.str();
}


/*static*/ std::string ClangUtil::apIntStr(llvm::APInt const &n,
                                           bool isSigned)
{
  llvm::SmallVector<char, 20> digits;
  n.toString(digits, 10 /*radix*/, isSigned);
  return smallVectorStr(digits);
}


/*static*/ std::string ClangUtil::apsIntStr(llvm::APSInt const &n)
{
  llvm::SmallVector<char, 20> digits;
  n.toString(digits);
  return smallVectorStr(digits);
}


bool ClangUtil::isOperatorDecl(clang::NamedDecl const *decl) const
{
  if (auto functionDecl = dyn_cast<clang::FunctionDecl>(decl)) {
    clang::DeclarationNameInfo dni = functionDecl->getNameInfo();
    clang::DeclarationName dn = dni.getName();
    return dn.getNameKind() == clang::DeclarationName::CXXOperatorName;
  }
  return false;
}


bool ClangUtil::locInSourceFile(clang::SourceLocation loc) const
{
  if (!loc.isValid()) {
    return false;
  }

  if (!loc.isFileID()) {
    // Macro expansion.
    return false;
  }

  clang::FileEntry const *entry = getFileEntryForLoc(loc);
  if (!entry) {
    // A location referring to "<command line>" ends up in this case.
    return false;
  }

  // I could also check for a named pipe, but I'm not sure when that can
  // happen or how I would want to treat it.

  // Seems to be a legit file.
  return true;
}


clang::FileEntry const *ClangUtil::getFileEntryForLoc(
  clang::SourceLocation loc) const
{
  // Previously, I had been calling 'getSpellingLoc' here, although I do
  // not recall why.
  //
  // For dependency analysis, I think I generally want to depend on the
  // place that a name was expanded, rather than originally written.
  // Furthermore, in a case like in/src/token-paste-client.cc, the
  // spelling location can be "scratch space", so only the expansion
  // location has an associated file.
  loc = m_srcMgr.getExpansionLoc(loc);

  clang::FileID fid = m_srcMgr.getFileID(loc);
  if (!fid.isValid()) {
    return nullptr;
  }

  return m_srcMgr.getFileEntryForID(fid);
}


clang::SourceLocation ClangUtil::getDeclPrecedingTokenLoc(
  clang::Decl const *decl) const
{
  // Find the declaration before 'decl'.
  //
  // TODO: This does not work if 'decl' appears in a context that allows
  // non-declarations, such as a compound statement.
  clang::Decl const *prevDecl = nullptr;
  clang::DeclContext const *parent = decl->getLexicalDeclContext();
  for (clang::Decl const *child : parent->decls()) {
    // For reasons I do not understand, I have to call
    // Decl::getLocation() here, not declLoc(), for this to work.
    if (child->getLocation() == decl->getLocation()) {
      break;
    }
    prevDecl = child;
  }

  if (prevDecl) {
    return prevDecl->getEndLoc();
  }

  assert(!"getDeclPrecedingTokenLoc: unimplemented: decl is first in its context");
  return clang::SourceLocation();
}


/*static*/ clang::FunctionDecl const *ClangUtil::getUserWrittenFunctionDecl(
  clang::FunctionDecl const *fd)
{
  assert(fd);

  // I'm not sure if this has precisely the semantics I want yet.
  clang::FunctionDecl const *pattern =
    fd->getTemplateInstantiationPattern();

  return pattern? pattern : fd;
}


/*static*/ clang::FunctionDecl *ClangUtil::getUserWrittenFunctionDecl(
  clang::FunctionDecl *fd)
{
  return const_cast<clang::FunctionDecl*>(
    getUserWrittenFunctionDecl(
      const_cast<clang::FunctionDecl const *>(fd)));
}


/*static*/ bool ClangUtil::isUserWrittenFunctionDecl(
  clang::FunctionDecl const *fd)
{
  return fd == getUserWrittenFunctionDecl(fd);
}


bool ClangUtil::getPrimarySourceFileLines(std::vector<std::string> &lines)
{
  // Get the entire text of the PSF.
  clang::FileID psfFileId = m_srcMgr.getMainFileID();
  std::optional<llvm::StringRef> textOpt =
    m_srcMgr.getBufferDataOrNone(psfFileId);
  if (!textOpt) {
    return false;
  }
  llvm::StringRef const &text = *textOpt;

  // Index of the start of the next line to gather.
  size_t i = 0;

  // Parse lines at newline characters.
  while (i < text.size()) {
    // Set 'j' to one past the last character in the next line (which
    // is a newline character unless we hit EOF first).
    size_t j = i;
    while (j < text.size() && text[j] != '\n') {
      ++j;
    }
    if (j < text.size()) {
      assert(text[j] == '\n');
      ++j;
    }

    // Grab the line.
    lines.push_back(text.substr(i, j-i).str());

    // Ensure progress.
    assert(j > i);

    // Move to the next line.
    i = j;
  }

  return true;
}


std::string stringRefRange(
  llvm::StringRef const &sr, unsigned begin, unsigned end)
{
  assert(begin <= end);
  assert(end <= sr.size());
  return sr.substr(begin, end-begin).str();
}


int compare(clang::SourceRange const &a, clang::SourceRange const &b)
{
  int ret = compare(a.getBegin(), b.getBegin());
  if (ret) { return ret; }

  ret = compare(a.getEnd(), b.getEnd());
  if (ret) { return ret; }

  return 0;
}


/*static*/ int DeclCompare::compare(clang::Decl const *a,
                                    clang::Decl const *b)
{
  // The ID of a decl is calculated from its address by walking up the
  // DeclContext tree to get to the ASTContext, then iterating over the
  // allocator slabs to find the one that contains it, then calculating
  // an ID based on the position in the slab.  Under the assumption that
  // objects are allocated in a deterministic order during parsing, and
  // that object locations in a slab are never re-used, the resulting ID
  // will be consistent across program executions.
  //
  // However, it is somewhat expensive to compute because walking up the
  // DC tree takes time roughly logarithmic in the size of the TU, and
  // I'm not sure about the cost of walking the slabs; that too is at
  // least logarithmic and possibly linear.

  return ::compare(a->getID(), b->getID());
}


std::string getDynamicTypeClassName(clang::Type const *type)
{
  return stringb(type->getTypeClassName() << "Type");
}


std::string getDynamicTypeClassName(clang::Decl const *decl)
{
  return stringb(decl->getDeclKindName() << "Decl");
}


std::string getDynamicTypeClassName(clang::Stmt const *stmt)
{
  return stmt->getStmtClassName();
}


// Return the Requirement subclass name corresponding to 'rkind'.
static std::string requirementKindToClassName(
  clang::concepts::Requirement::RequirementKind rkind)
{
  static struct Entry {
    clang::concepts::Requirement::RequirementKind m_rkind;
    char const *m_name;
  } const entries[] = {
    #define ENTRY(kind, name) { clang::concepts::Requirement::kind, name },

    ENTRY(RK_Type,     "TypeRequirement")
    ENTRY(RK_Simple,   "ExprRequirement")
    ENTRY(RK_Compound, "ExprRequirement")
    ENTRY(RK_Nested,   "NestedRequirement")

    #undef ENTRY
  };

  for (Entry const &e : entries) {
    if (e.m_rkind == rkind) {
      return e.m_name;
    }
  }

  return stringb("RequirementKindClass(" << (int)rkind << ")");
}


std::string getDynamicTypeClassName(clang::concepts::Requirement const *req)
{
  return requirementKindToClassName(req->getKind());
}


void assert_dyn_cast_null(
  char const *destTypeName,
  char const *sourceFile,
  int sourceLine)
{
  die_fileLine(stringb("dyn_cast of null: destType=" << destTypeName),
               sourceFile, sourceLine);
}


void assert_dyn_cast_failed(
  char const *destTypeName,
  std::string const &srcTypeName,
  char const *sourceFile,
  int sourceLine)
{
  die_fileLine(stringb("dyn_cast failed: srcType=" << srcTypeName <<
                       " destType=" << destTypeName),
               sourceFile, sourceLine);
}


// EOF
