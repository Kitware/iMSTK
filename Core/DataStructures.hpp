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
EntryList<T>::EntryList()
{
    totalEntries = 0;
    p_entry = NULL;
}
template<typename T>
HashIterator<T>::HashIterator()
{
    tableIndex = 0;
    currentIndex = 0;
    bucketStart = NULL;
}
template<typename T> void
HashIterator<T>::clone( HashIterator<T> &p_iterator )
{
    tableIndex = p_iterator.tableIndex;
    currentIndex = p_iterator.currentIndex;
    iterator = p_iterator.iterator;
}

template<typename T> void
HashIterator<T>::resetBucketIteration()
{
    currentIndex = 0;
    iterator = bucketStart;
}
template<typename T>
std::ostream &operator<<( std::ostream &out, HashIterator<T> &p )
{
    out << "Table Index:" << p.tableIndex << " Current Bucket Index:" << p.currentIndex;
    return out;
}
template<typename T>
void Hash<T>::moveEntriesInBucket( EntryList<T> &p_Bucket, int p_entryIndex )
{
    for ( unsigned int i = p_entryIndex; i < p_Bucket.totalEntries - 1; i++ )
    {
        p_Bucket.ID[i] = p_Bucket.ID[i + 1];
    }

    p_Bucket.totalEntries--;
}
template<typename T>
bool Hash<T>::checkIdentical( EntryList<T> &p_entry, T p_prim )
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
bool Hash<T>::findandUpdateEntry( EntryList<T> &p_startEntry, T &p_prim )
{
    EntryList<T> *currentBucket = &p_startEntry;

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
void Hash<T>::clearBuckets( EntryList<T> &p_startEntry )
{
    EntryList<T> *currentBucket = &p_startEntry;

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
Hash<T>::Hash( int p_tableSize )
{
    tableSize = p_tableSize;
    primitiveIDs = new EntryList<T>[tableSize];
    currentEntryIndex = 0;
    currentTableIndex = 0;
    num_prim = 0;
}
template<typename T>
SIMMEDTK_HASHRETURN_CODES Hash<T>::insert( T p_triangle, unsigned int hashIndex )
{
    EntryList<T> *currentBucket;
    EntryList<T> *prevBucket;
    EntryList<T> *emptySpace = NULL;
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
        prevBucket->p_entry = new EntryList<T>;
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
SIMMEDTK_HASHRETURN_CODES Hash<T>::checkAndInsert( T p_triangle, unsigned int hashIndex )
{
    EntryList<T> *currentBucket;
    EntryList<T> *prevBucket;
    EntryList<T> *emptySpace = NULL;
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
        prevBucket->p_entry = new EntryList<T>;
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
void Hash<T>::startIteration()
{
    currentTableIndex = 0;
    currentEntryIndex = 0;
    currentIterationBucket = &primitiveIDs[currentTableIndex];
}
template<typename T>
bool Hash<T>::next( HashIterator<T> &p_iterator )
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
bool Hash<T>::nextBucketItem( HashIterator<T> &p_iterator, T &p_prim )
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
bool Hash<T>::next( T &p_prim )
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
bool Hash<T>::nextByRef( T **p_prim )
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
void Hash<T>::printContent()
{
    T prim;
    startIteration();

    while ( next( prim ) )
    {
        std::cout << "Table:" << currentTableIndex << " Bucket Index:" << currentEntryIndex << " Prim:" << prim << "\n";
    }
}
template<typename T>
void Hash<T>::clearAll()
{
    for ( int i = 0; i < tableSize; i++ )
    {
        clearBuckets( primitiveIDs[i] );
    }

    num_prim = 0;
}

#endif
