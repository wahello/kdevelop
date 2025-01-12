/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "overridespage.h"
#include "ui_overridevirtuals.h"
#include "debug.h"

#include <language/duchain/duchain.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/alltypes.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

using namespace KDevelop;

enum Column {
    ClassOrFunctionColumn, ///< Column represents either a base class item or a function item
    AccessColumn, ///< Column represents the access policy of a function
    PropertiesColumn ///< Column represents the properties of a function (e.g. if it's a ctor, dtor, signal, slot, ...)
};

static QString accessPolicyToString(Declaration::AccessPolicy accessPolicy)
{
    switch (accessPolicy) {
    case Declaration::DefaultAccess:
    case Declaration::Public:
        return i18nc("@item access policy", "Public");
    case Declaration::Protected:
        return i18nc("@item access policy", "Protected");
    case Declaration::Private:
        return i18nc("@item access policy", "Private");
    }
    Q_UNREACHABLE();
}

static QString functionPropertiesToString(ClassFunctionDeclaration* decl)
{
    Q_ASSERT(decl);
    QStringList properties;
    if (decl->isConstructor()) {
        properties << i18nc("@item function property", "Constructor");
    } else if (decl->isDestructor()) {
        properties << i18nc("@item function property", "Destructor");
    } else if (decl->isSignal()) {
        properties << i18nc("@item function property", "Signal");
    } else if (decl->isSlot()) {
        properties << i18nc("@item function property", "Slot");
    } else if (decl->isAbstract()) {
        properties << i18nc("@item function property", "Abstract function");
    }
    return properties.join(QLatin1String(", "));
}

struct KDevelop::OverridesPagePrivate
{
    OverridesPagePrivate()
        : overrides(nullptr)
    {
    }
    Ui::OverridesDialog* overrides;
    QMultiHash<Identifier, DeclarationPointer> overriddenFunctions;
    QMap<QTreeWidgetItem *, DeclarationPointer> declarationMap;
    QList<DeclarationPointer> chosenOverrides;
};

OverridesPage::OverridesPage(QWidget* parent)
    : QWidget(parent)
    , d(new OverridesPagePrivate)
{
    d->overrides = new Ui::OverridesDialog;
    d->overrides->setupUi(this);

    connect(d->overrides->selectAllPushButton, &QPushButton::pressed, this, &OverridesPage::selectAll);
    connect(d->overrides->deselectAllPushButton, &QPushButton::pressed, this, &OverridesPage::deselectAll);
}

OverridesPage::~OverridesPage()
{
    delete d->overrides;
    delete d;
}

QList< DeclarationPointer > OverridesPage::selectedOverrides() const
{
    QList<DeclarationPointer> declarations;

    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j)
        {
            QTreeWidgetItem* child = item->child(j);
            if (child->checkState(ClassOrFunctionColumn) == Qt::Checked)
            {
                qCDebug(PLUGIN_FILETEMPLATES) << "Adding declaration" << d->declarationMap[child]->toString();
                declarations << d->declarationMap[child];
            }
        }
    }

    qCDebug(PLUGIN_FILETEMPLATES) << declarations.size();
    return declarations;
}

void OverridesPage::clear()
{
    d->overriddenFunctions.clear();
    overrideTree()->clear();
    d->chosenOverrides.clear();
    d->declarationMap.clear();
}

void OverridesPage::addBaseClasses(const QList<DeclarationPointer>& directBases,
                                   const QList<DeclarationPointer>& allBases)
{
    DUChainReadLocker lock;

    for (const DeclarationPointer& baseClass : allBases) {
        auto* classItem = new QTreeWidgetItem(overrideTree(), QStringList() << baseClass->qualifiedIdentifier().toString());
        classItem->setIcon(ClassOrFunctionColumn, DUChainUtils::iconForDeclaration(baseClass.data()));

        DUContext* context = baseClass->internalContext();
        if (!context) {
            continue;
        }

        //For this internal context get all the function declarations inside the class
        const auto localDeclarations = context->localDeclarations();
        for (Declaration* childDeclaration : localDeclarations) {
            if (auto * func = dynamic_cast<AbstractFunctionDeclaration*>(childDeclaration))
            {
                if (func->isVirtual())
                {
                    // Its a virtual function, add it to the list unless it's a destructor
                    auto* cFunc = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration);
                    if (cFunc && !cFunc->isDestructor())
                    {
                        addPotentialOverride(classItem, DeclarationPointer(childDeclaration));
                    }
                }
                else if (directBases.contains(baseClass))
                {
                    // add ctors of direct parents
                    auto* cFunc = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration);
                    if (cFunc && cFunc->isConstructor())
                    {
                        addPotentialOverride(classItem, DeclarationPointer(childDeclaration));
                    }
                }
            }
        }
    }

    overrideTree()->expandAll();
    overrideTree()->header()->resizeSections(QHeaderView::ResizeToContents);
}

void OverridesPage::addPotentialOverride(QTreeWidgetItem* classItem, const DeclarationPointer& childDeclaration)
{
    auto* function = dynamic_cast<ClassFunctionDeclaration*>(childDeclaration.data());
    if (!function) {
        qCDebug(PLUGIN_FILETEMPLATES) << "Declaration is not a function:" << childDeclaration->identifier().toString();
        return;
    }

    if (function->accessPolicy() == Declaration::Private) {
        qCDebug(PLUGIN_FILETEMPLATES) << "Declaration is private, returning:" << function->identifier().toString();
        return;
    }

    qCDebug(PLUGIN_FILETEMPLATES) << childDeclaration->toString();
    auto overridenFunctionIt = d->overriddenFunctions.constFind(childDeclaration->identifier());
    if (overridenFunctionIt != d->overriddenFunctions.constEnd()) {
        for (; overridenFunctionIt != d->overriddenFunctions.constEnd(); ++overridenFunctionIt) {
            const DeclarationPointer& decl = *overridenFunctionIt;
            if (decl->indexedType() == childDeclaration->indexedType())
            {
                qCDebug(PLUGIN_FILETEMPLATES) << "Declaration is already shown";
                return;
            }
        }
    }

    d->overriddenFunctions.insert(childDeclaration->identifier(), childDeclaration);

    auto* overrideItem = new QTreeWidgetItem(classItem, QStringList() << childDeclaration->toString());
    overrideItem->setFlags( Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable) );
    overrideItem->setCheckState(ClassOrFunctionColumn, d->chosenOverrides.contains(childDeclaration) ? Qt::Checked : Qt::Unchecked);
    overrideItem->setIcon(ClassOrFunctionColumn, DUChainUtils::iconForDeclaration(childDeclaration.data()));
    overrideItem->setData(ClassOrFunctionColumn, Qt::UserRole, QVariant::fromValue(IndexedDeclaration(childDeclaration.data())));
    overrideItem->setText(AccessColumn, accessPolicyToString(function->accessPolicy()));
    overrideItem->setText(PropertiesColumn, functionPropertiesToString(function));

    if (function->isAbstract()) {
        overrideItem->setIcon(ClassOrFunctionColumn, QIcon::fromTheme(QStringLiteral("flag-red")));
        overrideItem->setCheckState(ClassOrFunctionColumn, Qt::Checked);
        classItem->removeChild(overrideItem);
        classItem->insertChild(0, overrideItem);
    }

    d->declarationMap[overrideItem] = childDeclaration;
}

QTreeWidget* OverridesPage::overrideTree() const
{
    return d->overrides->overridesTree;
}

QWidget* OverridesPage::extraFunctionsContainer() const
{
    return d->overrides->extraFunctionWidget;
}

void OverridesPage::selectAll()
{
    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j)
            item->child(j)->setCheckState(ClassOrFunctionColumn, Qt::Checked);
    }
}

void OverridesPage::deselectAll()
{
    for (int i = 0; i < d->overrides->overridesTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = d->overrides->overridesTree->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j)
            item->child(j)->setCheckState(ClassOrFunctionColumn, Qt::Unchecked);
    }
}

void OverridesPage::addCustomDeclarations (const QString& category, const QList<DeclarationPointer>& declarations)
{
    qCDebug(PLUGIN_FILETEMPLATES) << category << declarations.size();
    DUChainReadLocker lock(DUChain::lock());

    auto* item = new QTreeWidgetItem(overrideTree(), QStringList() << category);
    for (const DeclarationPointer& declaration : declarations)
    {
        addPotentialOverride(item, declaration);
    }

    overrideTree()->expandAll();
    overrideTree()->header()->resizeSections(QHeaderView::ResizeToContents);
}

void OverridesPage::setFocusToFirstEditWidget()
{
    d->overrides->overridesTree->setFocus();
}
