#pragma once

template <typename T>
class UniquePtr : public std::unique_ptr<T, void(*)(T* ptr)>
{
public:
	// Constructor taking a specific deleter function to be called on destruction
	UniquePtr(void(deleter)(T* ptr)) : std::unique_ptr<T, void(*)(T* ptr)>(nullptr, deleter) {};

	// Returns a pointer to the internal pointer storage.
	T** GetRef() { return reinterpret_cast<T**>(&_Mypair._Get_second()); }
};

template <typename T>
class UniqueReleasePtr : public UniquePtr<T>
{
public:
	// Default constructor, sets up Release destructor
	UniqueReleasePtr() : UniquePtr<T>([](T* ptr) { ptr->Release(); }) {};
};

template <typename T>
class UniqueFreePtr : public UniquePtr<T>
{
public:
	// Default constructor, sets up Release destructor
	UniqueFreePtr() : UniquePtr<T>([](T* ptr) { free(ptr); }) {};
};