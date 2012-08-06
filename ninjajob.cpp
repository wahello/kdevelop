/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ninjajob.h"
#include <KProcess>
#include <KUrl>
#include <KDebug>
#include <outputview/outputmodel.h>
#include <util/commandexecutor.h>

NinjaJob::NinjaJob(const KUrl& dir, const QStringList& arguments, QObject* parent)
    : OutputJob(parent, Verbose)
{
    Q_ASSERT(!dir.isRelative());
    setToolTitle("Ninja");
    setCapabilities(Killable);
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
 
    m_process = new KDevelop::CommandExecutor("ninja", this);
    m_process->setArguments( arguments );
    m_process->setWorkingDirectory(dir.toLocalFile(KUrl::RemoveTrailingSlash));
    
    m_model = new KDevelop::OutputModel(this);
    setModel( m_model, KDevelop::IOutputView::TakeOwnership );
    m_model->setFilteringStrategy(KDevelop::OutputModel::CompilerFilter);

    connect(m_process, SIGNAL(receivedStandardError(QStringList)),
            model(), SLOT(appendLines(QStringList)) );
    connect(m_process, SIGNAL(receivedStandardOutput(QStringList)),
            model(), SLOT(appendLines(QStringList)) );
    
    connect( m_process, SIGNAL(failed(QProcess::ProcessError)), this, SLOT(slotFailed(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(completed()), this, SLOT(slotCompleted()) );
}

void NinjaJob::start()
{
    startOutput();
    kDebug() << "Executing ninja" << m_process->arguments();
    m_model->appendLine( m_process->workingDirectory() + "> " + m_process->command() + " " + m_process->arguments().join(" ") );
    m_process->start();
}

bool NinjaJob::doKill()
{
    m_process->kill();
    return true;
}
