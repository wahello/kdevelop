/***************************************************************************
                          kdlgpropwidget.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KDLGPROPWIDGET_H
#define KDLGPROPWIDGET_H

#include <qwidget.h>

class AdvListView;
class KDlgItem_Widget;

/**
  *@author Pascal Krahmer
  */

class KDlgPropWidget : public QWidget  {
   Q_OBJECT
public:
	KDlgPropWidget(QWidget *parent=0, const char *name=0);
	~KDlgPropWidget();

protected:
        virtual void resizeEvent ( QResizeEvent * );

private:
        AdvListView *lv;
};

#endif
