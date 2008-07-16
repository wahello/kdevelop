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

#ifndef ITEMREPOSITORY_H
#define ITEMREPOSITORY_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QMutex>
#include <QList>
#include <QDir>
#include <QFile>
#include <kdebug.h>
#include "../languageexport.h"

namespace KDevelop {

  /**
   * This file implements a generic bucket-based indexing repository, that can be used for example to index strings.
   * 
   * All you need to do is define your item type that you want to store into the repository, and create a request item
   * similar to ExampleRequestItem that compares and fills the defined item type.
   * 
   * For example the string repository uses "unsigned short" as item-type, uses that actual value to store the length of the string,
   * and uses the space behind to store the actual string content.
   *
   * @see ItemRepository
   * @see stringrepository.h
   * @see indexedstring.h
   *
   * Items must not be larger than the bucket size!
   * */


class KDEVPLATFORMLANGUAGE_EXPORT AbstractItemRepository {
  public:
    virtual ~AbstractItemRepository();
    ///@param path is supposed to be a shared directory-name that the item-repository is to be loaded from
    ///@param clear will be true if the old repository should be discarded and a new one started
    ///If this returns false, that indicates that loading failed. In that case, all repositories will be discarded.
    virtual bool open(const QString& path, bool clear) = 0;
    virtual void close() = 0;
};

/**
 * Manages a set of item-repositores and allows loading/storing them all at once from/to disk.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry {
  public:
    ItemRepositoryRegistry();
    ~ItemRepositoryRegistry();
    
    ///Path is supposed to be a shared directory-name that the item-repositories are to be loaded from
    ///@param clear Whether a fresh start should be done, and all repositories cleared
    ///If this returns false, loading has failed, and all repositories have been discarded.
    bool open(const QString& path, bool clear = false);
    void close();
    ///The registered repository will automatically be opened with the current path, if one is set.
    void registerRepository(AbstractItemRepository* repository);
    ///The registered repository will automatically be closed if it was open.
    void unRegisterRepository(AbstractItemRepository* repository);
  private:
    QString m_path;
    QList<AbstractItemRepository*> m_repositories;
    bool m_cleared;
};

///The global item-repository registry that is used by default
KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry& globalItemRepositoryRegistry();

  ///This is the actual data that is stored in the repository. All the data that is not directly in the class-body,
  ///like the text of a string, can be stored behind the item in the same memory region. The only important thing is
  ///that the Request item(@see ExampleItemRequest) correctly advertises the space needed by this item.
class ExampleItem {
};

/**
 * A request represents the information that is searched in the repository.
 * It must be able to compare itself to items stored in the repository, and it must be able to
 * create items in the. The item-types can also be basic data-types, with additional information stored behind.
 * */

enum {
  ItemRepositoryBucketSize = 1<<16
};

class ExampleRequestItem {

  enum {
    AverageSize = 10 //This should be the approximate average size of an Item
  };

  typedef unsigned int HashType;
  
  //Should return the hash-value associated with this request(For example the hash of a string)
  HashType hash() const {
    return 0;
  }
  
  //Should return the size of an item created with createItem
  size_t itemSize() const {
      return 0;
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  ///@warning Never call non-constant functions on the repository from within this function!
  void createItem(ExampleItem* /*item*/) const {
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const ExampleItem* /*item*/) const {
    return false;
  }
};

template<class Item, class ItemRequest>
class KDEVPLATFORMLANGUAGE_EXPORT Bucket {
  enum {
    NextBucketHashSize = 500 //Affects the average count of bucket-chains that need to be walked in ItemRepository::index
  };
  public:
    enum {
      ObjectMapSize = (ItemRepositoryBucketSize / ItemRequest::AverageSize) + 1,
      DataSize = sizeof(unsigned int) + ItemRepositoryBucketSize + sizeof(short unsigned int) * (ObjectMapSize + NextBucketHashSize)
    };
    Bucket() : m_available(0), m_data(0), m_objectMap(0), m_objectMapSize(0), m_largestFreeItem(0), m_nextBucketHash(0) {
    }
    ~Bucket() {
      delete[] m_data;
      delete[] m_nextBucketHash;
      delete[] m_objectMap;
    }

    void initialize() {
      if(!m_data) {
        m_available = ItemRepositoryBucketSize;
        m_data = new char[ItemRepositoryBucketSize];
        //The bigger we make the map, the lower the probability of a clash(and thus bad performance). However it increases memory usage.
        m_objectMapSize = ObjectMapSize;
        m_objectMap = new short unsigned int[m_objectMapSize];
        memset(m_objectMap, 0, m_objectMapSize * sizeof(short unsigned int));
        m_nextBucketHash = new short unsigned int[NextBucketHashSize];
        memset(m_nextBucketHash, 0, NextBucketHashSize * sizeof(short unsigned int));
      }
    }
    void initialize(QFile* file, size_t offset) {
      if(!m_data) {
        initialize();
        if(file->size() >= offset + DataSize) {
          file->seek(offset);
          
          file->read((char*)&m_available, sizeof(unsigned int));
          file->read(m_data, ItemRepositoryBucketSize);
          file->read((char*)m_objectMap, sizeof(short unsigned int) * m_objectMapSize);
          file->read((char*)m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize);
        }
      }
    }

    void store(QFile* file, size_t offset) {
      if(!m_data)
        return;
      
      if(file->size() < offset + DataSize)
        file->resize(offset + DataSize);

      file->seek(offset);
      
      file->write((char*)&m_available, sizeof(unsigned int));
      file->write(m_data, ItemRepositoryBucketSize);
      file->write((char*)m_objectMap, sizeof(short unsigned int) * m_objectMapSize);
      file->write((char*)m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize);
    }

    //Tries to find the index this item has in this bucket, or returns zero if the item isn't there yet.
    unsigned short findIndex(const ItemRequest& request) const {
      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index)))
        return index; //We have found the item

      return 0;
    }
    
    //Tries to get the index within this bucket, or returns zero. Will put the item into the bucket if there is room.
    //Created indices will never begin with 0xffff____, so you can use that index-range for own purposes.
    unsigned short index(const ItemRequest& request) {
      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];
      unsigned short insertedAt = 0;

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index)))
        return index; //We have found the item

      unsigned int totalSize = request.itemSize() + 2;
      if(totalSize > m_available) {
        //Try finding the smallest freed item that can hold the data
        unsigned short currentIndex = m_largestFreeItem;
        unsigned short previousIndex = 0;
        while(currentIndex && freeSize(currentIndex) >= (totalSize-2)) {
          unsigned short follower = followerIndex(currentIndex);
          if(follower && freeSize(follower) >= (totalSize-2)) {
            //The item also fits into the smaller follower, so use that one
            previousIndex = currentIndex;
            currentIndex = follower;
          }else{
            //The item fits into currentIndex, but not into the follower. So use currentIndex
            break;
          }
        }
        
        if(!currentIndex || freeSize(currentIndex) < (totalSize-2))
          return 0;
        
        if(previousIndex)
          setFollowerIndex(previousIndex, followerIndex(currentIndex));
        else
          m_largestFreeItem = followerIndex(currentIndex);
        
        insertedAt = currentIndex;
      }else{
        //We have to insert the item
        insertedAt = ItemRepositoryBucketSize - m_available;
        
        insertedAt += 2; //Room for the prepended follower-index

        m_available -= totalSize;
      }
      
      if(index)
        setFollowerIndex(index, insertedAt);
      setFollowerIndex(insertedAt, 0);
      
      if(m_objectMap[localHash] == 0)
        m_objectMap[localHash] = insertedAt;
      
      char* borderBehind = m_data + insertedAt + (totalSize-2);
      
      quint64 oldValueBehind = 0;
      if(m_available >= 8) {
        oldValueBehind = *(quint64*)borderBehind;
        *((quint64*)borderBehind) = 0xfafafafafafafafaLLU;
      }
      
      //Last thing we do, because createItem may recursively do even more transformation of the repository
      request.createItem((Item*)(m_data + insertedAt));
      
      if(m_available >= 8) {
        //If this assertion triggers, then the item writes a bigger range than it advertised in 
        Q_ASSERT(*((quint64*)borderBehind) == 0xfafafafafafafafaLLU);
        *((quint64*)borderBehind) = oldValueBehind;
      }
      
      return insertedAt;
    }
    
    void deleteItem(unsigned short index, unsigned int hash, unsigned short size) {
      //Step 1: Remove the item from the data-structures that allow finding it: m_objectMap
      unsigned short localHash = hash % m_objectMapSize;
      unsigned short currentIndex = m_objectMap[localHash];
      unsigned short previousIndex = 0;
      //Fix the follower-link by setting the follower of the previous item to the next one, or updating m_objectMap
      while(currentIndex != index) {
        previousIndex = currentIndex;
        currentIndex = followerIndex(currentIndex);
        //If this assertion triggers, the deleted item was not registered under the given hash
        Q_ASSERT(currentIndex);
      }
      
      if(!previousIndex)
        //The item was directly in the object map
        m_objectMap[localHash] = followerIndex(index);
      else
        setFollowerIndex(previousIndex, followerIndex(index));
      
      setFreeSize(index, size);
      
      //Insert the free item into the chain opened by m_largestFreeItem
      currentIndex = m_largestFreeItem;
      previousIndex = 0;
      
      while(currentIndex && *(unsigned short*)(m_data + currentIndex) > size) {
        previousIndex = currentIndex;
        currentIndex = followerIndex(currentIndex);
      }
      
      if(previousIndex)
        //This item is larger than all already registered free items, or there are none.
        setFollowerIndex(previousIndex, index);

      setFollowerIndex(index, currentIndex);
    }
    
    inline const Item* itemFromIndex(unsigned short index) const {
      return (Item*)(m_data+index);
    }
    
    uint usedMemory() const {
      return ItemRepositoryBucketSize - m_available;
    }
    
    template<class Visitor>
    bool visitAllItems(Visitor& visitor) const {
      for(int a = 0; a < m_objectMapSize; ++a) {
        uint currentIndex = m_objectMap[a];
        while(currentIndex) {
          if(!visitor((const Item*)(m_data+currentIndex)))
            return false;
          
          currentIndex = followerIndex(currentIndex);
        }
      }
      return true;
    }
    
    unsigned short nextBucketForHash(uint hash) {
      return m_nextBucketHash[hash % NextBucketHashSize];
    }
    
    void setNextBucketForHash(unsigned int hash, unsigned short bucket) {
      m_nextBucketHash[hash % NextBucketHashSize] = bucket;
    }
    
  private:
    
    ///@param index the index of an item @return The index of the next item in the chain of items with a same local hash, or zero
    inline unsigned short followerIndex(unsigned short index) const {
      Q_ASSERT(index >= 2);
      return *((unsigned short*)(m_data+(index-2)));
    }

    void setFollowerIndex(unsigned short index, unsigned short follower) {
      Q_ASSERT(index >= 2);
      *((unsigned short*)(m_data+(index-2))) = follower;
    }
    //Only returns the corrent value if the item is actually free
    inline unsigned short freeSize(unsigned short index) const {
      return *((unsigned short*)(m_data+index));
    }

    //Convenience function to set the free-size, only for freed items
    void setFreeSize(unsigned short index, unsigned short size) {
      *((unsigned short*)(m_data+index)) = size;
    }

    unsigned int m_available;
    char* m_data; //Structure of the data: <Position of next item with same hash modulo ItemRepositoryBucketSize>(2 byte), <Item>(item.size() byte)
    short unsigned int* m_objectMap; //Points to the first object in m_data with (hash % m_objectMapSize) == index. Points to the item itself, so substract 1 to get the pointer to the next item with same local hash.
    uint m_objectMapSize;
    short unsigned int m_largestFreeItem; //Points to the largest item that is currently marked as free, or zero. That one points to the next largest one through followerIndex
    
    unsigned short* m_nextBucketHash;
};

template<bool lock>
struct Locker { //This is a dummy that does nothing
  template<class T>
  Locker(const T& /*t*/) {
  }
};
template<>
struct Locker<true> : public QMutexLocker {
  Locker(QMutex* mutex) : QMutexLocker(mutex) {
  }
};

///@param Item @see ExampleItem
///@param threadSafe Whether class access should be thread-safe
template<class Item, class ItemRequest, bool threadSafe = true, unsigned int bucketHashSize = 524288>
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepository : public AbstractItemRepository {

  typedef Locker<threadSafe> ThisLocker;
  
  enum {
    ItemRepositoryVersion = 1,
    BucketStartOffset = sizeof(uint) * 7 + sizeof(short unsigned int) * bucketHashSize //Position in the data where the bucket array starts
  };
  
  public:
  ///@param registry May be zero, then the repository will not be registered at all. Else, the repository will register itself to that registry.
  ItemRepository(QString repositoryName, ItemRepositoryRegistry* registry  = &globalItemRepositoryRegistry(), uint repositoryVersion = 1) : m_mutex(QMutex::Recursive), m_repositoryName(repositoryName), m_registry(registry), m_file(0), m_repositoryVersion(repositoryVersion) {
    m_buckets.resize(10);
    m_buckets.fill(0);
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    
    m_firstBucketForHash = new short unsigned int[bucketHashSize];
    memset(m_firstBucketForHash, 0, bucketHashSize * sizeof(short unsigned int));
    
    m_statBucketHashClashes = m_statItemCount = 0;
    m_currentBucket = 1; //Skip the first bucket, we won't use it so we have the zero indices for special purposes
    if(m_registry)
      m_registry->registerRepository(this);
  }
  
  ~ItemRepository() {
    if(m_registry)
      m_registry->unRegisterRepository(this);

    close();
  }

  ///Returns the index for the given item. If the item is not in the repository yet, it is inserted.
  ///The index can never be zero. Zero is reserved for your own usage as invalid
  unsigned int index(const ItemRequest& request) {
    
    ThisLocker lock(&m_mutex);
    
    unsigned int hash = request.hash();
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + ((hash * 1234271) % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;
    
    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there
      
      Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[previousBucketNumber];
      if(!bucketPtr) {
        initializeBucket(previousBucketNumber);
        bucketPtr = m_fastBuckets[previousBucketNumber];
      }
      
      unsigned short indexInBucket = bucketPtr->findIndex(request);
      if(indexInBucket) {
        //We've found the item, it's already there
        return (previousBucketNumber << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
      } else {
        //The item isn't in bucket previousBucketNumber, but maybe the bucket has a pointer to the next bucket that might contain the item
        //Should happen rarely
        short unsigned int next = bucketPtr->nextBucketForHash(hash);
        if(next)
          previousBucketNumber = next;
        else
          break;
      }
    }
    
    //The item isn't in the repository set, find a new bucket for it
    
    while(1) {
      if(m_currentBucket >= m_bucketCount) {
          if(m_bucketCount >= 0xfffe) { //We have reserved the last bucket index 0xffff for special purposes
          //the repository has overflown.
          kWarning() << "Found no room for an item in" << m_repositoryName << "size of the item:" << request.itemSize();
          return 0;
        }else{
          //Allocate new buckets
          uint oldBucketCount = m_bucketCount;
          m_bucketCount += 10;
          m_buckets.resize(m_bucketCount);
          m_fastBuckets = m_buckets.data();
          memset(m_fastBuckets + oldBucketCount, 0, (m_bucketCount-oldBucketCount) * sizeof(void*));
        }
      }
      Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[m_currentBucket];
      if(!bucketPtr) {
        initializeBucket(m_currentBucket);
        bucketPtr = m_fastBuckets[m_currentBucket];
      }
      unsigned short indexInBucket = bucketPtr->index(request);
      
      if(indexInBucket) {
        if(!(*bucketHashPosition))
          (*bucketHashPosition) = m_currentBucket;
        
        ++m_statItemCount;

        //Set the follower pointer in the earlier bucket for the hash
        if(previousBucketNumber && previousBucketNumber != m_currentBucket) {
          //Should happen rarely
          ++m_statBucketHashClashes;
          m_buckets[previousBucketNumber]->setNextBucketForHash(request.hash(), m_currentBucket);
        }
        
        return (m_currentBucket << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
      }else{
        ++m_currentBucket;
      }
    }
    //We haven't found a bucket that already contains the item, so append it to the current bucket
    
    kWarning() << "Found no bucket for an item in" << m_repositoryName;
    return 0;
  }

  ///Deletes the item from the repository. It is crucial that the given hash and size are correct.
//   void deleteItem(unsigned int index, unsigned int hash, unsigned short size) {
//     ThisLocker lock(&m_mutex);
//     
//     short unsigned int previousBucketNumber = m_firstBucketForHash[hash % bucketHashSize];
//     
//     while(previousBucketNumber) {
//       //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there
//       
//       Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[previousBucketNumber];
//       if(!bucketPtr) {
//         initializeBucket(previousBucketNumber);
//         bucketPtr = m_fastBuckets[previousBucketNumber];
//       }
//       
//       unsigned short indexInBucket = bucketPtr->findIndex(request);
//       if(indexInBucket) {
//         //We've found the item, it's already there
//         return (previousBucketNumber << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
//       } else {
//         //The item isn't in bucket previousBucketNumber, but maybe the bucket has a pointer to the next bucket that might contain the item
//         //Should happen rarely
//         short unsigned int next = bucketPtr->nextBucketForHash(hash);
//         if(next)
//           previousBucketNumber = next;
//         else
//           break;
//       }
//     }
//   }

  ///@param index The index. It must be valid(match an existing item), and nonzero.
  const Item* itemFromIndex(unsigned int index) const {
    Q_ASSERT(index);
    
    ThisLocker lock(&m_mutex);
    
    unsigned short bucket = (index >> 16);
    
    const Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[bucket];
    Q_ASSERT(bucket < m_bucketCount);
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    unsigned short indexInBucket = index & 0xffff;
    return bucketPtr->itemFromIndex(indexInBucket);
  }
  
  QString statistics() const {
    QString ret;
    uint loadedBuckets = 0;
    for(uint a = 0; a < m_bucketCount; ++a)
      if(m_fastBuckets[a])
        ++loadedBuckets;
    
    ret += QString("loaded buckets: %1 current bucket: %2 used memory: %3").arg(loadedBuckets).arg(m_currentBucket).arg(usedMemory());
    ret += QString("\nbucket hash clashed items: %1 total items: %2").arg(m_statBucketHashClashes).arg(m_statItemCount);
    //If m_statBucketHashClashes is high, the bucket-hash needs to be bigger
    return ret;
  }
  
  uint usedMemory() const {
    uint used = 0;
    for(int a = 0; a < m_buckets.size(); ++a) {
      if(m_buckets[a]) {
        used += m_buckets[a]->usedMemory();
      }
    }
    return used;
  }
  
  ///This can be used to safely iterate through all items in the repository
  ///@param visitor Should be an instance of a class that has a bool operator()(const Item*) member function,
  ///               that returns whether more items are wanted.
  ///@param onlyInMemory If this is true, only items are visited that are currently in memory.
  template<class Visitor>
  void visitAllItems(Visitor& visitor, bool /*onlyInMemory*/ = false) const {
    ThisLocker lock(&m_mutex);
    for(uint a = 0; a < m_bucketCount; ++a) {
      if(m_buckets[a])
        if(!m_buckets[a]->visitAllItems(visitor))
          return;
    }
  }

  private:
  virtual bool open(const QString& path, bool clear) {
    close();
    m_currentOpenPath = path;
    kDebug() << "opening repository" << m_repositoryName << "at" << path;
    QDir dir(path);
    m_file = new QFile(dir.absoluteFilePath( m_repositoryName ));
    if(!m_file->open( QFile::ReadWrite )) {
      delete m_file;
      m_file = 0;
      return false;
    }
    
    if(clear) {
      m_file->resize(0);
      m_file->write((char*)&m_repositoryVersion, sizeof(uint));
      uint hashSize = bucketHashSize;
      m_file->write((char*)&hashSize, sizeof(uint));
      uint itemRepositoryVersion  = ItemRepositoryVersion;
      m_file->write((char*)&itemRepositoryVersion, sizeof(uint));
      
      m_statBucketHashClashes = m_statItemCount = 0;
      
      m_file->write((char*)&m_statBucketHashClashes, sizeof(uint));
      m_file->write((char*)&m_statItemCount, sizeof(uint));
      
      m_buckets.resize(10);
      m_buckets.fill(0);
      uint bucketCount = m_buckets.size();
      m_file->write((char*)&bucketCount, sizeof(uint));

      m_firstBucketForHash = new short unsigned int[bucketHashSize];
      memset(m_firstBucketForHash, 0, bucketHashSize * sizeof(short unsigned int));

      m_currentBucket = 1; //Skip the first bucket, we won't use it so we have the zero indices for special purposes
      m_file->write((char*)&m_currentBucket, sizeof(uint));
      m_file->write((char*)m_firstBucketForHash, sizeof(short unsigned int) * bucketHashSize);
      //We have completely initialized the file now
      Q_ASSERT(m_file->pos() == BucketStartOffset);
    }else{
      //Check that the version is correct
      uint storedVersion = 0, hashSize = 0, itemRepositoryVersion = 0;

      m_file->read((char*)&storedVersion, sizeof(uint));
      m_file->read((char*)&hashSize, sizeof(uint));
      m_file->read((char*)&itemRepositoryVersion, sizeof(uint));
      m_file->read((char*)&m_statBucketHashClashes, sizeof(uint));
      m_file->read((char*)&m_statItemCount, sizeof(uint));
      
      if(storedVersion != m_repositoryVersion || hashSize != bucketHashSize || itemRepositoryVersion != ItemRepositoryVersion) {
        kDebug() << "repository" << m_repositoryName << "version mismatch in" << m_file->fileName() << ", stored: version " << storedVersion << "hashsize" << hashSize << "repository-version" << itemRepositoryVersion << " current: version" << m_repositoryVersion << "hashsize" << bucketHashSize << "repository-version" << ItemRepositoryVersion;
        delete m_file;
        m_file = 0;
        return false;
      }
      uint bucketCount;
      m_file->read((char*)&bucketCount, sizeof(uint));
      m_buckets.resize(bucketCount);
      m_buckets.fill(0);
      m_file->read((char*)&m_currentBucket, sizeof(uint));
    
      m_firstBucketForHash = new short unsigned int[bucketHashSize];
      m_file->read((char*)m_firstBucketForHash, sizeof(short unsigned int) * bucketHashSize);
      Q_ASSERT(m_file->pos() == BucketStartOffset);
    }
    
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    return true;
  }
  
  virtual void close() {
    if(!m_currentOpenPath.isEmpty()) {
    }
    m_currentOpenPath = QString();
    
    typedef Bucket<Item, ItemRequest> B;
    for(int a = 0; a < m_buckets.size(); ++a) {
      storeBucket(a);
      delete m_buckets[a];
    }

    delete m_file;
    m_file = 0;
    
    delete[] m_firstBucketForHash;
    
    m_buckets.clear();
    m_firstBucketForHash = 0;
  }

  inline void initializeBucket(unsigned int bucketNumber) const {
    if(!m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber] = new Bucket<Item, ItemRequest>();
      if(m_file)
        m_fastBuckets[bucketNumber]->initialize(m_file, BucketStartOffset + (bucketNumber-1) * Bucket<Item, ItemRequest>::DataSize);
      else
        m_fastBuckets[bucketNumber]->initialize();
    }
  }
  
  void storeBucket(unsigned int bucketNumber) const {
    if(m_file && m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber]->store(m_file, BucketStartOffset + (bucketNumber-1) * Bucket<Item, ItemRequest>::DataSize);
    }
  }

  mutable QMutex m_mutex;
  QString m_repositoryName;
  unsigned int m_size;
  mutable uint m_currentBucket;
  mutable QVector<Bucket<Item, ItemRequest>* > m_buckets;
  mutable Bucket<Item, ItemRequest>** m_fastBuckets; //For faster access
  mutable uint m_bucketCount;
  uint m_statBucketHashClashes, m_statItemCount;
  //Maps hash-values modulo 1<<bucketHashSizeBits to the first bucket such a hash-value appears in
  short unsigned int* m_firstBucketForHash;
  
  QString m_currentOpenPath;
  ItemRepositoryRegistry* m_registry;
  QFile* m_file;
  uint m_repositoryVersion;
};

}

#endif
