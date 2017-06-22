#pragma once
#include <cstring>

namespace PanzerJson
{

// Add some inline methods for builds without link-time optimization.

inline ValueBase::Type Value::GetType() const noexcept
{
	return value_->type;
}

inline bool Value::IsNull() const noexcept
{
	return GetType() == ValueBase::Type::Null;
}

inline bool Value::IsObject() const noexcept
{
	return GetType() == ValueBase::Type::Object;
}

inline bool Value::IsArray() const noexcept
{
	return GetType() == ValueBase::Type::Array;
}

inline bool Value::IsString() const noexcept
{
	return GetType() == ValueBase::Type::String;
}

inline bool Value::IsNumber() const noexcept
{
	return GetType() == ValueBase::Type::Number;
}

inline bool Value::IsBool() const noexcept
{
	return GetType() == ValueBase::Type::Bool;
}

inline Value::UniversalIterator Value::cbegin() const noexcept
{
	return begin();
}

inline Value::UniversalIterator Value::cend() const noexcept
{
	return end();
}

inline Value::ArrayIterator Value::array_cbegin() const noexcept
{
	return array_begin();
}

inline Value::ArrayIterator Value::array_cend() const noexcept
{
	return array_end();
}

inline Value::ObjectIterator Value::object_cbegin() const noexcept
{
	return object_begin();
}

inline Value::ObjectIterator Value::object_cend() const noexcept
{
	return object_end();
}

inline const ValueBase* Value::GetInternalValue() const noexcept
{
	return value_;
}

// UniversalIterator

inline Value::UniversalIterator::UniversalIterator( const ValueBase::Type type, const Ptr ptr ) noexcept
	: type_(type)
	, ptr_(ptr)
{
	// TODO - assert, if type is not array or object.
}

inline bool Value::UniversalIterator::operator==( const UniversalIterator& other ) const noexcept
{
	return type_ == other.type_ && std::memcmp( &ptr_, &other.ptr_, sizeof(Ptr) ) == 0;
}

inline bool Value::UniversalIterator::operator!=( const UniversalIterator& other ) const noexcept
{
	return !( *this == other );
}

inline Value::UniversalIterator& Value::UniversalIterator::operator++() noexcept
{
	if( type_ == ValueBase::Type::Array )
		++ptr_.array_value;
	else
		++ptr_.object_entry;
	return *this;
}

inline Value::UniversalIterator& Value::UniversalIterator::operator--() noexcept
{
	if( type_ == ValueBase::Type::Array )
		--ptr_.array_value;
	else
		--ptr_.object_entry;
	return *this;
}

inline Value::UniversalIterator Value::UniversalIterator::operator++(int) noexcept
{
	UniversalIterator result= *this;
	++*this;
	return result;
}

inline Value::UniversalIterator Value::UniversalIterator::operator--(int) noexcept
{
	UniversalIterator result= *this;
	--*this;
	return result;
}

inline Value Value::UniversalIterator::operator*() const noexcept
{
	if( type_ == ValueBase::Type::Array )
		return Value( *ptr_.array_value );
	else
		return Value( ptr_.object_entry->value );
}

// ArrayIterator

inline Value::ArrayIterator::ArrayIterator( const ValueBase* const* const ptr ) noexcept
	: ptr_(ptr)
{}

inline bool Value::ArrayIterator::operator==( const ArrayIterator& other ) const noexcept
{
	return ptr_ == other.ptr_;
}

inline bool Value::ArrayIterator::operator!=( const ArrayIterator& other ) const noexcept
{
	return !( *this == other );
}

inline Value::ArrayIterator& Value::ArrayIterator::operator++() noexcept
{
	++ptr_;
	return *this;
}

inline Value::ArrayIterator& Value::ArrayIterator::operator--() noexcept
{
	--ptr_;
	return *this;
}

inline Value::ArrayIterator Value::ArrayIterator::operator++(int) noexcept
{
	ArrayIterator result= *this;
	++*this;
	return result;
}

inline Value::ArrayIterator Value::ArrayIterator::operator--(int) noexcept
{
	ArrayIterator result= *this;
	--*this;
	return result;
}

inline Value Value::ArrayIterator::operator*() const noexcept
{
	return Value( *ptr_ );
}

// ObjectIterator

inline Value::ObjectIterator::ObjectIterator( const ObjectValue::ObjectEntry* const ptr ) noexcept
	: ptr_(ptr)
{}

inline bool Value::ObjectIterator::operator==( const ObjectIterator& other ) const noexcept
{
	return ptr_ == other.ptr_;
}

inline bool Value::ObjectIterator::operator!=( const ObjectIterator& other ) const noexcept
{
	return !( *this == other );
}

inline Value::ObjectIterator& Value::ObjectIterator::operator++() noexcept
{
	++ptr_;
	return *this;
}

inline Value::ObjectIterator& Value::ObjectIterator::operator--() noexcept
{
	--ptr_;
	return *this;
}

inline Value::ObjectIterator Value::ObjectIterator::operator++(int) noexcept
{
	ObjectIterator result= *this;
	++*this;
	return result;
}

inline Value::ObjectIterator Value::ObjectIterator::operator--(int) noexcept
{
	ObjectIterator result= *this;
	--*this;
	return result;
}

inline Value::ObjectIterator::value_type Value::ObjectIterator::operator*() const noexcept
{
	return value_type( ptr_->key, Value( ptr_->value ) );
}

// IteratorRange

template<class Iterator>
Value::IteratorRange<Iterator>::IteratorRange( const Iterator begin, const Iterator end ) noexcept
	: begin_(begin), end_(end)
{
}

template<class Iterator>
Iterator Value::IteratorRange<Iterator>::begin() const noexcept
{
	return begin_;
}

template<class Iterator>
Iterator Value::IteratorRange<Iterator>::end() const noexcept
{
	return end_;
}

} // namespace PanzerJson
