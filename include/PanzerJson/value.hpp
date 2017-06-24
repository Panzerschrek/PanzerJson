#pragma once
#include <cstdint>
#include <iterator>
#include <utility>

#include "fwd.hpp"

namespace PanzerJson
{

template<size_t N>
class PaddingHelper
{
public:
	constexpr PaddingHelper() noexcept
		: padding{0u} {}

private:
	unsigned char padding[N];
};

template<>
class PaddingHelper<0u>
{};

// String type is null-terminated UTF-8.
// TODO - maybe add support for width encodings?
// TODO - maybe support string views?
using StringType= const char*;
int StringCompare( const StringType& l, const StringType& r ) noexcept;

struct ValueBase
{
	enum class Type
	{
		Null,
		Object,
		Array,
		String,
		Number,
		Bool,
	};
	Type type;

	constexpr ValueBase( const Type& in_type ) noexcept
		: type(in_type)
	{}
};

struct NullValue final
	: private PaddingHelper< sizeof(void*) - sizeof(ValueBase) >
	, public ValueBase
{
	constexpr NullValue() noexcept
		: ValueBase(Type::Null)
	{}
};

struct ObjectValue final : public ValueBase
{
	struct ObjectEntry final
	{
		StringType key;
		const ValueBase* value;
	};

	uint32_t object_count;

	explicit constexpr ObjectValue( const uint32_t in_object_count ) noexcept
		: ValueBase(Type::Object)
		, object_count(in_object_count)
	{}

	const ObjectEntry* GetEntries() const noexcept
	{
		// Objects stores their members just after it.
		return reinterpret_cast<const ObjectEntry*>(this + 1u);
	}
};

template<size_t N>
struct ObjectValueWithEntriesStorage final
{
	ObjectValue value;
	// WARNING! Entries must be sorted by key!
	ObjectValue::ObjectEntry entries[N];
};

struct ArrayValue final : public ValueBase
{
	uint32_t object_count;

	explicit constexpr ArrayValue( const uint32_t in_object_count ) noexcept
		: ValueBase(Type::Array)
		, object_count(in_object_count)
	{}

	const ValueBase* const* GetElements() const noexcept
	{
		// Arrays stores their elements just after it.
		return reinterpret_cast<const ValueBase* const*>(this + 1u);
	}
};

template<size_t N>
struct ArrayValueWithElementsStorage final
{
	ArrayValue value;
	const ValueBase* elements[N];
};

struct StringValue final
	: private PaddingHelper< sizeof(void*) - sizeof(ValueBase) >
	, public ValueBase
{
	constexpr StringValue() noexcept
		: ValueBase(Type::String)
	{}

	const char* GetString() const noexcept
	{
		// String storage placed just after StringValue.
		return reinterpret_cast<const char*>(this + 1u);
	}
};

template<size_t N>
struct StringValueWithStorage final
{
	StringValue value;
	char string[N]; // Null-terminated
};

struct NumberValue final : public ValueBase
{
	bool has_string;
	int64_t int_value;
	double double_value;

	// Creator (script or parser) must store original str, and correctly convert to int/double values.
	constexpr NumberValue( const int64_t in_int_value, const double in_double_value, const bool in_has_string= false ) noexcept
		: ValueBase(Type::Number)
		, has_string(in_has_string)
		, int_value(in_int_value)
		, double_value(in_double_value)
	{}

	const char* GetString() const noexcept
	{
		// String storage placed just after StringValue.
		return has_string ? reinterpret_cast<const char*>(this + 1u) : "";
	}
};

template<size_t N>
struct NumberValueWithStringStorage final
{
	NumberValue value;
	char string[N];
};

struct BoolValue final : public ValueBase
{
	bool value;

	constexpr BoolValue( const bool in_value ) noexcept
		: ValueBase(Type::Bool)
		, value(in_value)
		, padding{0u}
	{}

private:
	unsigned char padding[3u];
};

// Class for hight-level json access.
// Note, that this class is pointer-like. It is lightweight and does not contains
// any data. Instead, it only usues data from some storage.
class Value final
{
public:
	Value() noexcept;
	explicit Value( const ValueBase* value ) noexcept;

	// Type access.
	ValueBase::Type GetType() const noexcept;
	bool IsNull() const noexcept;
	bool IsObject() const noexcept;
	bool IsArray() const noexcept;
	bool IsString() const noexcept;
	bool IsNumber() const noexcept;
	bool IsBool() const noexcept;

	// Returns element count for object/array types. Returns 0 for others.
	size_t ElementCount() const noexcept;

	// Returns true if type is object and it have member.
	bool IsMember( const StringType& key ) const noexcept;

	// Member access for arrays.
	// Returns NullValue, if value is not array or if index out of bounds.
	Value operator[]( size_t array_index ) const noexcept;

	// Special overload for operator[], when "size_t" and "unsigned int" are different types.
	template<
		class IndexType= unsigned int>
		typename std::enable_if< !std::is_same< IndexType, size_t >::value, Value >::type
	operator[]( IndexType array_index ) const noexcept
	{
		return (*this)[ static_cast<size_t>(array_index) ];
	}

	// Member access for objects. Returns NullValue, if value does not containt key,
	Value operator[]( const StringType& key ) const noexcept;

	// Convertions to numbers.
	// For boolnean values returns "0" or "1".
	// For string, object, array objects returns "0".
	double AsDouble() const noexcept;
	int64_t AsInt64() const noexcept;
	uint64_t AsUint64() const noexcept;
	int32_t AsInt() const noexcept;
	uint32_t AsUint() const noexcept;

	// Returns original string for string values.
	// Returns empty string for object and array values.
	// Returns empty string for null values.
	// Returns string representation for numbers. If parser didn`t save original value, empty string returned.
	// Returns "true" or "false" for bool values.
	StringType AsString() const noexcept;

	// Equality operations.
	// Methods can be slow for big values, especially for arrays.

	// Null values are allways equals.
	// Objects is equals, if size equals and equals eack key-value pair.
	// Arrays is equals, if size equals and equals each value pair.
	// String are equals, if equals it content.
	// Numbers are equals if equals both double and integer representations.
	// Bools are equals if equals bool values.
	bool operator==( const Value& other ) const noexcept;
	bool operator!=( const Value& other ) const noexcept;

	// Iterators
private:
	// TODO - maybe support random access iterators?
	typedef std::bidirectional_iterator_tag IteratorsTag;
public:

	// Universal iterator for objects and arrays.
	// Can iteratre over values, but not over objects keys.
	// Slower, then specialized iterators.
	class UniversalIterator final : public std::iterator< IteratorsTag, Value >
	{
	private:
		friend class Value;

		union Ptr
		{
			const ValueBase* const* array_value;
			const ObjectValue::ObjectEntry* object_entry;
		};

		UniversalIterator( ValueBase::Type type, Ptr ptr ) noexcept;

	public:
		UniversalIterator() noexcept {}

		bool operator==( const UniversalIterator& other ) const noexcept;
		bool operator!=( const UniversalIterator& other ) const noexcept;

		UniversalIterator& operator++() noexcept;
		UniversalIterator& operator--() noexcept;
		UniversalIterator operator++(int) noexcept;
		UniversalIterator operator--(int) noexcept;

		Value operator*() const noexcept;
		// We can not use operator-> here, because we can not return pointer-type or poiter-like type.
		// TODO - maybe add operator-> for Value class?

	private:
		ValueBase::Type type_; // Must be array or object.
		Ptr ptr_;
	};

	// Iterator for arrays.
	class ArrayIterator final : public std::iterator< IteratorsTag, Value >
	{
	private:
		friend class Value;
		explicit ArrayIterator( const ValueBase* const* ptr ) noexcept;

	public:
		ArrayIterator() noexcept {}

		bool operator==( const ArrayIterator& other ) const noexcept;
		bool operator!=( const ArrayIterator& other ) const noexcept;

		ArrayIterator& operator++() noexcept;
		ArrayIterator& operator--() noexcept;
		ArrayIterator operator++(int) noexcept;
		ArrayIterator operator--(int) noexcept;

		Value operator*() const noexcept;
		// We can not use operator-> here, because we can not return pointer-type or poiter-like type.
		// TODO - maybe add operator-> for Value class?

	private:
		const ValueBase* const* ptr_;
	};

	// Iterator for objects.
	// Provides access for keys too, unlike UniversalIterator.
	class ObjectIterator final
		: public std::iterator< IteratorsTag, std::pair<StringType, Value> >
	{
	private:
		friend class Value;
		explicit ObjectIterator( const ObjectValue::ObjectEntry* ptr ) noexcept;

	public:
		ObjectIterator() noexcept {}

		bool operator==( const ObjectIterator& other ) const noexcept;
		bool operator!=( const ObjectIterator& other ) const noexcept;

		ObjectIterator& operator++() noexcept;
		ObjectIterator& operator--() noexcept;
		ObjectIterator operator++(int) noexcept;
		ObjectIterator operator--(int) noexcept;

		value_type operator*() const noexcept;
		// We can not use operator-> here, because we can not return pointer-type or poiter-like type.
		// TODO - maybe add operator-> for Value class?

	private:
		const ObjectValue::ObjectEntry* ptr_;
	};

	// Helper class for iteration over array/object values.
	template<class Iterator>
	class IteratorRange final
	{
	public:
		IteratorRange( const Iterator begin, const Iterator end ) noexcept;

		Iterator begin() const noexcept;
		Iterator end() const noexcept;

	private:
		Iterator begin_;
		Iterator end_;
	};

	// Iterators.
	// All iterators valid until "Value" destroying.

	// Universal iterators - for both arrays and objects.
	// For orher types begin() == end().
	UniversalIterator begin() const noexcept;
	UniversalIterator end() const noexcept;
	UniversalIterator cbegin() const noexcept;
	UniversalIterator cend() const noexcept;

	// Iterators for arrays.
	// For other types begin() == end().
	ArrayIterator array_begin() const noexcept;
	ArrayIterator array_end() const noexcept;
	ArrayIterator array_cbegin() const noexcept;
	ArrayIterator array_cend() const noexcept;

	// Iterators for objects.
	// For other types begin() == end().
	ObjectIterator object_begin() const noexcept;
	ObjectIterator object_end() const noexcept;
	ObjectIterator object_cbegin() const noexcept;
	ObjectIterator object_cend() const noexcept;

	// Helpers for simple iteration as object/array.
	IteratorRange<ArrayIterator> array_elements() const noexcept;
	IteratorRange<ObjectIterator> object_elements() const noexcept;

	// Get internal value.
	// Do it, if you know, what you did.
	const ValueBase* GetInternalValue() const noexcept;

private:
	const ValueBase* SearchObject( const ObjectValue& object, const StringType& key ) const noexcept;

private:
	const ValueBase* value_;
};

} // namespace PanzerJson

#include "value.inl"
