#pragma once
#include "common.h"

template<class T>
class ObjectPool
{
public:
	// constructor.
	ObjectPool();

	// destructor.
	~ObjectPool();

	// initialize
	bool Initialize(uint size);

	// terminate
	void Terminate();

	// allocate
	T * Allocate();

	// free object
	void Free(uint uid);

	// get object
	T * Get(uint uid);

	void ClearDataMem(uint size);

	// get size
	uint Size() { return size; }

	uint Count() { return count; }

	// to data array
	operator T* () { return data_array; }
	
	// begin
	T * Begin() { return data_array; }

	// end
	T * End() { return data_array + size; }

protected:
	uint count;
	uint size;
	T *  data_array;
	T ** link_array;
	T *  free_obj;
	bool * data_allocate;
};

// construtor
template<class T>
ObjectPool<T>::ObjectPool()
	: count(0)
	, size(0)
	, data_array(NULL)
	, link_array(NULL)
	, free_obj(NULL)
        , data_allocate(NULL)
{
}

// destructor.
template <class T>
ObjectPool<T>::~ObjectPool()
{
	Terminate();
}

// clear mem
template <class T>
void ObjectPool<T>::ClearDataMem(uint size)
{
	if(data_array)
	{
		memset(data_array, '\0', sizeof(T) * size);
	}
}

// initialize
template <class T>
bool ObjectPool<T>::Initialize(uint size)
{
	if (size == 0 || size > 0xffff)
		return false;

	this->size = size;

	// allocate data array
	data_array = new T[size];
	if (data_array == NULL)
		goto err;

	// allocate data allocate
	data_allocate = new bool[size];
	if (data_allocate == NULL)
		goto err;

	// allocate link array
	link_array = new T*[size];
	if (link_array == NULL)
		goto err;

	
	// initialze link
	for (uint i = 0; i < size; i ++)
	{
		data_allocate[i] = false;
		data_array[i].uid = 0x00010000 | i;
		link_array[i] = &data_array[i + 1];
	}
	link_array[size - 1] = NULL;
	free_obj = data_array;

	return true;

err:
	Terminate();
	return false;
}

// terminate
template<class T>
void ObjectPool<T>::Terminate()
{
	if (data_array)
	{
		delete[] data_array;
		data_array = NULL;
	}

	if (data_allocate)
	{
		delete[] data_allocate;
		data_allocate = NULL;
	}

	if (link_array)
	{
		delete[] link_array;
		link_array = NULL;
	}

	free_obj = NULL;
	size = 0;
}

// allocate
template<class T>
T * ObjectPool<T>::Allocate()
{
	T * ret = NULL;

	if (free_obj)
	{
		ret = free_obj;
		free_obj = link_array[ret - data_array];
		link_array[ret - data_array] = NULL;
		data_allocate[ret - data_array] = true;

		count++;
	}

	return ret;
}

	// free object
template<class T>
void ObjectPool<T>::Free(uint uid)
{
	T * obj = Get(uid);

	if (obj)
	{

		if (!data_allocate[obj - data_array])
		{
			//log_write(LOG_ERROR, "ObjectPool dup free uid = %d", uid);
			return;
		}


		link_array[obj - data_array] = free_obj;
		free_obj = obj;

		count--;
		
		// update magic
		ushort magic = uid >> 16;
		magic = magic == 0 ? 1 : magic + 1;
		obj->uid = (magic << 16) | (uid & 0xffff);

		data_allocate[obj - data_array] = false;
	}
}

// get object
template<class T>
T * ObjectPool<T>::Get(uint uid)
{
	uint index = uid & 0xffff;
	if (index < size)
	{
		T * obj = &data_array[index];
		if (obj->uid == uid)
			return obj;
	}

	return NULL;
}

