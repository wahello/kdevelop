/***************************************************************************
                          kdlgpropwidget.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by                          
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "kdlgpropwidget.h"
#include "kdlgproplvis.h"
#include <kcolorbtn.h>

AdvListView::AdvListView( QWidget * parent , const char * name )
  : QListView( parent, name )
{
}

void AdvListView::viewportMousePressEvent ( QMouseEvent *e )
{
  QListView::viewportMousePressEvent( e );
  updateWidgets();
}

void AdvListView::paintEvent ( QPaintEvent *e )
{
  QListView::paintEvent( e );
  updateWidgets();
}

void AdvListView::mousePressEvent ( QMouseEvent *e )
{
  QListView::mousePressEvent( e );
  updateWidgets();
}

void AdvListView::mouseMoveEvent ( QMouseEvent *e )
{
  QListView::mousePressEvent( e );
  updateWidgets();
}

void AdvListView::keyPressEvent ( QKeyEvent *e )
{
  QListView::keyPressEvent( e );
  updateWidgets();
}

void AdvListView::moveEvent ( QMoveEvent *e )
{
  QListView::moveEvent( e );
  updateWidgets();
}

void AdvListView::resizeEvent ( QResizeEvent *e )
{
  QListView::resizeEvent( e );
  updateWidgets();
}

void AdvListView::updateWidgets()
{
  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    i->updateWidgets();
    i = (AdvListViewItem*)i->nextSibling();
  }
}

/**
 *
*/
AdvListViewItem::AdvListViewItem( QListView * parent, QString a, QString b)
   : QListViewItem( parent, a, b )
{
  init();
//  parent->connect( parent, SIGNAL(scrollBy(int, int)), SLOT(updateWidgets()));
}

/**
 *
*/
AdvListViewItem::AdvListViewItem( AdvListViewItem * parent, QString a, QString b )
   : QListViewItem( parent, a, b )
{
  init();
}

/**
 *
*/
void AdvListViewItem::init()
{
  clearAllColumnWidgets();
}
		
/**
 *
*/
void AdvListViewItem::setColumnWidget( int col, AdvLvi_Base *wp, bool activated )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  colwid[ col ] = wp;
  colactivated[ col ] = activated;
}

/**
 *
*/
void AdvListViewItem::clearColumnWidget( int col, bool deleteit )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  if ( !getColumnWidget( col ) )
    return;

  if ( deleteit )
    delete getColumnWidget( col );

  setColumnWidget( col, 0 );
}

/**
 *
*/
void AdvListViewItem::clearAllColumnWidgets( bool deletethem )
{
  for (int i=0; i<=MAX_WIDGETCOLS_PER_LINE; i++)
    clearColumnWidget(i, deletethem);
}

/**
 *
*/
QWidget* AdvListViewItem::getColumnWidget( int col )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return 0;

  return colwid[ col ];
}

/**
 *
*/
void AdvListViewItem::activateColumnWidget( int col, bool activate )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  colactivated[ col ] = activate;
}

/**
 *
*/
bool AdvListViewItem::columnWidgetActive( int col )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return false;

  return colactivated[ col ];
}


void AdvListViewItem::testAndResizeAllWidgets()
{
  for (int i = 0; i < MAX_WIDGETCOLS_PER_LINE; i++)
    if (colwid[i])
      testAndResizeWidget( i );
}



void AdvListViewItem::updateWidgets()
{
  testAndResizeAllWidgets();

  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    i->updateWidgets();
    i = (AdvListViewItem*)i->nextSibling();
  } ;
}					



void AdvListViewItem::testAndResizeWidget(int column)
{
  // if the given column is in  valid range for widgets and
  //   we have  been given  a valid pointer  on a widget we
  //   continue the widget handling.
  if ((column >= 0) && (column <= MAX_WIDGETCOLS_PER_LINE) && (colwid[ column ]))
    {

#if defined(LVDEBUG)
    debug("testAndResizeWidget %s, (selected=%d)", text(column), isSelected());
#endif

      // if the line the class  builds is selected and if the
      //   widget  for this column  is active we  can show it
      //   otherwise we have to hide and take  the focus away
      //   from it.
      if (isSelected() && (columnWidgetActive( column )) && ((!parent()) || (parent()->isOpen())))
        {
          colwid[column]->show();
          listView()->ensureItemVisible(this);
        }
      else
        {
          colwid[column]->hide();

          if (colwid[column]->hasFocus())
            colwid[column]->clearFocus();

          if (colwid[column])
            setText(column, colwid[column]->getText());
        }

    }

}






/**
 * Overloaded QListViewItem::paintCell() method. This method tests
 * if we  have a widget  prepared for this  cell and if we have it
 * shows and moves or hides it.
*/		
void AdvListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, int align )
{
  if ( !p )
    return;

  QListView *lv = listView();

  if (!lv)
    return;


  // if the given column is in  valid range for widgets and
  //   we have  been given  a valid pointer  on a widget we
  //   continue the widget handling. Otherwise we just call
  //   the parents paintCell() method (see below).
  if ((column >= 0) && (column <= MAX_WIDGETCOLS_PER_LINE) && (colwid[ column ]))
    {

#if defined(LVDEBUG)
    debug("paintCell %s, (selected=%d)", text(column), isSelected());
#endif

      // if the line the class  builds is selected and if the
      //   widget for this column is active we can move it to
      //   the right place  and show it  otherwise we have to
      //   hide and take the focus away from it.

      if ((lv->isSelected(this)) && (columnWidgetActive( column )))
        {
          QRect r(lv->itemRect( this ));

          // test if the column is _completely_ visible by testing
          //   wheter it is  not starting at y 0 or,  if it is the
          //   first item, whether it is not starting at y under 0
          if ( (r.y()>0) || ((lv->firstChild() == this) && (r.y()>=0) ) )
            {
              int x = 0;
              int dummy = column;
              while (dummy--)
                x += lv->columnWidth(dummy);

              x = (int)p->worldMatrix().dx();

              // we have to test if the colums goes out of the
              //   visible (x or width) area and if it does we
              //   have to resize it in order to avoid that it
              //   overpaintes the vertical scrollbar.
              int wd = width;
              int vs = 0;
              if (lv->verticalScrollBar()->isVisible())
                vs = lv->verticalScrollBar()->width();

              if (x+width > lv->width() - vs)
                wd = lv->width() - vs - x;

              colwid[column]->setGeometry( x,
                                    r.y() + lv->viewport()->y(),
                                    wd, height());

              colwid[column]->show();
            }
          else
            {
              colwid[column]->hide();

              if (colwid[column])
                setText(column, colwid[column]->getText());
            }


          // if more than one widget exists and one of these do have
          //   the focus we do not  need to set it to this (makes it
          //   possible to jump between  several widgets in one line
          //   using the tabulator [tab] key)
          bool flag = true;
          for (int i=0; i < MAX_WIDGETCOLS_PER_LINE; i++)
            if (colwid[i])
              if (colwid[i]->hasFocus())
                flag = false;

          // if we do not have found a widget with the focus in this
          //   line (see above) we must  set the focus to the one in
          //   this line and column.
          if (flag)
              colwid[column]->setFocus();
        }
      else
        {
          // since this  widget has been  disactivated or the line this
          //   instance of the class represents is not selected we have
          //   to hide the widget and take the focus away from it
          colwid[column]->hide();

          if (colwid[column]->hasFocus())
            colwid[column]->clearFocus();
        }
    }


  QListViewItem::paintCell(p,cg,column, width,align);
}

KDlgPropWidget::KDlgPropWidget(QWidget *parent, const char *name ) : QWidget(parent,name)
{
  lv = new AdvListView(this);
  lv->addColumn("Property");
  lv->addColumn("Value");
  lv->show();

  lv->setRootIsDecorated(true);

  AdvListViewItem *lvi1 = new AdvListViewItem(lv,"Prop 1","Val 1");
  lvi1->setColumnWidget(1, new AdvLvi_Filename( lv ));
  AdvListViewItem *lvi2 = new AdvListViewItem(lv,"Prop 2","Val 2");
  lvi2->setColumnWidget(1, new AdvLvi_ColorEdit( lv ));
  AdvListViewItem *lvi3 = new AdvListViewItem(lvi2,"Prop 3","Val 3");
  lvi3->setColumnWidget(1, new AdvLvi_Filename( lv ));
  AdvListViewItem *lvi4 = new AdvListViewItem(lvi2,"Prop 4","Val 4");
  lvi4->setColumnWidget(1, new AdvLvi_Filename( lv ));
  AdvListViewItem *lvi5 = new AdvListViewItem(lvi2,"Prop 5","Val 5");
  lvi5->setColumnWidget(1, new AdvLvi_Filename( lv ));
  AdvListViewItem *lvi6 = new AdvListViewItem(lvi2,"Prop 6","Val 6");
  lvi6->setColumnWidget(1, new AdvLvi_Filename( lv ));
  AdvListViewItem *lvi7 = new AdvListViewItem(lv,"Prop 7","Val 7");
  lvi7->setColumnWidget(1, new AdvLvi_Bool( lv ));
}

KDlgPropWidget::~KDlgPropWidget()
{
}

void KDlgPropWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent( e );

  lv->setGeometry(0,0,width(),height());
}










AdvLvi_Base::AdvLvi_Base(QWidget *parent, const char *name)
  : QWidget( parent, name )
{
  setBackgroundColor( colorGroup().base() );
//  setFocusPolicy(NoFocus);
  setEnabled(false);
}

void AdvLvi_Base::paintEvent ( QPaintEvent * e )
{
  setBackgroundColor( colorGroup().base() );
  QWidget::paintEvent( e );
}



AdvLvi_ExtEdit::AdvLvi_ExtEdit(QWidget *parent, const char *name )
  : AdvLvi_Base( parent, name )
{
  btnMore = new QPushButton("...",this);
  leInput = new QLineEdit( this );
//  connect( leInput, SIGNAL( textChanged ( const char * ) ), SLOT( updateParentLvi() ) );
}


void AdvLvi_ExtEdit::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );
  if (btnMore)
    btnMore->setGeometry(width()-15,0,15,height());

  if (leInput)
    leInput->setGeometry(0,0,width()-15,height()+1);
}



AdvLvi_Filename::AdvLvi_Filename(QWidget *parent, const char *name)
  : AdvLvi_ExtEdit( parent, name )
{
  connect( btnMore, SIGNAL( clicked() ), this, SLOT( btnPressed() ) );
}

void AdvLvi_Filename::btnPressed()
{
  QString fname = KFileDialog::getOpenFileName();
  if (!fname.isNull())
    {
      leInput->setText(fname);
    }
}


AdvLvi_Bool::AdvLvi_Bool(QWidget *parent, const char *name)
  : AdvLvi_Base( parent, name )
{
  cbBool = new QComboBox( FALSE, this );
  cbBool->insertItem("TRUE");
  cbBool->insertItem("FALSE");
}

void AdvLvi_Bool::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbBool)
    cbBool->setGeometry(0,0,width(),height()+1);
}



AdvLvi_ColorEdit::AdvLvi_ColorEdit(QWidget *parent, const char *name)
  : AdvLvi_Base( parent, name )
{
  btn = new KColorButton(this);

//  connect( btn, SIGNAL( clicked() ), this, SLOT( btnPressed() ) );
}

void AdvLvi_ColorEdit::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (btn)
    btn->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_ColorEdit::getText()
{
  if (!btn) return QString();
  char s[255];
  sprintf(s,"0x%.2x%.2x%.2x",btn->color().red(),btn->color().green(),btn->color().blue());
  return QString(s);
}

/*
QString AdvLvi_intToHex(int i)
{
  char s[10];
  sprintf(s,"%.2x",i);
  return QString(s);
}

void AdvLvi_ColorEdit::btnPressed()
{
  int l=0;
  sscanf((const char*)getText(), "%i", &l);

  QColor myColor(QColor((unsigned char)(l>>16),(unsigned char)(l>>8),(unsigned char)(l)));
  int res = KColorDialog::getColor( myColor );
  if (res)
    {
      QString st = "0x"+AdvLvi_intToHex(myColor.red())+AdvLvi_intToHex(myColor.green())+AdvLvi_intToHex(myColor.blue());
      leInput->setText(st);
    }
}     */

