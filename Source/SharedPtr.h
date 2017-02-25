#pragma once
#include <memory>

template <typename T>
class SharedPtr : public std::shared_ptr<T>
{
public:
	// Constructor taking a specific deleter function to be called on destruction
	SharedPtr(void(deleter)(T* ptr)) : std::shared_ptr<T>(nullptr, deleter) {};
};

template <typename T>
class SharedDeletePtr : public SharedPtr<T>
{
public:
	// Default constructor, sets up destructor
	SharedDeletePtr() : SharedPtr<T>([](T* ptr) { delete ptr; }) {}

	// Construct with a pointer
	SharedDeletePtr(T* ptr) : SharedPtr<T>([](T* ptr) { delete ptr; }) { reset(ptr); }
};