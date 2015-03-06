// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------


#ifndef SMDATASTRUTCS_H
#define SMDATASTRUTCS_H
#include "smCore/smConfig.h"

template<typename T>
class smIndiceArrayIter;
/// \brief  inidice array for fast access to index
template<typename T>
class smIndiceArray
{
protected:
    /// \brief  storage
    T *storage;
    /// \brief  check if the slot is empty
    smBool *isEmpty;
    /// \brief  indices
    smInt *indices;
    /// \brief  number of elements
    smInt nbrElements;
    /// \brief  maximum storage
    smInt maxStorage;
public:
    /// \brief  destructor
    ~smIndiceArray()
    {
        delete [] indices;
        delete [] storage;
        delete [] isEmpty;
    }
    /// \brief constructor; gets maximum number of elements
    inline smIndiceArray(smInt p_maxStorage)
    {
        maxStorage = p_maxStorage;
        indices = new smInt[p_maxStorage];
        storage = new T[p_maxStorage];
        isEmpty = new smBool[p_maxStorage];

        for (smInt i = 0; i < p_maxStorage; i++)
        {
            isEmpty[i] = true;
        }

        nbrElements = 0;
    }
    /// \brief  add item
    inline smInt add(T p_item)
    {
        smInt index = -1;

        if (nbrElements < maxStorage)
        {
            for (smInt i = 0; i < maxStorage; i++)
            {
                if (isEmpty[i])
                {
                    index = i;
                    break;
                }
            }

            if (index >= 0 && index < maxStorage)
            {
                isEmpty[index] = false;
                storage[index] = p_item;
                indices[nbrElements] = index;
                nbrElements++;
                return index;
            }
        }

        return index;
    }
    /// \brief check if the item exists, if not add
    inline smInt checkAndAdd(T p_item)
    {
        smInt index = -1;

        if (nbrElements < maxStorage)
        {
            for (smInt i = 0; i < nbrElements; i++)
                if (storage[indices[i]] == p_item)
                {
                    break;
                }

            return add(p_item);
        }

        return index;
    }
    /// \brief remove the element from array
    inline smBool remove(smInt p_itemIndex)
    {
        smInt counter = 0;

        if (p_itemIndex >= 0 && p_itemIndex < maxStorage)
        {
            for (smInt i = 0; i < nbrElements; i++)
            {
                if (i == p_itemIndex)
                {
                    continue;
                }

                indices[counter] = indices[i];
                counter++;
            }

            nbrElements--;
            isEmpty[p_itemIndex] = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    /// \brief replace the entry in p_index slot with p_item
    inline smBool replace(smInt p_index, T &p_item)
    {
        if (isEmpty[p_index] == false)
        {
            storage[p_index] = p_item;
            return true;
        }

        return false;
    }
    /// \brief get element by reference
    inline T& getByRef(smInt p_index)
    {
        return storage[p_index];
    }
    /// \brief  get element by safe. If the element doesn't exist it will return false
    inline smBool getByRefSafe(smInt p_index, T&p_item)
    {
        if (isEmpty[p_index])
        {
            return false;
        }
        else
        {
            p_item = storage[p_index];
            return true;
        }
    }
    /// \brief get element by reference with string
    inline T& getByRef(smString p_string)
    {
        for (smInt i = 0; i < nbrElements; i++)
        {
            if (storage[indices[i]] == p_string)
            {
                return storage[indices[i]];
            }
        }
    }
    /// \brief for iterative access
    friend smIndiceArrayIter<T>;
    /// \brief print the elements
    inline void print()
    {
        for (smInt i = 0; i < nbrElements; i++)
        {
            storage[indices[i]]->print();
        }
    }
    /// \brief operators
    inline T& operator[](smInt p_index)
    {
        return storage[indices[p_index]];
    }

    inline smInt size()
    {
        return nbrElements;
    }
    /// \brief copy from another p_array
    inline smBool copy(smIndiceArray &p_array)
    {
        if (maxStorage < p_array.maxStorage)
        {
            for (smInt j = 0; j < maxStorage; j++)
            {
                isEmpty[j] = true;
            }

            for (smInt i = 0; i < p_array.nbrElements; i++)
            {
                storage[indices[i]] = p_array.storage[p_array.indices[i]];
                indices[i] = p_array.indices[i];
                isEmpty[i] = p_array.isEmpty[p_array.indices[i]];
            }

            return true;
        }

        return false;
    }
};

/// \brief  iteration
template<class T>
class smIndiceArrayIter
{
    smInt index;
    smIndiceArray<T> *arrayPtr;
public:
    /// \brief constructor that require index array
    smIndiceArrayIter(smIndiceArray<T> *p_array)
    {
        arrayPtr = p_array;
    }
    /// \brief  operators for accessing and iteration
    T& operator[](smInt p_index)
    {
        return arrayPtr->storage[ arrayPtr->indices[p_index]];
    }

    smInt& operator++() //prefix
    {
        return ++index;
    }

    smInt operator++(const int)  //postfix
    {
        return index++;
    }

    smInt& operator--() //prefix
    {
        return --index;
    }

    smInt operator--(const int)  //postfix
    {
        return index--;
    }

    inline smInt begin()
    {
        index = 0;
        return index;
    }

    inline smInt end()
    {
        return arrayPtr->nbrElements;
    }
};
/// \brief hash bucket size
#define SIMMEDTK_HASHBUCKET_SIZE    10
/// \brief a hash entry; bucket
template <class T>
struct smEntryList
{
public:
    smEntryList()
    {
        totalEntries = 0;
        p_entry = NULL;
    }

    T ID[SIMMEDTK_HASHBUCKET_SIZE];
    smUInt totalEntries;
    smEntryList* p_entry;
};
/// \brief hash iterator
template <class T>
struct smHashIterator
{
    /// \brief  default initializaiton
    smHashIterator()
    {
        tableIndex = 0;
        currentIndex = 0;
        bucketStart = NULL;
    }
    /// \brief  to clone the has iterator
    inline void clone(smHashIterator<T> &p_iterator)
    {
        tableIndex = p_iterator.tableIndex;
        currentIndex = p_iterator.currentIndex;
        iterator = p_iterator.iterator;
    }

    template<typename K>
    inline void clone(smHashIterator<K> &p_iterator)
    {
        tableIndex = p_iterator.tableIndex;
        currentIndex = p_iterator.currentIndex;

    }
    /// \brief reseting the index for bucket iteration
    inline void resetBucketIteration()
    {
        currentIndex = 0;
        iterator = bucketStart;
    }

    smEntryList<T> *iterator;
    smEntryList<T> *bucketStart;
    smInt tableIndex;
    smInt currentIndex;

    friend std::ostream &operator<<(std::ostream &out, smHashIterator<T> &p)
    {
        out << "Table Index:" << p.tableIndex << " Current Bucket Index:" << p.currentIndex;
        return out;
    }
};
/// \brief hash return codes
enum SIMMEDTK_HASHRETURN_CODES
{
    SIMMEDTK_HASH_ENTRYREMOVED,
    SIMMEDTK_HASH_ENTRYALREADYEXISTS,
    SIMMEDTK_HASH_ENTRYOTEXIST,
    SIMMEDTK_HASH_NOTBUCKETS,
    SIMMEDTK_HASH_ENTRYINSERTED,
    SIMMEDTK_HASH_ALLOCATED_INSERTED,
    SIMMEDTK_HASH_SUCCESS
};
/// \brief hash implementation. Table->
template <class T>
class smHash
{
    /// \brief primitive ids that is stored with entry list
    smEntryList<T> *primitiveIDs;
    /// \brief  current table index
    smLongInt currentTableIndex;
    /// \brief current entry index in the bucket
    smLongInt currentEntryIndex;
    /// \brief  current bucket pointer
    smEntryList<T> *currentIterationBucket;
public:
    /// \brief  number of pritimives
    smLongInt num_prim;
    /// \brief  table size
    smLongInt tableSize;
private:
    /// \brief entries in the bucket
    inline void moveEntriesInBucket(smEntryList<T> &p_Bucket, smInt p_entryIndex)
    {
        for (unsigned smInt i = p_entryIndex; i < p_Bucket.totalEntries - 1; i++)
        {
            p_Bucket.ID[i] = p_Bucket.ID[i + 1];
        }

        p_Bucket.totalEntries--;
    }
    /// \brief check if the p_prim exists in the bucket
    inline bool checkIdentical(smEntryList<T> &p_entry, T p_prim)
    {
        for (unsigned smInt i = 0; i < p_entry.totalEntries; i++)
        {
            if (p_entry.ID[i] == p_prim)
            {
                return true;
            }
        }

        return false;
    }
    /// \brief find the bucket entry and update it
    inline bool findandUpdateEntry(smEntryList<T> &p_startEntry, T &p_prim)
    {
        smEntryList<T> *currentBucket = &p_startEntry;

        while (true)
        {
            if (currentBucket == NULL)
            {
                return false;
            }

            for (smInt i = 0; i < currentBucket->totalEntries; i++)
            {
                if (currentBucket->ID[i] == p_prim)
                {
                    currentBucket->ID[i] = p_prim;
                    return true;
                }
            }

            currentBucket = currentBucket->p_entry;
        }
    }
    /// \brief clear buckets
    inline void clearBuckets(smEntryList<T> &p_startEntry)
    {
        smEntryList<T> *currentBucket = &p_startEntry;

        while (true)
        {
            if (currentBucket == NULL)
            {
                return;
            }

            currentBucket->totalEntries = 0;
            currentBucket = currentBucket->p_entry;
        }
    }

public:
    /// \brief initializes the table; entry
    smHash(smInt p_tableSize)
    {
        tableSize = p_tableSize;
        primitiveIDs = new smEntryList<T>[tableSize];
        currentEntryIndex = 0;
        currentTableIndex = 0;
        num_prim = 0;
    }
    /// \brief  insert the entry; indicate as p_triangle. It can be any type not only a triangle
    /// hashindex is the has number generated for the entry
    inline SIMMEDTK_HASHRETURN_CODES insert(T p_triangle, smUInt hashIndex)
    {
        smEntryList<T> *currentBucket;
        smEntryList<T> *prevBucket;
        smEntryList<T> *emptySpace = NULL;
        prevBucket = &primitiveIDs[hashIndex];
        currentBucket = &primitiveIDs[hashIndex];

        while (true)
        {
            if (currentBucket == NULL)
            {
                break;
            }

            if (emptySpace == NULL && currentBucket->totalEntries < SIMMEDTK_HASHBUCKET_SIZE)
            {
                emptySpace = currentBucket;
                break;
            }

            prevBucket = currentBucket;
            currentBucket = currentBucket->p_entry;
        }

        if (currentBucket == NULL && emptySpace == NULL)
        {
            prevBucket->p_entry = new smEntryList<T>;
            prevBucket->p_entry->ID[prevBucket->p_entry->totalEntries] = p_triangle;
            prevBucket->p_entry->totalEntries++;
            num_prim++;
            return  SIMMEDTK_HASH_ALLOCATED_INSERTED;
        }
        else
        {
            emptySpace->ID[emptySpace->totalEntries] = p_triangle;
            emptySpace->totalEntries++;
        }

        num_prim++;
        return SIMMEDTK_HASH_ENTRYINSERTED;
    }
    /// \brief check if there is an entry and index
    inline SIMMEDTK_HASHRETURN_CODES checkAndInsert(T p_triangle, smUInt hashIndex)
    {
        smEntryList<T> *currentBucket;
        smEntryList<T> *prevBucket;
        smEntryList<T> *emptySpace = NULL;
        prevBucket = &primitiveIDs[hashIndex];
        currentBucket = &primitiveIDs[hashIndex];

        while (true)
        {
            if (currentBucket == NULL)
            {
                break;
            }

            if (checkIdentical(*currentBucket, p_triangle))
            {
                return SIMMEDTK_HASH_ENTRYALREADYEXISTS;
            }

            if (emptySpace == NULL && currentBucket->totalEntries < SIMMEDTK_HASHBUCKET_SIZE)
            {
                emptySpace = currentBucket;
                break;
            }

            prevBucket = currentBucket;
            currentBucket = currentBucket->p_entry;
        }

        if (currentBucket == NULL && emptySpace == NULL)
        {
            prevBucket->p_entry = new smEntryList<T>;
            prevBucket->p_entry->ID[prevBucket->p_entry->totalEntries] = p_triangle;
            prevBucket->p_entry->totalEntries++;
            num_prim++;
            return SIMMEDTK_HASH_ALLOCATED_INSERTED;
        }
        else
        {
            emptySpace->ID[emptySpace->totalEntries] = p_triangle;
            emptySpace->totalEntries++;
        }

        num_prim++;
        return SIMMEDTK_HASH_ENTRYINSERTED;
    }
    /// \brief starts the iteration. reset the indices
    inline void startIteration()
    {
        currentTableIndex = 0;
        currentEntryIndex = 0;
        currentIterationBucket = &primitiveIDs[currentTableIndex];
    }
    /// \brief  go to next table index
    inline bool next(smHashIterator<T> &p_iterator)
    {
        if (p_iterator.tableIndex >= tableSize)
        {
            return false;
        }

        p_iterator.iterator = &primitiveIDs[p_iterator.tableIndex];
        p_iterator.bucketStart = &primitiveIDs[p_iterator.tableIndex];
        p_iterator.currentIndex = 0;
        p_iterator.tableIndex++;
        return true;
    }
    /// \brief  iterate over the next bucket item
    inline bool nextBucketItem(smHashIterator<T> &p_iterator, T &p_prim)
    {
        while (true)
        {
            if (p_iterator.iterator == NULL)
            {
                p_iterator.currentIndex = 0;
                return false;
            }

            if (p_iterator.iterator->totalEntries > 0 && p_iterator.iterator->totalEntries > p_iterator.currentIndex)
            {
                p_prim = p_iterator.iterator->ID[p_iterator.currentIndex];
                p_iterator.currentIndex++;
                return true;
            }

            if (p_iterator.iterator->totalEntries == 0 || p_iterator.currentIndex >= p_iterator.iterator->totalEntries)
            {

                p_iterator.iterator = p_iterator.iterator->p_entry;
                p_iterator.currentIndex = 0;
                continue;
            }
        }
    }
    /// \brief proceed to next entry and return the element. it the bucket ends it will go to next table index
    inline bool next(T &p_prim)
    {
        while (true)
        {
            if (currentTableIndex >= tableSize)
            {
                return false;
            }

            if (currentIterationBucket == NULL)
            {
                currentTableIndex++;
                currentEntryIndex = 0;
                currentIterationBucket = &primitiveIDs[currentTableIndex];
                continue;
            }

            if (currentIterationBucket->totalEntries == 0 || currentEntryIndex >= currentIterationBucket->totalEntries)
            {
                currentIterationBucket = currentIterationBucket->p_entry;
                currentEntryIndex = 0;
                continue;
            }

            if (currentIterationBucket->totalEntries > 0 && currentIterationBucket->totalEntries > currentEntryIndex)
            {
                p_prim = currentIterationBucket->ID[currentEntryIndex];
                currentEntryIndex++;
                return true;
            }

            currentTableIndex++;
        }
    }
    /// \brief next element by getting reference to it
    inline smBool nextByRef(T **p_prim)
    {
        while (true)
        {
            if (currentTableIndex >= tableSize)
            {
                return false;
            }

            if (currentIterationBucket == NULL)
            {
                currentTableIndex++;
                currentEntryIndex = 0;
                currentIterationBucket = &primitiveIDs[currentTableIndex];
                continue;
            }

            if (currentIterationBucket->totalEntries == 0 || currentEntryIndex >= currentIterationBucket->totalEntries)
            {
                currentIterationBucket = currentIterationBucket->p_entry;
                currentEntryIndex = 0;
                continue;
            }

            if (currentIterationBucket->totalEntries > 0 && currentIterationBucket->totalEntries > currentEntryIndex)
            {
                *p_prim = &currentIterationBucket->ID[currentEntryIndex];
                currentEntryIndex++;
                return true;
            }

            currentTableIndex++;
        }
    }
    /// \brief  print all the content
    inline void printContent()
    {
        T prim;
        startIteration();

        while (next(prim))
        {
            std::cout << "Table:" << currentTableIndex << " Bucket Index:" << currentEntryIndex << " Prim:" << prim << "\n";
        }
    }
    /// \brief  clear all table, including the elements
    void clearAll()
    {
        for (smInt i = 0; i < tableSize; i++)
        {
            clearBuckets(primitiveIDs[i]);
        }

        num_prim = 0;
    }
};
/// \brief sliding  storage type
enum smStorageSlideType
{
    SIMMEDTK_STORAGESLIDING_FRONTFIRST,
    SIMMEDTK_STORAGESLIDING_LASTFIRST
};
/// \brief sliding storage window
template<typename T>
class smStorageSlidingWindow
{
private:
    /// \brief  type
    smStorageSlideType strorageType;
public:
    /// \brief  data
    T *data;
    /// \brief window size
    unsigned int windowSize;
    /// \brief constructor
    ~smStorageSlidingWindow()
    {
        delete[] data;
    }
    /// \brief siding window storage. constructor  gets window size, type
    smStorageSlidingWindow(int p_windowSize = 10, smStorageSlideType p_type = SIMMEDTK_STORAGESLIDING_LASTFIRST)
    {
        data = new T[p_windowSize];
        windowSize = p_windowSize;
        memset(data, 0, sizeof(T)*p_windowSize);
        strorageType = p_type;
    }
    /// \brief storage type
    inline smStorageSlideType getStorageType()
    {
        return strorageType;
    }
    /// \brief  resize storage
    inline void resize(int p_newSize)
    {
        T*tempData;
        int index;

        if (windowSize == p_newSize)
        {
            return;
        }

        tempData = new T[p_newSize];
        memset(tempData, 0, sizeof(T)*p_newSize);

        if (strorageType == SIMMEDTK_STORAGESLIDING_LASTFIRST)
        {
            if (p_newSize > windowSize)
            {
                memcpy(tempData + p_newSize - windowSize, data, windowSize * sizeof(T));
            }
            else
            {
                memcpy(tempData, data + p_newSize, p_newSize * sizeof(T));
            }
        }
        else
        {
            if (p_newSize > windowSize)
            {
                memcpy(tempData, data, windowSize * sizeof(T));
            }
            else
            {
                memcpy(tempData, data, p_newSize * sizeof(T));
            }
        }

        delete[] data;
        data = tempData;
        windowSize = p_newSize;
    }
    /// \brief add value
    inline void add(T p_value)
    {
        if (strorageType == SIMMEDTK_STORAGESLIDING_LASTFIRST)
        {
            memcpy(data, &data[1], (windowSize - 1)*sizeof(T));
            data[windowSize - 1] = p_value;
        }
        else
        {
            for (int i = windowSize - 1; i > 0; i--)
            {
                data[i] = data[i - 1];
            }

            data[0] = p_value;
        }
    }
    /// \brief  zero all the entries
    inline void zeroed()
    {
        memset(data, 0, sizeof(T)*windowSize);
    }
    /// \brief  print all the elements
    void print()
    {
        for (int i = 0; i < windowSize; i++)
        {
            std::cout << data[i] << " ";
        }

        std::cout << "\n";
    }
};

#endif
