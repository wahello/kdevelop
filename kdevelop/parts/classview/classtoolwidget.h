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

#ifndef _CLASSTOOLWIDGET_H_
#define _CLASSTOOLWIDGET_H_

#include "classtreebase.h"


class ClassToolWidget : public ClassTreeBase
{
    Q_OBJECT
    
public:
    ClassToolWidget(ClassView *part, QWidget *parent=0);
    ~ClassToolWidget();

    void insertClassAndClasses(ParsedClass *parsedClass, QList<ParsedClass> *classList);
    void insertClassAndClasses(ParsedClass *parsedClass, QList<ParsedParent> *parentList);
    void insertAllClassMethods(ParsedClass *parsedClass, PIExport filter);
    void insertAllClassAttributes(ParsedClass *parsedClass, PIExport filter);

protected:
    virtual KPopupMenu *createPopup();
  
private:
    void addClassAndAttributes(ParsedClass *parsedClass, PIExport filter, ClassTreeItem **lastItem);
    void addClassAndMethods(ParsedClass *parsedClass, PIExport filter, ClassTreeItem **lastItem);
};

#endif
