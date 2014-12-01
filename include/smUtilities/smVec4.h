#ifndef SMVEC4_H
#define SMVEC4_H

template <typename T>
class smVec4{
public:
	inline smVec4():vec(){
		w=1.0;
	}
	smVec3<T> vec;
	T w;
};

#endif
