/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include "kdevlanguagesupport.h"
#include "classstore.h"
#include "classtooldlg.h"
#include "classtreewidget.h"


ClassTreeWidget::ClassTreeWidget(ClassView *part)
    : ClassTreeBase(part, 0, "class tree widget")
{
    connect( part, SIGNAL(setLanguageSupport(KDevLanguageSupport*)),
             this, SLOT(setLanguageSupport(KDevLanguageSupport*)) );
    connect( part, SIGNAL(setClassStore(ClassStore*)),
             this, SLOT(setClassStore(ClassStore*)) );
}


ClassTreeWidget::~ClassTreeWidget()
{}


KPopupMenu *ClassTreeWidget::createPopup()
{
    KPopupMenu *popup = contextItem->createPopup();
    if (!popup) {
        popup = new KPopupMenu();
        popup->insertTitle(i18n("Class view"), -1, 0);
    }

    popup->setCheckable(true);
    int id1 = popup->insertItem( i18n("List by namespaces"), this, SLOT(slotTreeModeChanged()) );
    int id2 = popup->insertItem( i18n("Full identifier scopes"), this, SLOT(slotScopeModeChanged()) );
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    bool byNamespace = config->readBoolEntry("ListByNamespace");
    popup->setItemChecked(id1, byNamespace);
    bool identifierScopes = config->readBoolEntry("FullIdentifierScopes");
    popup->setItemChecked(id2, identifierScopes);

    return popup;
}


void ClassTreeWidget::setLanguageSupport(KDevLanguageSupport *ls)
{
    ClassTreeBase::setLanguageSupport(ls);
    connect(ls, SIGNAL(updateSourceInfo()), this, SLOT(refresh()));
}


void ClassTreeWidget::setClassStore(ClassStore *store)
{
    ClassTreeBase::setClassStore(store);
    refresh();
}


void ClassTreeWidget::slotTreeModeChanged()
{
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    config->writeEntry("ListByNamespace", !config->readBoolEntry("ListByNamespace"));
    buildTree(true);
}


void ClassTreeWidget::slotScopeModeChanged()
{
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    config->writeEntry("FullIdentifierScopes", !config->readBoolEntry("FullIdentifierScopes"));
    buildTree(false);
}


void ClassTreeWidget::refresh()
{
    buildTree(false);
}


void ClassTreeWidget::buildTree(bool fromScratch)
{
    if (!m_store)
        return;
    
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    if (config->readBoolEntry("ListByNamespace", false))
        buildTreeByNamespace(fromScratch);
    else
        buildTreeByCategory(fromScratch);
}


void ClassTreeWidget::buildTreeByCategory(bool fromScratch)
{
    // TODO: Some smart open/closed state restoring if !fromScratch
    clear();
    
    ParsedScopeContainer *scope = &m_store->globalContainer;
    
    ClassTreeItem *ilastItem, *lastItem = 0;

    // Add classes
    // TODO: Optionally, separate classes in separate directories
    lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Classes"));
    ilastItem = 0;
    QList<ParsedClass> *classList = scope->getSortedClassList();
    for ( ParsedClass *pClass = classList->first();
          pClass != 0;
          pClass = classList->next() ) {
        ilastItem = new ClassTreeClassItem(lastItem, ilastItem, pClass);
    }
    delete classList;
    if (fromScratch)
        lastItem->setOpen(true);
    
    // Add structs
    lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Structs"));
    ilastItem = 0;
    QList<ParsedStruct> *structList = scope->getSortedStructList();
    for ( ParsedStruct *pStruct = structList->first();
          pStruct != 0;
          pStruct = structList->next() ) {
        ilastItem = new ClassTreeStructItem(lastItem, ilastItem, pStruct);
    }
    delete structList;
    if (fromScratch)
        lastItem->setOpen(true);
                                 
    // Add functions
    lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Functions"));
    ilastItem = 0;
    QList<ParsedMethod> *methodList = scope->getSortedMethodList();
    for ( ParsedMethod *pMethod = methodList->first();
          pMethod != 0;
          pMethod = methodList->next() ) {
        ilastItem = new ClassTreeMethodItem(lastItem, ilastItem, pMethod);
    }
    delete methodList;
    if (fromScratch)
        lastItem->setOpen(true);

    // Add attributes
    lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Variables"));
    ilastItem = 0;
    QList<ParsedAttribute> *attrList = scope->getSortedAttributeList();
    for ( ParsedAttribute *pAttr = attrList->first();
          pAttr != 0;
          pAttr = attrList->next() ) {
        ilastItem = new ClassTreeAttrItem(lastItem, ilastItem, pAttr);
    }
    delete attrList;
    if (fromScratch)
        lastItem->setOpen(true);

    // Add namespaces
    lastItem = new ClassTreeOrganizerItem(this, lastItem, i18n("Namespaces"));
    ilastItem = 0;
    QList<ParsedScopeContainer> *scopeList = scope->getSortedScopeList();
    for ( ParsedScopeContainer *pScope = scopeList->first();
          pScope != 0;
          pScope = scopeList->next() ) {
        ilastItem = new ClassTreeScopeItem(lastItem, ilastItem, pScope);
    }
    delete scopeList;
    if (fromScratch)
        lastItem->setOpen(true);
}


void ClassTreeWidget::buildTreeByNamespace(bool fromScratch)
{
    // TODO: Some smart open/closed state restoring if !fromScratch
    clear();

    ClassTreeItem *lastItem = 0;
    
    // Global namespace?
    
    QList<ParsedScopeContainer> *scopeList = m_store->globalContainer.getSortedScopeList();
    for (ParsedScopeContainer *pScope = scopeList->first(); pScope != 0; pScope = scopeList->next()) {
        lastItem = new ClassTreeScopeItem(this, lastItem, pScope);
        if (fromScratch)
            lastItem->setOpen(true);
    }
    delete scopeList;
}


