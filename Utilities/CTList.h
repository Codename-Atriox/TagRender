#pragma once
#include <stdint.h>
#include <exception>
#include <vcruntime_string.h> // why is this required for memcpy??

// NOTE: this list just grabs the ptr to whatever you add, so you do not need to do any of that cleanup stuff

// we're just going to have to assume that we cant use ptrs as types, or else we're going to apply delete when we should be using delete[]
template <typename T>
class CTList {
public:
	CTList()
	{
		// do nothing
	}
	CTList(uint64_t alloc_size) {
		Alloc(alloc_size);
	}
	~CTList() {
		ClearBuffer(); // call destructors of contained content
	}
	T* operator[] (uint64_t index) {
		if (index >= count)
			throw std::exception("out of bounds CTList index");
		return content_ptr[index];
	}
	uint64_t Size() {
		return count;
	}

	void Append(T* item) {
		// test whether the current buffer fits everything
		if (allocated_count < count + 1)
			Alloc(block_alloc_count);

		write_index(count, item);
		count++;
	}
	void Insert(T* item, uint64_t index){
		if (index >= count){ // if its not in the array, then push to back
			Append(item);
			return;}
		// test whether the current buffer fits everything
		if (allocated_count < count + 1)
			Alloc(block_alloc_count);

		ShiftForward(index, item);
	}
	void RemoveAt(uint64_t index) {
		if (index >= count)
			throw std::exception("out of bounds CList removal index");
		//ShiftBack(index);
		QuickRemove(index); // this should be much more efficient with managing many simutaneous assets, although this would only be usefuly in the scenario that we have a very large list
	}
	void Alloc(uint64_t extra_size) {
		allocated_count += extra_size;
		if (allocated_count < extra_size)
			throw std::exception("CTList buffer size overflow (size > uint64_MAX)"); // virtually impossible

		T** new_array = (T**)new void* [allocated_count];

		if (content_ptr != 0) // do not copy from buffer if its empty, aka first initialization
			memcpy(new_array, content_ptr, (size_t)count); // NOTE: count is converted to longlong, meaning our highest item index is a signed long long;
		ClearBuffer();
		content_ptr = new_array;
	}
	void ClearBuffer() {
		if (content_ptr == 0) return; // nothing to delete
		// call destructor of each object
		for (uint64_t i = 0; i < count; i++)
			delete content_ptr[i];
		// then delete the array
		delete[] content_ptr;
		content_ptr = 0;
	}
private:
	void write_index(uint64_t index, T* ptr_to_insert) {
		content_ptr[index] = ptr_to_insert;
	}
	void ShiftForward(uint64_t index, T* item) { // we have to ensure we have the room before calling this
		// start from the end of the array
		for (uint64_t i = count; i > index; i--)
			write_index(i, content_ptr[i-1]);
		write_index(index, item);
		count++;
	}
	void ShiftBack(uint64_t index) {
		// make sure we call the destructor of the one we're removing
		delete content_ptr[index];
		// start from the start of the array
		for (uint64_t i = index; i < count-1; i++)
			write_index(i, content_ptr[i+1]);
		write_index(count-1, (T*)0);
		count--; // since we cleared the last one
	}
	void QuickRemove(uint64_t index) {
		delete content_ptr[index]; // call destructor
		write_index(index, content_ptr[count - 1]);
		write_index(count - 1, nullptr);
		count--; // since we cleared the last one
	}
private:
	T** content_ptr = 0;
	uint64_t count = 0;
	uint64_t allocated_count = 0;
	const static uint64_t block_alloc_count = 8; // thats 64 byte blocks
};

