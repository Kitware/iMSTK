/*
****************************************************
SIMMEDTK LICENSE

****************************************************
*/


#ifndef SMDATASTRUTCS_H
#define SMDATASTRUTCS_H
#include "smCore/smConfig.h"

template<typename T>
class smIndiceArrayIter;

template<typename T>
class smIndiceArray{
protected:
	T *storage;
	smBool *isEmpty;
	smInt *indices;
	smInt nbrElements;
	smInt maxStorage;
public:
	~smIndiceArray(){
		delete [] indices;
		delete [] storage;
		delete [] isEmpty;
	}

	inline smIndiceArray(smInt p_maxStorage){
		maxStorage=p_maxStorage;
		indices=new smInt[p_maxStorage];
		storage=new T[p_maxStorage];
		isEmpty=new smBool[p_maxStorage];
		for(smInt i=0;i<p_maxStorage;i++)
			isEmpty[i]=true;
		nbrElements=0;
	}

	inline smInt add(T p_item){
		smInt index=-1;
		if(nbrElements<maxStorage){
			for(smInt i=0;i<maxStorage;i++){
				if(isEmpty[i]){
					index=i;
					break;
				}
			}
			if(index>=0&&index<maxStorage){
				isEmpty[index]=false;
				storage[index]=p_item;
				indices[nbrElements]=index;
				nbrElements++;
				return index;
			}
		}
		return index;
	}

	inline smInt checkAndAdd(T p_item){
		smInt index=-1;
		if(nbrElements<maxStorage){
			for(smInt i=0;i<nbrElements;i++)
				if(storage[indices[i]]==p_item)
					break;
			return add(p_item);
		}
		return index;
	}

	inline smBool remove(smInt p_itemIndex){
		smInt counter=0;
		if(p_itemIndex>=0&&p_itemIndex<maxStorage){
			for(smInt i=0;i<nbrElements;i++){
				if(i==p_itemIndex)
					continue;
				indices[counter]=indices[i];
				counter++;
			}
			nbrElements--;
			isEmpty[p_itemIndex]=true;
			return true;
		}
		else
			return false;
	}

	inline smBool replace(smInt p_index,T &p_item){
		if(isEmpty[p_index]==false){
			storage[p_index]=p_item;
			return true;
		}
		return false;
	}

	inline T& getByRef(smInt p_index){
		return storage[p_index];
	}

	inline smBool getByRefSafe(smInt p_index, T&p_item){
		if(isEmpty[p_index])
			return false;
		else
		{
			p_item=storage[p_index];
			return true;
		}
	}

	inline T& getByRef(QString p_string){
		for(smInt i=0;i<nbrElements;i++){
			if(storage[indices[i]]==p_string){
				return storage[indices[i]];
			}
		}
	}

	friend smIndiceArrayIter<T>;

	inline void print(){
		for(smInt i=0;i<nbrElements;i++){
			storage[indices[i]]->print();
		}
	}

	inline T& operator[](smInt p_index){
		return storage[indices[p_index]];
	}

	inline smInt size(){
		return nbrElements;
	}

	inline smBool copy(smIndiceArray &p_array){
		if(maxStorage<p_array.maxStorage){
			for(smInt j=0;j<maxStorage;j++)
				isEmpty[j]=true;
			for(smInt i=0;i<p_array.nbrElements;i++){
				storage[indices[i]]=p_array.storage[p_array.indices[i]];
				indices[i]=p_array.indices[i];
				isEmpty[i]=p_array.isEmpty[p_array.indices[i]];
			}
			return true;
		}
		return false;
	}
};

template<class T>
class smIndiceArrayIter{
	smInt index;
	smIndiceArray<T> *arrayPtr;
public:
	smIndiceArrayIter(smIndiceArray<T> *p_array){
		arrayPtr=p_array;
	}

	T& operator[](smInt p_index){
		return arrayPtr->storage[ arrayPtr->indices[p_index]];
	}

	smInt& operator++(){//prefix
		return ++index;
	}

	smInt operator++(const int) {//postfix
		return index++;
	}

	smInt& operator--(){//prefix
		return --index;
	}

	smInt operator--(const int) {//postfix
		return index--;
	}

	inline smInt begin(){
		index=0;
		return index;
	}

	inline smInt end(){
		return arrayPtr->nbrElements;
	}
};

#define SOFMIS_HASHBUCKET_SIZE	10

template <class T>
struct smEntryList{
public:
	smEntryList(){
		totalEntries=0;
		p_entry=NULL;
	}

	T ID[SOFMIS_HASHBUCKET_SIZE];
	smUInt totalEntries;
	smEntryList* p_entry;
};

template <class T>
struct smHashIterator{
	smHashIterator(){
		tableIndex=0;
		currentIndex=0;
		bucketStart=NULL;
	}

	inline void clone(smHashIterator<T> &p_iterator){
		tableIndex=p_iterator.tableIndex;
		currentIndex=p_iterator.currentIndex;
		iterator=p_iterator.iterator;
	}

	template<typename K>
	inline void clone(smHashIterator<K> &p_iterator){
		tableIndex=p_iterator.tableIndex;
		currentIndex=p_iterator.currentIndex;
		
	}

	inline void resetBucketIteration(){
		currentIndex=0;
		iterator=bucketStart;
	}

	smEntryList<T> *iterator;
	smEntryList<T> *bucketStart;
	smInt tableIndex;
	smInt currentIndex;

	friend ostream &operator<<( ostream &out, smHashIterator<T> &p ) {
		out<<"Table Index:"<<p.tableIndex<< " Current Bucket Index:"<<p.currentIndex;
		return out;
	}
};

enum SOFMIS_HASHRETURN_CODES{
	SOFMIS_HASH_ENTRYREMOVED,
	SOFMIS_HASH_ENTRYALREADYEXISTS,
	SOFMIS_HASH_ENTRYOTEXIST,
	SOFMIS_HASH_NOTBUCKETS,
	SOFMIS_HASH_ENTRYINSERTED,
	SOFMIS_HASH_ALLOCATED_INSERTED,
	SOFMIS_HASH_SUCCESS
};

template <class T>
class smHash{
	smEntryList<T> *primitiveIDs;
	smLongInt currentTableIndex;
	smLongInt currentEntryIndex;
	smEntryList<T> *currentIterationBucket;
public:
	smLongInt num_prim;
	smLongInt tableSize;
private:
	inline void moveEntriesInBucket(smEntryList<T> &p_Bucket,smInt p_entryIndex ){
		for(unsigned smInt i=p_entryIndex;i<p_Bucket.totalEntries-1;i++)
			p_Bucket.ID[i]=p_Bucket.ID[i+1];
		p_Bucket.totalEntries--;
	}

	inline bool checkIdentical(smEntryList<T> &p_entry,T p_prim){
		for(unsigned smInt i =0;i<p_entry.totalEntries;i++){
			if(p_entry.ID[i]==p_prim)
				return true;
		}
		return false;
	}

	inline bool findandUpdateEntry(smEntryList<T> &p_startEntry,T &p_prim){
		smEntryList<T> *currentBucket=&p_startEntry;
		while(true){
			if(currentBucket==NULL)
				return false;
			for(smInt i=0;i<currentBucket->totalEntries;i++){
				if(currentBucket->ID[i]==p_prim){
					currentBucket->ID[i]=p_prim;
					return true;
				}
			}
			currentBucket=currentBucket->p_entry;
		}
	}

	inline void clearBuckets(smEntryList<T> &p_startEntry){
		smEntryList<T> *currentBucket=&p_startEntry;
		while(true){
			if(currentBucket==NULL)
				return;
			currentBucket->totalEntries=0;
			currentBucket=currentBucket->p_entry;
		}
	}

public:
	smHash(smInt p_tableSize){
		tableSize=p_tableSize;
		primitiveIDs=new smEntryList<T>[tableSize];
		currentEntryIndex=0;
		currentTableIndex=0;
		num_prim=0;
	}

	inline SOFMIS_HASHRETURN_CODES insert(T p_triangle,smUInt hashIndex){
		smEntryList<T> *currentBucket;
		smEntryList<T> *prevBucket;
		smEntryList<T> *emptySpace=NULL;
		prevBucket=&primitiveIDs[hashIndex];
		currentBucket=&primitiveIDs[hashIndex];

		while(true){
			if(currentBucket==NULL)
				break;
			if(emptySpace==NULL&&currentBucket->totalEntries<SOFMIS_HASHBUCKET_SIZE){
				emptySpace=currentBucket;
				break;
			}

			prevBucket=currentBucket;
			currentBucket=currentBucket->p_entry;
		}

		if(currentBucket==NULL&&emptySpace==NULL){
			prevBucket->p_entry=new smEntryList<T>;
			prevBucket->p_entry->ID[prevBucket->p_entry->totalEntries]=p_triangle;
			prevBucket->p_entry->totalEntries++;
			num_prim++;
			return  SOFMIS_HASH_ALLOCATED_INSERTED;
		}
		else {
			emptySpace->ID[emptySpace->totalEntries]=p_triangle;
			emptySpace->totalEntries++;
		}

		num_prim++;
		return SOFMIS_HASH_ENTRYINSERTED;
	}

	inline SOFMIS_HASHRETURN_CODES checkAndInsert(T p_triangle,smUInt hashIndex){
		smEntryList<T> *currentBucket;
		smEntryList<T> *prevBucket;
		smEntryList<T> *emptySpace=NULL;
		prevBucket=&primitiveIDs[hashIndex];
		currentBucket=&primitiveIDs[hashIndex];

		while(true){
			if(currentBucket==NULL)
				break;

			if(checkIdentical(*currentBucket,p_triangle)){
				return SOFMIS_HASH_ENTRYALREADYEXISTS;
			}

			if(emptySpace==NULL&&currentBucket->totalEntries<SOFMIS_HASHBUCKET_SIZE){
				emptySpace=currentBucket;
				break;
			}

			prevBucket=currentBucket;
			currentBucket=currentBucket->p_entry;
		}

		if(currentBucket==NULL&&emptySpace==NULL){
			prevBucket->p_entry=new smEntryList<T>;
			prevBucket->p_entry->ID[prevBucket->p_entry->totalEntries]=p_triangle;
			prevBucket->p_entry->totalEntries++;
			num_prim++;
			return SOFMIS_HASH_ALLOCATED_INSERTED;
		}
		else {
			emptySpace->ID[emptySpace->totalEntries]=p_triangle;
			emptySpace->totalEntries++;
		}

		num_prim++;
		return SOFMIS_HASH_ENTRYINSERTED;
	}

	inline void startIteration(){
		currentTableIndex=0;
		currentEntryIndex=0;
		currentIterationBucket=&primitiveIDs[currentTableIndex];
	}

	inline bool next(smHashIterator<T> &p_iterator){
		if(p_iterator.tableIndex>=tableSize)
			return false;

		p_iterator.iterator=&primitiveIDs[p_iterator.tableIndex];
		p_iterator.bucketStart=&primitiveIDs[p_iterator.tableIndex];
		p_iterator.currentIndex=0;
		p_iterator.tableIndex++;
		return true;
	}

	inline bool nextBucketItem(smHashIterator<T> &p_iterator,T &p_prim){
		while(true){
			if(p_iterator.iterator==NULL){
				p_iterator.currentIndex=0;
				return false;
			}

			if(p_iterator.iterator->totalEntries>0&&p_iterator.iterator->totalEntries>p_iterator.currentIndex){
				p_prim= p_iterator.iterator->ID[p_iterator.currentIndex];
				p_iterator.currentIndex++;
				return true;
			}

			if(p_iterator.iterator->totalEntries==0||p_iterator.currentIndex>=p_iterator.iterator->totalEntries){

				p_iterator.iterator=p_iterator.iterator->p_entry;
				p_iterator.currentIndex=0;
				continue;
			}
		}
	}

	inline bool next(T &p_prim){
		while(true){
			if(currentTableIndex>=tableSize)
				return false;

			if(currentIterationBucket==NULL){
				currentTableIndex++;
				currentEntryIndex=0;
				currentIterationBucket=&primitiveIDs[currentTableIndex];
				continue;
			}

			if(currentIterationBucket->totalEntries==0||currentEntryIndex>=currentIterationBucket->totalEntries){
				currentIterationBucket=currentIterationBucket->p_entry;
				currentEntryIndex=0;
				continue;
			}

			if(currentIterationBucket->totalEntries>0&&currentIterationBucket->totalEntries>currentEntryIndex){
				p_prim= currentIterationBucket->ID[currentEntryIndex];
				currentEntryIndex++;
				return true;
			}

			currentTableIndex++;
		}
	}

	inline smBool nextByRef(T **p_prim){
		while(true){
			if(currentTableIndex>=tableSize)
				return false;

			if(currentIterationBucket==NULL){
				currentTableIndex++;
				currentEntryIndex=0;
				currentIterationBucket=&primitiveIDs[currentTableIndex];
				continue;
			}

			if(currentIterationBucket->totalEntries==0||currentEntryIndex>=currentIterationBucket->totalEntries){
				currentIterationBucket=currentIterationBucket->p_entry;
				currentEntryIndex=0;
				continue;
			}

			if(currentIterationBucket->totalEntries>0&&currentIterationBucket->totalEntries>currentEntryIndex){
				*p_prim= &currentIterationBucket->ID[currentEntryIndex];
				currentEntryIndex++;
				return true;
			}

			currentTableIndex++;
		}
	}

	inline void printContent(){
		T prim;
		startIteration();
		while(next(prim)){
			cout<<"Table:"<<currentTableIndex<<" Bucket Index:"<<currentEntryIndex<<" Prim:"<<prim<<endl;
		}
	}

	void clearAll(){
		for(smInt i=0;i<tableSize;i++){
			clearBuckets(primitiveIDs[i]);
		}
		num_prim=0;
	}
};

enum smStorageSlideType{
	SOFMIS_STORAGESLIDING_FRONTFIRST,
	SOFMIS_STORAGESLIDING_LASTFIRST
};

template<typename T>
class smStorageSlidingWindow{
private:
	smStorageSlideType strorageType;
public:
	T *data;
	unsigned int windowSize;

	~smStorageSlidingWindow(){
		delete[] data;
	}

	smStorageSlidingWindow(int p_windowSize=10,smStorageSlideType p_type=SOFMIS_STORAGESLIDING_LASTFIRST){
		data=new T[p_windowSize];
		windowSize=p_windowSize;
		memset(data,0,sizeof(T)*p_windowSize);
		strorageType=p_type;
	}

	inline smStorageSlideType getStorageType(){
		return strorageType;
	}

	inline void resize(int p_newSize){
		T*tempData;
		int index;

		if(windowSize==p_newSize)
			return;

		tempData=new T[p_newSize];
		memset(tempData,0,sizeof(T)*p_newSize);

		if(strorageType==SOFMIS_STORAGESLIDING_LASTFIRST){
			if(p_newSize>windowSize)
				memcpy(tempData+p_newSize-windowSize,data,windowSize*sizeof(T));
			else
				memcpy(tempData,data+p_newSize,p_newSize*sizeof(T));
		}
		else{
			if(p_newSize>windowSize)
				memcpy(tempData,data,windowSize*sizeof(T));
			else
				memcpy(tempData,data,p_newSize*sizeof(T));
		}

		delete[] data;
		data=tempData;
		windowSize=p_newSize;
	}

	inline void add(T p_value){
		if(strorageType==SOFMIS_STORAGESLIDING_LASTFIRST){
			memcpy(data,&data[1],(windowSize-1)*sizeof(T));
			data[windowSize-1]=p_value;
		}
		else{
			for(int i=windowSize-1;i>0;i--)
				data[i]=data[i-1];

			data[0]=p_value;
		}
	}

	inline void zeroed(){
		memset(data,0,sizeof(T)*windowSize);
	}

	void print(){
		for(int i=0;i<windowSize;i++){
			cout<<data[i]<<" ";
		}
		cout<<endl;
	}
};

#endif
