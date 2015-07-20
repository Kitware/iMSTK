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

#ifndef SMDATASTUCTURES_HPP
#define SMDATASTUCTURES_HPP

template<typename T>
IndiceArray<T>::~IndiceArray()
{
    delete [] indices;
    delete [] storage;
    delete [] isEmpty;
}
template<typename T>
IndiceArray<T>::IndiceArray( int p_maxStorage )
{
    maxStorage = p_maxStorage;
    indices = new int[p_maxStorage];
    storage = new T[p_maxStorage];
    isEmpty = new bool[p_maxStorage];

    for ( int i = 0; i < p_maxStorage; i++ )
    {
        isEmpty[i] = true;
    }

    nbrElements = 0;
}
template<typename T> int
IndiceArray<T>::add( T p_item )
{
    int index = -1;

    if ( nbrElements < maxStorage )
    {
        for ( int i = 0; i < maxStorage; i++ )
        {
            if ( isEmpty[i] )
            {
                index = i;
                break;
            }
        }

        if ( index >= 0 && index < maxStorage )
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
template<typename T> int
IndiceArray<T>::checkAndAdd( T p_item )
{
    int index = -1;

    if ( nbrElements < maxStorage )
    {
        for ( int i = 0; i < nbrElements; i++ )
            if ( storage[indices[i]] == p_item )
            {
                break;
            }

        return add( p_item );
    }

    return index;
}
template<typename T> bool
IndiceArray<T>::remove( int p_itemIndex )
{
    int counter = 0;

    if ( p_itemIndex >= 0 && p_itemIndex < maxStorage )
    {
        for ( int i = 0; i < nbrElements; i++ )
        {
            if ( i == p_itemIndex )
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
template<typename T> bool
IndiceArray<T>::replace( int p_index, T &p_item )
{
    if ( isEmpty[p_index] == false )
    {
        storage[p_index] = p_item;
        return true;
    }

    return false;
}
template<typename T>
T &IndiceArray<T>::getByRef( int p_index )
{
    return storage[p_index];
}
template<typename T> bool
IndiceArray<T>::getByRefSafe( int p_index, T &p_item )
{
    if ( isEmpty[p_index] )
    {
        return false;
    }
    else
    {
        p_item = storage[p_index];
        return true;
    }
}
template<typename T>
T &IndiceArray<T>::getByRef( std::string p_string )
{
    for ( int i = 0; i < nbrElements; i++ )
    {
        if ( storage[indices[i]] == p_string )
        {
            return storage[indices[i]];
        }
    }
}
template<typename T> void
IndiceArray<T>::print() const
{
    for ( int i = 0; i < nbrElements; i++ )
    {
        storage[indices[i]]->print();
    }
}
template<typename T>
T &IndiceArray<T>::operator[]( int p_index )
{
    return storage[indices[p_index]];
}
template<typename T> int
IndiceArray<T>::size()
{
    return nbrElements;
}
template<typename T> bool
IndiceArray<T>::copy( IndiceArray &p_array )
{
    if ( maxStorage < p_array.maxStorage )
    {
        for ( int j = 0; j < maxStorage; j++ )
        {
            isEmpty[j] = true;
        }

        for ( int i = 0; i < p_array.nbrElements; i++ )
        {
            storage[indices[i]] = p_array.storage[p_array.indices[i]];
            indices[i] = p_array.indices[i];
            isEmpty[i] = p_array.isEmpty[p_array.indices[i]];
        }

        return true;
    }

    return false;
}

template<typename T>
IndiceArrayIter<T>::IndiceArrayIter( IndiceArray< T > *p_array )
{
    arrayPtr = p_array;
}
template<typename T>
T &IndiceArrayIter<T>::operator[]( int p_index )
{
    return arrayPtr->storage[ arrayPtr->indices[p_index]];
}
template<typename T> int&
IndiceArrayIter<T>::operator++()
{
    return ++index;
}
template<typename T> int
IndiceArrayIter<T>::operator++( const int )
{
    return index++;
}
template<typename T> int&
IndiceArrayIter<T>::operator--()
{
    return --index;
}
template<typename T> int
IndiceArrayIter<T>::operator--( const int )
{
    return index--;
}
template<typename T> int
IndiceArrayIter<T>::begin()
{
    index = 0;
    return index;
}
template<typename T> int
IndiceArrayIter<T>::end()
{
    return arrayPtr->nbrElements;
}
template<typename T>
smEntryList<T>::smEntryList()
{
    totalEntries = 0;
    p_entry = NULL;
}
template<typename T>
smHashIterator<T>::smHashIterator()
{
    tableIndex = 0;
    currentIndex = 0;
    bucketStart = NULL;
}
template<typename T> void
smHashIterator<T>::clone( smHashIterator< T > &p_iterator )
{
    tableIndex = p_iterator.tableIndex;
    currentIndex = p_iterator.currentIndex;
    iterator = p_iterator.iterator;
}

template<typename T> void
smHashIterator<T>::resetBucketIteration()
{
    currentIndex = 0;
    iterator = bucketStart;
}
template<typename T>
std::ostream &operator<<( std::ostream &out, smHashIterator< T > &p )
{
    out << "Table Index:" << p.tableIndex << " Current Bucket Index:" << p.currentIndex;
    return out;
}
template<typename T>
void smHash<T>::moveEntriesInBucket( smEntryList< T > &p_Bucket, int p_entryIndex )
{
    for ( unsigned int i = p_entryIndex; i < p_Bucket.totalEntries - 1; i++ )
    {
        p_Bucket.ID[i] = p_Bucket.ID[i + 1];
    }

    p_Bucket.totalEntries--;
}
template<typename T>
bool smHash<T>::checkIdentical( smEntryList< T > &p_entry, T p_prim )
{
    for ( unsigned int i = 0; i < p_entry.totalEntries; i++ )
    {
        if ( p_entry.ID[i] == p_prim )
        {
            return true;
        }
    }

    return false;
}
template<typename T>
bool smHash<T>::findandUpdateEntry( smEntryList< T > &p_startEntry, T &p_prim )
{
    smEntryList<T> *currentBucket = &p_startEntry;

    while ( true )
    {
        if ( currentBucket == NULL )
        {
            return false;
        }

        for ( int i = 0; i < currentBucket->totalEntries; i++ )
        {
            if ( currentBucket->ID[i] == p_prim )
            {
                currentBucket->ID[i] = p_prim;
                return true;
            }
        }

        currentBucket = currentBucket->p_entry;
    }
}
template<typename T>
void smHash<T>::clearBuckets( smEntryList< T > &p_startEntry )
{
    smEntryList<T> *currentBucket = &p_startEntry;

    while ( true )
    {
        if ( currentBucket == NULL )
        {
            return;
        }

        currentBucket->totalEntries = 0;
        currentBucket = currentBucket->p_entry;
    }
}
template<typename T>
smHash<T>::smHash( int p_tableSize )
{
    tableSize = p_tableSize;
    primitiveIDs = new smEntryList<T>[tableSize];
    currentEntryIndex = 0;
    currentTableIndex = 0;
    num_prim = 0;
}
template<typename T>
SIMMEDTK_HASHRETURN_CODES smHash<T>::insert( T p_triangle, unsigned int hashIndex )
{
    smEntryList<T> *currentBucket;
    smEntryList<T> *prevBucket;
    smEntryList<T> *emptySpace = NULL;
    prevBucket = &primitiveIDs[hashIndex];
    currentBucket = &primitiveIDs[hashIndex];

    while ( true )
    {
        if ( currentBucket == NULL )
        {
            break;
        }

        if ( emptySpace == NULL && currentBucket->totalEntries < SIMMEDTK_HASHBUCKET_SIZE )
        {
            emptySpace = currentBucket;
            break;
        }

        prevBucket = currentBucket;
        currentBucket = currentBucket->p_entry;
    }

    if ( currentBucket == NULL && emptySpace == NULL )
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
template<typename T>
SIMMEDTK_HASHRETURN_CODES smHash<T>::checkAndInsert( T p_triangle, unsigned int hashIndex )
{
    smEntryList<T> *currentBucket;
    smEntryList<T> *prevBucket;
    smEntryList<T> *emptySpace = NULL;
    prevBucket = &primitiveIDs[hashIndex];
    currentBucket = &primitiveIDs[hashIndex];

    while ( true )
    {
        if ( currentBucket == NULL )
        {
            break;
        }

        if ( checkIdentical( *currentBucket, p_triangle ) )
        {
            return SIMMEDTK_HASH_ENTRYALREADYEXISTS;
        }

        if ( emptySpace == NULL && currentBucket->totalEntries < SIMMEDTK_HASHBUCKET_SIZE )
        {
            emptySpace = currentBucket;
            break;
        }

        prevBucket = currentBucket;
        currentBucket = currentBucket->p_entry;
    }

    if ( currentBucket == NULL && emptySpace == NULL )
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
template<typename T>
void smHash<T>::startIteration()
{
    currentTableIndex = 0;
    currentEntryIndex = 0;
    currentIterationBucket = &primitiveIDs[currentTableIndex];
}
template<typename T>
bool smHash<T>::next( smHashIterator< T > &p_iterator )
{
    if ( p_iterator.tableIndex >= tableSize )
    {
        return false;
    }

    p_iterator.iterator = &primitiveIDs[p_iterator.tableIndex];
    p_iterator.bucketStart = &primitiveIDs[p_iterator.tableIndex];
    p_iterator.currentIndex = 0;
    p_iterator.tableIndex++;
    return true;
}
template<typename T>
bool smHash<T>::nextBucketItem( smHashIterator< T > &p_iterator, T &p_prim )
{
    while ( true )
    {
        if ( p_iterator.iterator == NULL )
        {
            p_iterator.currentIndex = 0;
            return false;
        }

        if ( p_iterator.iterator->totalEntries > 0 && p_iterator.iterator->totalEntries > p_iterator.currentIndex )
        {
            p_prim = p_iterator.iterator->ID[p_iterator.currentIndex];
            p_iterator.currentIndex++;
            return true;
        }

        if ( p_iterator.iterator->totalEntries == 0 || p_iterator.currentIndex >= p_iterator.iterator->totalEntries )
        {

            p_iterator.iterator = p_iterator.iterator->p_entry;
            p_iterator.currentIndex = 0;
            continue;
        }
    }
}
template<typename T>
bool smHash<T>::next( T &p_prim )
{
    while ( true )
    {
        if ( currentTableIndex >= tableSize )
        {
            return false;
        }

        if ( currentIterationBucket == NULL )
        {
            currentTableIndex++;
            currentEntryIndex = 0;
            currentIterationBucket = &primitiveIDs[currentTableIndex];
            continue;
        }

        if ( currentIterationBucket->totalEntries == 0 || currentEntryIndex >= currentIterationBucket->totalEntries )
        {
            currentIterationBucket = currentIterationBucket->p_entry;
            currentEntryIndex = 0;
            continue;
        }

        if ( currentIterationBucket->totalEntries > 0 && currentIterationBucket->totalEntries > currentEntryIndex )
        {
            p_prim = currentIterationBucket->ID[currentEntryIndex];
            currentEntryIndex++;
            return true;
        }

        currentTableIndex++;
    }
}
template<typename T>
bool smHash<T>::nextByRef( T **p_prim )
{
    while ( true )
    {
        if ( currentTableIndex >= tableSize )
        {
            return false;
        }

        if ( currentIterationBucket == NULL )
        {
            currentTableIndex++;
            currentEntryIndex = 0;
            currentIterationBucket = &primitiveIDs[currentTableIndex];
            continue;
        }

        if ( currentIterationBucket->totalEntries == 0 || currentEntryIndex >= currentIterationBucket->totalEntries )
        {
            currentIterationBucket = currentIterationBucket->p_entry;
            currentEntryIndex = 0;
            continue;
        }

        if ( currentIterationBucket->totalEntries > 0 && currentIterationBucket->totalEntries > currentEntryIndex )
        {
            *p_prim = &currentIterationBucket->ID[currentEntryIndex];
            currentEntryIndex++;
            return true;
        }

        currentTableIndex++;
    }
}
template<typename T>
void smHash<T>::printContent()
{
    T prim;
    startIteration();

    while ( next( prim ) )
    {
        std::cout << "Table:" << currentTableIndex << " Bucket Index:" << currentEntryIndex << " Prim:" << prim << "\n";
    }
}
template<typename T>
void smHash<T>::clearAll()
{
    for ( int i = 0; i < tableSize; i++ )
    {
        clearBuckets( primitiveIDs[i] );
    }

    num_prim = 0;
}
template<typename T>
smStorageSlidingWindow<T>::~smStorageSlidingWindow()
{
    delete[] data;
}
template<typename T>
smStorageSlidingWindow<T>::smStorageSlidingWindow( int p_windowSize, smStorageSlideType p_type )
{
    data = new T[p_windowSize];
    windowSize = p_windowSize;
    memset( data, 0, sizeof( T )*p_windowSize );
    strorageType = p_type;
}
template<typename T>
smStorageSlideType smStorageSlidingWindow<T>::getStorageType()
{
    return strorageType;
}
template<typename T>
void smStorageSlidingWindow<T>::resize( int p_newSize )
{
    T *tempData;
    int index;

    if ( windowSize == p_newSize )
    {
        return;
    }

    tempData = new T[p_newSize];
    memset( tempData, 0, sizeof( T )*p_newSize );

    if ( strorageType == SIMMEDTK_STORAGESLIDING_LASTFIRST )
    {
        if ( p_newSize > windowSize )
        {
            memcpy( tempData + p_newSize - windowSize, data, windowSize * sizeof( T ) );
        }
        else
        {
            memcpy( tempData, data + p_newSize, p_newSize * sizeof( T ) );
        }
    }
    else
    {
        if ( p_newSize > windowSize )
        {
            memcpy( tempData, data, windowSize * sizeof( T ) );
        }
        else
        {
            memcpy( tempData, data, p_newSize * sizeof( T ) );
        }
    }

    delete[] data;
    data = tempData;
    windowSize = p_newSize;
}
template<typename T>
void smStorageSlidingWindow<T>::add( T p_value )
{
    if ( strorageType == SIMMEDTK_STORAGESLIDING_LASTFIRST )
    {
        memcpy( data, &data[1], ( windowSize - 1 )*sizeof( T ) );
        data[windowSize - 1] = p_value;
    }
    else
    {
        for ( int i = windowSize - 1; i > 0; i-- )
        {
            data[i] = data[i - 1];
        }

        data[0] = p_value;
    }
}
template<typename T>
void smStorageSlidingWindow<T>::zeroed()
{
    memset( data, 0, sizeof( T )*windowSize );
}
template<typename T>
void smStorageSlidingWindow<T>::print() const
{
    for ( int i = 0; i < windowSize; i++ )
    {
        std::cout << data[i] << " ";
    }

    std::cout << "\n";
}

#endif
