/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "duchainutils.h"

#include <algorithm>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "../interfaces/ilanguagesupport.h"
#include "../assistant/staticassistantsmanager.h"
#include <debug.h>

#include "declaration.h"
#include "classfunctiondeclaration.h"
#include "ducontext.h"
#include "duchain.h"
#include "use.h"
#include "duchainlock.h"
#include "classmemberdeclaration.h"
#include "functiondefinition.h"
#include "specializationstore.h"
#include "persistentsymboltable.h"
#include "classdeclaration.h"
#include "parsingenvironment.h"

#include <QStandardPaths>

using namespace KDevelop;
using namespace KTextEditor;

CodeCompletionModel::CompletionProperties DUChainUtils::completionProperties(const Declaration* dec)
{
  CodeCompletionModel::CompletionProperties p;

  if(dec->context()->type() == DUContext::Class) {
    if (const auto* member = dynamic_cast<const ClassMemberDeclaration*>(dec)) {
      switch (member->accessPolicy()) {
        case Declaration::Public:
          p |= CodeCompletionModel::Public;
          break;
        case Declaration::Protected:
          p |= CodeCompletionModel::Protected;
          break;
        case Declaration::Private:
          p |= CodeCompletionModel::Private;
          break;
        default:
          break;
      }

      if (member->isStatic())
        p |= CodeCompletionModel::Static;
      if (member->isAuto())
        {}//TODO
      if (member->isFriend())
        p |= CodeCompletionModel::Friend;
      if (member->isRegister())
        {}//TODO
      if (member->isExtern())
        {}//TODO
      if (member->isMutable())
        {}//TODO
    }
  }

  if (const auto* function = dynamic_cast<const AbstractFunctionDeclaration*>(dec)) {
    p |= CodeCompletionModel::Function;
    if (function->isVirtual())
      p |= CodeCompletionModel::Virtual;
    if (function->isInline())
      p |= CodeCompletionModel::Inline;
    if (function->isExplicit())
      {}//TODO
  }

  if( dec->isTypeAlias() )
    p |= CodeCompletionModel::TypeAlias;

  if (dec->abstractType()) {
    switch (dec->abstractType()->whichType()) {
      case AbstractType::TypeIntegral:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypePointer:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeReference:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeFunction:
        p |= CodeCompletionModel::Function;
        break;
      case AbstractType::TypeStructure:
        p |= CodeCompletionModel::Class;
        break;
      case AbstractType::TypeArray:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeEnumeration:
        p |= CodeCompletionModel::Enum;
        break;
      case AbstractType::TypeEnumerator:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeAbstract:
      case AbstractType::TypeDelayed:
      case AbstractType::TypeUnsure:
      case AbstractType::TypeAlias:
        // TODO
        break;
    }

    if( dec->abstractType()->modifiers() & AbstractType::ConstModifier )
      p |= CodeCompletionModel::Const;

    if( dec->kind() == Declaration::Instance && !dec->isFunctionDeclaration() )
      p |= CodeCompletionModel::Variable;
  }

  if (dec->context()) {
    if( dec->context()->type() == DUContext::Global )
      p |= CodeCompletionModel::GlobalScope;
    else if( dec->context()->type() == DUContext::Namespace )
      p |= CodeCompletionModel::NamespaceScope;
    else if( dec->context()->type() != DUContext::Class && dec->context()->type() != DUContext::Enum )
      p |= CodeCompletionModel::LocalScope;
  }

  return p;
}
/**We have to construct the item from the pixmap, else the icon will be marked as "load on demand",
 * and for some reason will be loaded every time it's used(this function returns a QIcon marked "load on demand"
 * each time this is called). And the loading is very slow. Seems like a bug somewhere, it cannot be ment to be that slow.
 */
#define RETURN_CACHED_ICON(name) {static QIcon icon(QIcon( \
      QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevelop/pics/" name ".png"))\
    ).pixmap(QSize(16, 16)));\
    return icon;}

QIcon DUChainUtils::iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p)
{

  if( (p & CodeCompletionModel::Variable) )
    if( (p & CodeCompletionModel::Protected) )
      RETURN_CACHED_ICON("CVprotected_var")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("CVprivate_var")
    else
      RETURN_CACHED_ICON("CVpublic_var")
  else
  if( (p & CodeCompletionModel::Union) && (p & CodeCompletionModel::Protected) )
    RETURN_CACHED_ICON("protected_union")

  else if( p & CodeCompletionModel::Enum )
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("protected_enum")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_enum")
    else
      RETURN_CACHED_ICON("enum")

  else if( p & CodeCompletionModel::Struct )
    if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_struct")
    else
      RETURN_CACHED_ICON("struct")

  else if( p & CodeCompletionModel::Slot )
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("CVprotected_slot")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("CVprivate_slot")
    else if(p & CodeCompletionModel::Public )
      RETURN_CACHED_ICON("CVpublic_slot")
    else RETURN_CACHED_ICON("slot")
  else if( p & CodeCompletionModel::Signal )
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("CVprotected_signal")
    else
      RETURN_CACHED_ICON("signal")

  else if( p & CodeCompletionModel::Class )
    if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Protected) )
      RETURN_CACHED_ICON("protected_class")
    else if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Private) )
      RETURN_CACHED_ICON("private_class")
    else
      RETURN_CACHED_ICON("code-class")

  else if( p & CodeCompletionModel::Union )
    if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_union")
    else
      RETURN_CACHED_ICON("union")

  else if( p & CodeCompletionModel::TypeAlias )
    if ((p & CodeCompletionModel::Const) /*||  (p & CodeCompletionModel::Volatile)*/)
      RETURN_CACHED_ICON("CVtypedef")
    else
      RETURN_CACHED_ICON("typedef")

  else if( p & CodeCompletionModel::Function ) {
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("protected_function")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_function")
    else
      RETURN_CACHED_ICON("code-function")
  }

  if( p & CodeCompletionModel::Protected )
    RETURN_CACHED_ICON("protected_field")
  else if( p & CodeCompletionModel::Private )
    RETURN_CACHED_ICON("private_field")
  else
    RETURN_CACHED_ICON("field")

  return QIcon();
}

QIcon DUChainUtils::iconForDeclaration(const Declaration* dec)
{
  return iconForProperties(completionProperties(dec));
}

TopDUContext* DUChainUtils::contentContextFromProxyContext(TopDUContext* top)
{
  if(!top)
    return nullptr;
  if(top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->isProxyContext()) {
    if(!top->importedParentContexts().isEmpty())
    {
      DUContext* ctx = top->importedParentContexts().at(0).context(nullptr);
      if(!ctx)
        return nullptr;
      TopDUContext* ret = ctx->topContext();
      if(!ret)
        return nullptr;
      if(ret->url() != top->url())
        qCDebug(LANGUAGE) << "url-mismatch between content and proxy:" << top->url().toUrl() << ret->url().toUrl();
      if(ret->url() == top->url() && !ret->parsingEnvironmentFile()->isProxyContext())
        return ret;
    }
    else {
      qCDebug(LANGUAGE) << "Proxy-context imports no content-context";
    }
  } else
    return top;
  return nullptr;
}

TopDUContext* DUChainUtils::standardContextForUrl(const QUrl& url, bool preferProxyContext) {
  KDevelop::TopDUContext* chosen = nullptr;

  const auto languages = ICore::self()->languageController()->languagesForUrl(url);

  for (const auto language : languages) {
    if(!chosen)
    {
      chosen = language->standardContext(url, preferProxyContext);
    }
  }

  if(!chosen)
    chosen = DUChain::self()->chainForDocument(IndexedString(url), preferProxyContext);

  if(!chosen && preferProxyContext)
    return standardContextForUrl(url, false); // Fall back to a normal context

  return chosen;
}

struct ItemUnderCursorInternal
{
  Declaration* declaration;
  DUContext* context;
  RangeInRevision range;
};

ItemUnderCursorInternal itemUnderCursorInternal(const CursorInRevision& c, DUContext* ctx, RangeInRevision::ContainsBehavior behavior)
{
  //Search all collapsed sub-contexts. In C++, those can contain declarations that have ranges out of the context
  const auto childContexts = ctx->childContexts();
  for (DUContext* subCtx : childContexts) {
    //This is a little hacky, but we need it in case of foreach macros and similar stuff
    if(subCtx->range().contains(c, behavior) || subCtx->range().isEmpty() || subCtx->range().start.line == c.line || subCtx->range().end.line == c.line) {
      ItemUnderCursorInternal sub = itemUnderCursorInternal(c, subCtx, behavior);
      if(sub.declaration) {
        return sub;
      }
    }
  }

  const auto localDeclarations = ctx->localDeclarations();
  for (Declaration* decl : localDeclarations) {
    if(decl->range().contains(c, behavior)) {
      return {decl, ctx, decl->range()};
    }
  }

  //Try finding a use under the cursor
  for(int a = 0; a < ctx->usesCount(); ++a) {
    if(ctx->uses()[a].m_range.contains(c, behavior)) {
      return {ctx->topContext()->usedDeclarationForIndex(ctx->uses()[a].m_declarationIndex), ctx, ctx->uses()[a].m_range};
    }
  }

  return {nullptr, nullptr, RangeInRevision()};
}

DUChainUtils::ItemUnderCursor DUChainUtils::itemUnderCursor(const QUrl& url, const KTextEditor::Cursor& cursor)
{
  KDevelop::TopDUContext* top = standardContextForUrl(url.adjusted(QUrl::NormalizePathSegments));

  if(!top) {
    return {nullptr, nullptr, KTextEditor::Range()};
  }

  ItemUnderCursorInternal decl = itemUnderCursorInternal(top->transformToLocalRevision(cursor), top, RangeInRevision::Default);
  if (decl.declaration == nullptr)
  {
    decl = itemUnderCursorInternal(top->transformToLocalRevision(cursor), top, RangeInRevision::IncludeBackEdge);
  }
  return {decl.declaration, decl.context, top->transformFromLocalRevision(decl.range)};
}

Declaration* DUChainUtils::declarationForDefinition(Declaration* definition, TopDUContext* topContext)
{
  if(!definition)
    return nullptr;

  if(!topContext)
    topContext = definition->topContext();

  if(dynamic_cast<FunctionDefinition*>(definition)) {
    Declaration* ret = static_cast<FunctionDefinition*>(definition)->declaration();
    if(ret)
      return ret;
  }

  return definition;
}

Declaration* DUChainUtils::declarationInLine(const KTextEditor::Cursor& _cursor, DUContext* ctx) {
  if(!ctx)
    return nullptr;

  CursorInRevision cursor = ctx->transformToLocalRevision(_cursor);

  const auto localDeclarations = ctx->localDeclarations();
  for (Declaration* decl : localDeclarations) {
    if(decl->range().start.line == cursor.line)
      return decl;
    DUContext* funCtx = functionContext(decl);
    if(funCtx && funCtx->range().contains(cursor))
      return decl;
  }

  const auto childContexts = ctx->childContexts();
  for (DUContext* child : childContexts){
    Declaration* decl = declarationInLine(_cursor, child);
    if(decl)
      return decl;
  }

  return nullptr;
}

DUChainUtils::DUChainItemFilter::~DUChainItemFilter() {
}

void DUChainUtils::collectItems( DUContext* context, DUChainItemFilter& filter ) {

  QVector<DUContext*> children = context->childContexts();
  QVector<Declaration*> localDeclarations = context->localDeclarations();

  QVector<DUContext*>::const_iterator childIt = children.constBegin();
  QVector<Declaration*>::const_iterator declIt = localDeclarations.constBegin();

  while(childIt != children.constEnd() || declIt != localDeclarations.constEnd()) {

    DUContext* child = nullptr;
    if(childIt != children.constEnd())
      child = *childIt;

    Declaration* decl = nullptr;
    if(declIt != localDeclarations.constEnd())
      decl = *declIt;

    if(decl) {
      if(child && child->range().start.line >= decl->range().start.line)
        child = nullptr;
    }

    if(child) {
      if(decl && decl->range().start >= child->range().start)
        decl = nullptr;
    }

    if(decl) {
      if( filter.accept(decl) ) {
        //Action is done in the filter
      }

      ++declIt;
      continue;
    }

    if(child) {
      if( filter.accept(child) )
        collectItems(child, filter);
      ++childIt;
      continue;
    }
  }
}

KDevelop::DUContext* DUChainUtils::argumentContext(KDevelop::Declaration* decl) {
  DUContext* internal = decl->internalContext();
  if( !internal )
    return nullptr;
  if( internal->type() == DUContext::Function )
    return internal;
  const auto importedParentContexts = internal->importedParentContexts();
  for (const DUContext::Import& ctx : importedParentContexts) {
    if( ctx.context(decl->topContext()) )
      if( ctx.context(decl->topContext())->type() == DUContext::Function )
        return ctx.context(decl->topContext());
  }
  return nullptr;
}

QList<IndexedDeclaration> DUChainUtils::collectAllVersions(Declaration* decl) {
    const auto indexedDecl = IndexedDeclaration(decl);

    QList<IndexedDeclaration> ret;
    ret << indexedDecl;

    if (decl->inSymbolTable()) {
        auto visitor = [&](const IndexedDeclaration& indexed) {
            if (indexed != indexedDecl) {
                ret << indexed;
            }
            return PersistentSymbolTable::VisitorState::Continue;
        };
        PersistentSymbolTable::self().visitDeclarations(decl->qualifiedIdentifier(), visitor);
    }

  return ret;
}

static QList<Declaration*> inheritersInternal(const Declaration* decl, uint& maxAllowedSteps, bool collectVersions)
{
  QList<Declaration*> ret;

  if(!dynamic_cast<const ClassDeclaration*>(decl))
    return ret;

  if(maxAllowedSteps == 0)
    return ret;

  if(decl->internalContext() && decl->internalContext()->type() == DUContext::Class) {
    const auto indexedImporters = decl->internalContext()->indexedImporters();
    for (const IndexedDUContext importer : indexedImporters) {

      DUContext* imp = importer.data();

      if(!imp)
        continue;

      if(imp->type() == DUContext::Class && imp->owner())
        ret << imp->owner();

      --maxAllowedSteps;

      if(maxAllowedSteps == 0)
        return ret;
    }
  }

  if(collectVersions && decl->inSymbolTable()) {
      auto visitor = [&](const IndexedDeclaration& indexedDeclaration) {
          ++maxAllowedSteps;
          auto declaration = indexedDeclaration.data();
          if (declaration && declaration != decl) {
              ret += inheritersInternal(declaration, maxAllowedSteps, false);
          }

          if (maxAllowedSteps == 0) {
              return PersistentSymbolTable::VisitorState::Break;
          } else {
              return PersistentSymbolTable::VisitorState::Continue;
          }
      };
      PersistentSymbolTable::self().visitDeclarations(decl->qualifiedIdentifier(), visitor);
  }

  return ret;
}

QList<Declaration*> DUChainUtils::inheriters(const Declaration* decl, uint& maxAllowedSteps, bool collectVersions)
{
  auto inheriters = inheritersInternal(decl, maxAllowedSteps, collectVersions);

  // remove duplicates
  std::sort(inheriters.begin(), inheriters.end());
  inheriters.erase(std::unique(inheriters.begin(), inheriters.end()), inheriters.end());

  return inheriters;
}

QList<Declaration*> DUChainUtils::overriders(const Declaration* currentClass, const Declaration* overriddenDeclaration, uint& maxAllowedSteps) {
  QList<Declaration*> ret;

  if(maxAllowedSteps == 0)
    return ret;

  if(currentClass != overriddenDeclaration->context()->owner() && currentClass->internalContext())
    ret += currentClass->internalContext()->findLocalDeclarations(overriddenDeclaration->identifier(), CursorInRevision::invalid(), currentClass->topContext(), overriddenDeclaration->abstractType());

  const auto inheriters = DUChainUtils::inheriters(currentClass, maxAllowedSteps);
  for (Declaration* inheriter : inheriters) {
    ret += overriders(inheriter, overriddenDeclaration, maxAllowedSteps);
  }

  return ret;
}

static bool hasUse(DUContext* context, int usedDeclarationIndex) {
  if(usedDeclarationIndex == std::numeric_limits<int>::max())
    return false;

  for(int a = 0; a < context->usesCount(); ++a)
    if(context->uses()[a].m_declarationIndex == usedDeclarationIndex)
      return true;

  const auto childContexts = context->childContexts();
  return std::any_of(childContexts.begin(), childContexts.end(), [&](DUContext* child) {
    return hasUse(child, usedDeclarationIndex);
  });
}

bool DUChainUtils::contextHasUse(DUContext* context, Declaration* declaration) {
  return hasUse(context, context->topContext()->indexForUsedDeclaration(declaration, false));
}

static uint countUses(DUContext* context, int usedDeclarationIndex) {
  if(usedDeclarationIndex == std::numeric_limits<int>::max())
    return 0;

  uint ret = 0;

  for(int a = 0; a < context->usesCount(); ++a)
    if(context->uses()[a].m_declarationIndex == usedDeclarationIndex)
      ++ret;

  const auto childContexts = context->childContexts();
  for (DUContext* child : childContexts) {
    ret += countUses(child, usedDeclarationIndex);
  }

  return ret;
}

uint DUChainUtils::contextCountUses(DUContext* context, Declaration* declaration) {
  return countUses(context, context->topContext()->indexForUsedDeclaration(declaration, false));
}

Declaration* DUChainUtils::overridden(const Declaration* decl) {
  const auto* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(decl);
  if(!classFunDecl || !classFunDecl->isVirtual())
    return nullptr;

  QList<Declaration*> decls;

  const auto importedParentContexts = decl->context()->importedParentContexts();
  for (const DUContext::Import &import : importedParentContexts) {
    DUContext* ctx = import.context(decl->topContext());
    if(ctx)
      decls += ctx->findDeclarations(QualifiedIdentifier(decl->identifier()),
                                            CursorInRevision::invalid(), decl->abstractType(), decl->topContext(), DUContext::DontSearchInParent);
  }

  auto it = std::find_if(decls.constBegin(), decls.constEnd(), [&](Declaration* found) {
    const auto* foundClassFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(found);
    return (foundClassFunDecl && foundClassFunDecl->isVirtual());
  });

  return (it != decls.constEnd()) ? *it : nullptr;
}

DUContext* DUChainUtils::functionContext(Declaration* decl) {
  DUContext* functionContext = decl->internalContext();
  if(functionContext && functionContext->type() != DUContext::Function) {
    const auto importedParentContexts = functionContext->importedParentContexts();
    for (const DUContext::Import& import : importedParentContexts) {
      DUContext* ctx = import.context(decl->topContext());
      if(ctx && ctx->type() == DUContext::Function)
        functionContext = ctx;
    }
  }

  if(functionContext && functionContext->type() == DUContext::Function)
    return functionContext;
  return nullptr;
}

QVector<KDevelop::Problem::Ptr> KDevelop::DUChainUtils::allProblemsForContext(const KDevelop::ReferencedTopDUContext& top)
{
    QVector<KDevelop::Problem::Ptr> ret;

    const auto problems = top->problems();
    const auto contextProblems = ICore::self()->languageController()->staticAssistantsManager()->problemsForContext(top);
    ret.reserve(problems.size() + contextProblems.size());

    for (const auto& p : problems) {
        ret << p;
    }
    for (const auto& p : contextProblems) {
        ret << p;
    }

    return ret;
}

