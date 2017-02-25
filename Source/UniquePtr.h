#pragma once
#include <memory>

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
	// Default constructor, sets up destructor
	UniqueReleasePtr() : UniquePtr<T>([](T* ptr) { ptr->Release(); }) {}

	// Construct with a pointer
	UniqueReleasePtr(T* ptr) : UniquePtr<T>([](T* ptr) { ptr->Release(); }) { reset(ptr); }
};

template <typename T>
class UniqueFreePtr : public UniquePtr<T>
{
public:
	// Default constructor, sets up destructor
	UniqueFreePtr() : UniquePtr<T>([](T* ptr) { free(ptr); }) {};
};