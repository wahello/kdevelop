/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef ITEMREPOSITORYREGISTRY_H
#define ITEMREPOSITORYREGISTRY_H

#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QAtomicInt>
#include <QtCore/QMap>

#include <language/languageexport.h>

namespace KDevelop {

class ISession;
class AbstractRepositoryManager;
class AbstractItemRepository;

/**
 * Manages a set of item-repositores and allows loading/storing them all at once from/to disk.
 * Does not automatically store contained repositories on destruction.
 * For the global registry, the storing is triggered from within duchain, so you don't need to care about it.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry {
    ItemRepositoryRegistry();

  public:
    ~ItemRepositoryRegistry();

    /// @returns The global item-repository registry.
    static ItemRepositoryRegistry* self();

    /// Deletes the item-repository of a specified session; or, if it is currently used, marks it for deletion at exit.
    static void deleteRepositoryFromDisk(ISession* session);

    /// @param path  A shared directory-path that the item-repositories are to be loaded from.
    /// @param clear Whether a fresh start should be done with all repositories cleared.
    /// @returns     Whether the repository registry has been opened successfully.
    ///              If @c false, then all registered repositories should have been deleted.
    /// @note        Currently the given path must reference a hidden directory, just to make sure we're
    ///              not accidentally deleting something important.
    bool open(const QString& path, bool clear = false);

    /// Close all contained repositories.
    /// @warning The current state is not stored to disk.
    void close();

    /// Add a new repository.
    /// It will automatically be opened with the current path, if one is set.
    void registerRepository(AbstractItemRepository* repository, AbstractRepositoryManager* manager);

    /// Remove a repository.
    /// It will automatically be closed (if it was open).
    void unRegisterRepository(AbstractItemRepository* repository);

    /// @returns The path to item-repositories.
    QString path() const;

    /// Stores all repositories to disk, eventually unloading unused data to save memory.
    /// @note Should be called on a regular basis.
    void store();

    /// Indicates that the application has been closed gracefully.
    /// @note Must be called somewhere at the end of the shutdown sequence.
    void shutdown();

    /// Does a big cleanup, removing all non-persistent items in the repositories.
    /// @returns Count of bytes of data that have been removed.
    int finalCleanup();

    /// Prints the statistics of all registered item-repositories to the command line using kDebug().
    void printAllStatistics() const;

    /// Marks the directory as inconsistent, so it will be discarded
    /// on next startup if the application crashes during the write process.
    void lockForWriting();

    /// Removes the inconsistency mark set by @ref lockForWriting().
    void unlockForWriting();

    /// Returns a custom counter persistently stored as part of item-repositories in the
    /// same directory, possibly creating it.
    /// @param identity     The string used to identify a counter.
    /// @param initialValue Value to initialize a previously inexistent counter with.
    QAtomicInt& getCustomCounter(const QString& identity, int initialValue);

    /// @returns The global item-repository mutex.
    /// @note    Can be used to protect the initialization.
    QMutex& mutex();

  private:
    /// @returns default item-repository path (e. g. ~/cache/.kdevduchain)
    /// for a given session, creating it if needed.
    static QString repositoryPathForSession(ISession* session);
    void deleteDataDirectory(bool recreate = true);
    static ItemRepositoryRegistry* m_self;
    bool m_shallDelete;
    QString m_path;
    QMap<AbstractItemRepository*, AbstractRepositoryManager*> m_repositories;
    QMap<QString, QAtomicInt*> m_customCounters;
    mutable QMutex m_mutex;
};

/// @returns The global item-repository registry (now it is @ref ItemRepositoryRegistry::self()).
KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry& globalItemRepositoryRegistry();

}

#endif // ITEMREPOSITORYREGISTRY_H

