/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVBACKGROUNDPARSER_H
#define KDEVBACKGROUNDPARSER_H

#include <QObject>

#include <kurl.h>
#include <QMap>
#include <QPair>
#include <QMutex>

namespace ThreadWeaver
{
class Weaver;
class Job;
}

namespace KTextEditor
{
class Document;
class SmartRange;
}

class QTimer;

class KDevAST;
class KDevDocument;
class KDevCodeModel;
class KDevLanguageSupport;
class KDevPersistentHash;

typedef QList< QPair<KUrl, KDevCodeModel* > > CodeModelCache;

using namespace ThreadWeaver;

class KDevBackgroundParser : public QObject
{
    Q_OBJECT
public:
    KDevBackgroundParser( QObject* parent = 0 );
    virtual ~KDevBackgroundParser();

public slots:
    void suspend();
    void resume();
    void addDocument( const KUrl &url, KDevDocument *document = 0L );
    void addDocumentList( const KUrl::List &urls );
    void removeDocumentFile( KDevDocument *document );
    void removeDocument( const KUrl &url );
    void parseDocuments();
    void parseComplete( Job *job );
    void documentChanged( KTextEditor::Document *document );
    void cacheModels( uint modelsToCache );
    KTextEditor::SmartRange *buildHighlight( KDevDocument *document );

private:
    QTimer *m_timer;
    bool m_suspend;
    uint m_modelsToCache;

    // A list of known documents, and whether they are due to be parsed or not
    QMap<KUrl, bool> m_documents;
    // A list of open documents
    QMap<KUrl, KDevDocument*> m_openDocuments;
    // A list of cached models when parsing a large amount of files.
    CodeModelCache m_modelCache;

    // The persistent AST storage
    KDevPersistentHash *m_peristentHash;

    mutable QMutex m_mutex;
    ThreadWeaver::Weaver* m_weaver;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
