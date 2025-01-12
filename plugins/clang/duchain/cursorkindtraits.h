/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CURSORKINDTRAITS_H
#define CURSORKINDTRAITS_H

#include <clang-c/Index.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/types/integraltype.h>

#include "templatehelpers.h"

namespace CursorKindTraits {

using namespace KDevelop;

constexpr bool isClassTemplate(CXCursorKind CK)
{
    return CK == CXCursor_ClassTemplate || CK == CXCursor_ClassTemplatePartialSpecialization;
}

constexpr bool isClass(CXCursorKind CK)
{
    return isClassTemplate(CK)
    || CK == CXCursor_StructDecl
    || CK == CXCursor_ClassDecl
    || CK == CXCursor_UnionDecl
    || CK == CXCursor_ObjCInterfaceDecl
    || CK == CXCursor_ObjCCategoryDecl
    || CK == CXCursor_ObjCImplementationDecl
    || CK == CXCursor_ObjCCategoryImplDecl;
}

constexpr bool isFunction(CXCursorKind CK)
{
    return CK == CXCursor_FunctionDecl
    || CK == CXCursor_CXXMethod
    || CK == CXCursor_Constructor
    || CK == CXCursor_Destructor
    || CK == CXCursor_ConversionFunction
    || CK == CXCursor_FunctionTemplate
    || CK == CXCursor_ObjCInstanceMethodDecl
    || CK == CXCursor_ObjCClassMethodDecl;
}

constexpr bool isDeclaration(CXCursorKind CK)
{
    return isClass(CK) || isFunction(CK)
    || CK == CXCursor_UnexposedDecl
    || CK == CXCursor_FieldDecl
    || CK == CXCursor_ObjCProtocolDecl
    || CK == CXCursor_ObjCPropertyDecl
    || CK == CXCursor_ObjCIvarDecl
    || CK == CXCursor_EnumConstantDecl
    || CK == CXCursor_VarDecl
    || CK == CXCursor_ParmDecl
    || CK == CXCursor_TypedefDecl
    || CK == CXCursor_TemplateTypeParameter
    || CK == CXCursor_NonTypeTemplateParameter
    || CK == CXCursor_TemplateTemplateParameter
    || CK == CXCursor_NamespaceAlias
    || CK == CXCursor_UsingDirective
    || CK == CXCursor_UsingDeclaration
    || CK == CXCursor_TypeAliasDecl
    || CK == CXCursor_LabelStmt;
}

constexpr Decision isDefinition(CXCursorKind CK)
{
    return CK == CXCursor_Namespace || CK == CXCursor_MacroDefinition ?
        Decision::True :
        isClass(CK) || isFunction(CK) || CK == CXCursor_EnumDecl ?
        Decision::Maybe :
        Decision::False;
}

constexpr Decision isInClass(CXCursorKind CK)
{
    return CK == CXCursor_FieldDecl
        || CK == CXCursor_ObjCPropertyDecl
        || CK == CXCursor_ObjCIvarDecl
        || CK == CXCursor_ObjCInstanceMethodDecl
        || CK == CXCursor_ObjCClassMethodDecl ?
        Decision::True :
           CK == CXCursor_Namespace
        || CK == CXCursor_TemplateTypeParameter
        || CK == CXCursor_FunctionDecl
        || CK == CXCursor_TemplateTemplateParameter
        || CK == CXCursor_NonTypeTemplateParameter
        || CK == CXCursor_MacroDefinition
        || CK == CXCursor_MacroExpansion ?
        Decision::False :
        Decision::Maybe;
}

constexpr DUContext::ContextType contextType(CXCursorKind CK)
{
    return CK == CXCursor_StructDecl                    ? DUContext::Class
    : CK == CXCursor_UnionDecl                          ? DUContext::Class
    : CK == CXCursor_ClassDecl                          ? DUContext::Class
    : CK == CXCursor_EnumDecl                           ? DUContext::Enum
    : CK == CXCursor_FunctionDecl                       ? DUContext::Function
    : CK == CXCursor_CXXMethod                          ? DUContext::Function
    : CK == CXCursor_Namespace                          ? DUContext::Namespace
    : CK == CXCursor_Constructor                        ? DUContext::Function
    : CK == CXCursor_Destructor                         ? DUContext::Function
    : CK == CXCursor_ConversionFunction                 ? DUContext::Function
    : CK == CXCursor_FunctionTemplate                   ? DUContext::Function
    : CK == CXCursor_ClassTemplate                      ? DUContext::Class
    : CK == CXCursor_ClassTemplatePartialSpecialization ? DUContext::Class
    : CK == CXCursor_MacroDefinition                    ? DUContext::Other
    : static_cast<DUContext::ContextType>(-1);
}

constexpr bool isKDevDeclaration(CXCursorKind CK, bool isClassMember)
{
    return !isClassMember &&
    (CK == CXCursor_UnexposedDecl
    || CK == CXCursor_FieldDecl
    || CK == CXCursor_ObjCProtocolDecl
    || CK == CXCursor_ObjCPropertyDecl
    || CK == CXCursor_ObjCIvarDecl
    || CK == CXCursor_EnumConstantDecl
    || CK == CXCursor_VarDecl
    || CK == CXCursor_ParmDecl
    || CK == CXCursor_TypedefDecl
    || CK == CXCursor_TemplateTypeParameter
    || CK == CXCursor_NonTypeTemplateParameter
    || CK == CXCursor_TemplateTemplateParameter
    || CK == CXCursor_UsingDirective
    || CK == CXCursor_UsingDeclaration
    || CK == CXCursor_TypeAliasDecl
    || CK == CXCursor_Namespace
    || CK == CXCursor_EnumDecl
    || CK == CXCursor_LabelStmt);
}

constexpr bool isKDevClassDeclaration(CXCursorKind CK, bool isDefinition)
{
    return isDefinition && isClass(CK);
}

constexpr bool isKDevForwardDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && isClass(CK);
}

constexpr bool isKDevClassFunctionDeclaration(CXCursorKind CK, bool isInClass)
{
    return isInClass && isFunction(CK);
}

constexpr bool isKDevFunctionDeclaration(CXCursorKind CK, bool isDefinition, bool isInClass)
{
    return !isDefinition && !isInClass && isFunction(CK);
}

constexpr bool isKDevFunctionDefinition(CXCursorKind CK, bool isDefinition, bool isInClass)
{
    return isDefinition && !isInClass && isFunction(CK);
}

constexpr bool isKDevNamespaceAliasDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && CK == CXCursor_NamespaceAlias;
}

constexpr bool isKDevClassMemberDeclaration(CXCursorKind CK, bool isInClass)
{
    return isInClass && isKDevDeclaration(CK, false);
}

constexpr Declaration::AccessPolicy kdevAccessPolicy(CX_CXXAccessSpecifier access)
{
    return access == CX_CXXPrivate ? Declaration::Private
    : access == CX_CXXProtected ?    Declaration::Protected
    : access == CX_CXXPublic ?       Declaration::Public
    :                                Declaration::DefaultAccess;
}

constexpr IntegralType::CommonIntegralTypes integralType(CXTypeKind TK)
{
    return TK == CXType_Void    ? IntegralType::TypeVoid
    : TK == CXType_Bool         ? IntegralType::TypeBoolean
    : TK == CXType_Half         ? IntegralType::TypeHalf
    : TK == CXType_Float        ? IntegralType::TypeFloat
    : TK == CXType_Char16       ? IntegralType::TypeChar16_t
    : TK == CXType_Char32       ? IntegralType::TypeChar32_t
    : TK == CXType_WChar        ? IntegralType::TypeWchar_t
    : ( TK == CXType_LongDouble
      ||TK == CXType_Double
#if CINDEX_VERSION_MINOR >= 38
      ||TK == CXType_Float128
#endif
                             )    ? IntegralType::TypeDouble
    : ( TK == CXType_Short
      ||TK == CXType_UShort
      ||TK == CXType_Int
      ||TK == CXType_UInt
      ||TK == CXType_Long
      ||TK == CXType_ULong
      ||TK == CXType_LongLong
      ||TK == CXType_ULongLong) ? IntegralType::TypeInt
    : ( TK == CXType_Char_U
      ||TK == CXType_Char_S
      ||TK == CXType_UChar
      ||TK == CXType_SChar)     ?  IntegralType::TypeChar
    : static_cast<IntegralType::CommonIntegralTypes>(-1);
}

constexpr bool isArrayType(CXTypeKind TK)
{
    return TK == CXType_ConstantArray
        || TK == CXType_IncompleteArray
        || TK == CXType_VariableArray
        || TK == CXType_DependentSizedArray;
}

constexpr bool isPointerType(CXTypeKind TK)
{
    return TK == CXType_Pointer
        || TK == CXType_BlockPointer
        || TK == CXType_MemberPointer
        || TK == CXType_ObjCObjectPointer;
}


constexpr bool isOpenCLType(CXTypeKind CK)
{
    return (CK == CXType_OCLImage1dRO ||
        CK == CXType_OCLImage1dArrayRO ||
        CK == CXType_OCLImage1dBufferRO ||
        CK == CXType_OCLImage2dRO ||
        CK == CXType_OCLImage2dArrayRO ||
        CK == CXType_OCLImage2dDepthRO ||
        CK == CXType_OCLImage2dArrayDepthRO ||
        CK == CXType_OCLImage2dMSAARO ||
        CK == CXType_OCLImage2dArrayMSAARO ||
        CK == CXType_OCLImage2dMSAADepthRO ||
        CK == CXType_OCLImage2dArrayMSAADepthRO ||
        CK == CXType_OCLImage3dRO ||
        CK == CXType_OCLImage1dWO ||
        CK == CXType_OCLImage1dArrayWO ||
        CK == CXType_OCLImage1dBufferWO ||
        CK == CXType_OCLImage2dWO ||
        CK == CXType_OCLImage2dArrayWO ||
        CK == CXType_OCLImage2dDepthWO ||
        CK == CXType_OCLImage2dArrayDepthWO ||
        CK == CXType_OCLImage2dMSAAWO ||
        CK == CXType_OCLImage2dArrayMSAAWO ||
        CK == CXType_OCLImage2dMSAADepthWO ||
        CK == CXType_OCLImage2dArrayMSAADepthWO ||
        CK == CXType_OCLImage3dWO ||
        CK == CXType_OCLImage1dRW ||
        CK == CXType_OCLImage1dArrayRW ||
        CK == CXType_OCLImage1dBufferRW ||
        CK == CXType_OCLImage2dRW ||
        CK == CXType_OCLImage2dArrayRW ||
        CK == CXType_OCLImage2dDepthRW ||
        CK == CXType_OCLImage2dArrayDepthRW ||
        CK == CXType_OCLImage2dMSAARW ||
        CK == CXType_OCLImage2dArrayMSAARW ||
        CK == CXType_OCLImage2dMSAADepthRW ||
        CK == CXType_OCLImage2dArrayMSAADepthRW ||
        CK == CXType_OCLImage3dRW ||
        CK == CXType_OCLSampler ||
        CK == CXType_OCLEvent ||
        CK == CXType_OCLQueue ||
        CK == CXType_OCLReserveID);
}

constexpr bool isAliasType(CXCursorKind CK)
{
    return CK == CXCursor_TypedefDecl || CK == CXCursor_TypeAliasDecl;
}

constexpr bool isIdentifiedType(CXCursorKind CK)
{
    return isClass(CK) || isAliasType(CK) || CK == CXCursor_EnumDecl || CK == CXCursor_EnumConstantDecl;
}

constexpr const char16_t* delayedTypeName(CXTypeKind TK)
{
    return TK == CXType_Int128 ? u"__int128"
        : TK == CXType_UInt128 ? u"unsigned __int128"
        : TK == CXType_ObjCId  ? u"id"
        : TK == CXType_ObjCSel ? u"SEL"
        : TK == CXType_NullPtr ? u"nullptr_t"
                               : nullptr;
}

}

#endif //CURSORKINDTRAITS_H
