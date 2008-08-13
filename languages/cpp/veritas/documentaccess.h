/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#ifndef VERITASCPP_DOCUMENT_ACCESS
#define VERITASCPP_DOCUMENT_ACCESS

#include <KUrl>
#include <QtCore/QObject>
#include <language/editor/simplerange.h>
#include "veritascppexport.h"

namespace Veritas
{

/*! Extra layer of indirection to get around ICore, IDocumentController, IDocument and KT::Document 
    dependencies in tests. (and put the shiny new stub generator to good use :> ) */
class VERITASCPP_EXPORT DocumentAccess : public QObject
{
Q_OBJECT
public:
    DocumentAccess(QObject* parent=0);
    virtual ~DocumentAccess();
    /*! Fetch the text contained in a @param range for a @param url */
    virtual QString text(const KUrl& url, const KDevelop::SimpleRange& range) const;
    virtual QString text(const KUrl&) const;
};

}

#endif // VERITASCPP_DOCUMENT_ACCESS
