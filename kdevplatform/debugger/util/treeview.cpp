/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "treeview.h"

#include "treemodel.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

using namespace KDevelop;

AsyncTreeView::AsyncTreeView(TreeModel& treeModel, QWidget* parent)
    : QTreeView(parent)
    , m_treeModel(treeModel)
{
    connect (this, &AsyncTreeView::expanded,
             this, &AsyncTreeView::slotExpanded);
    connect (this, &AsyncTreeView::collapsed,
             this, &AsyncTreeView::slotCollapsed);
    connect (this, &AsyncTreeView::clicked,
             this, &AsyncTreeView::slotClicked);
    connect(&m_treeModel, &TreeModel::itemChildrenReady, this, &AsyncTreeView::slotExpandedDataReady);
}


void AsyncTreeView::slotExpanded(const QModelIndex &index)
{
    m_treeModel.expanded(mapViewIndexToTreeModelIndex(index));
}

void AsyncTreeView::slotCollapsed(const QModelIndex &index)
{
    m_treeModel.collapsed(mapViewIndexToTreeModelIndex(index));
    resizeColumns();
}

void AsyncTreeView::slotClicked(const QModelIndex &index)
{
    m_treeModel.clicked(mapViewIndexToTreeModelIndex(index));
    resizeColumns();
}

QSize AsyncTreeView::sizeHint() const
{
    //Assuming that columns are always resized to fit their contents, return a size that will fit all without a scrollbar
    QMargins margins = contentsMargins();
    int horizontalSize = margins.left() + margins.right();
    for (int i = 0; i < model()->columnCount(); ++i) {
        horizontalSize += columnWidth(i);
    }
    horizontalSize = qMin(horizontalSize, QGuiApplication::primaryScreen()->geometry().width()*3/4);
    return QSize(horizontalSize, margins.top() + margins.bottom() + sizeHintForRow(0));
}

void AsyncTreeView::resizeColumns()
{
    for (int i = 0; i < model()->columnCount(); ++i) {
        this->resizeColumnToContents(i);
    }
    this->updateGeometry();
}

TreeModel& AsyncTreeView::treeModel()
{
    return m_treeModel;
}

void AsyncTreeView::slotExpandedDataReady()
{
    resizeColumns();
}

QModelIndex AsyncTreeView::mapViewIndexToTreeModelIndex(const QModelIndex& viewIndex) const
{
    return viewIndex;
}
