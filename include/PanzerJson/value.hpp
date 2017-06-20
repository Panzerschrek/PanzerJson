#pragma once
#include <cstdint>
#include <iterator>
#include <utility>

namespace PanzerJson
{

// String type is null-terminated UTF-8.
// TODO - maybe add support for width encodings?
// TODO - maybe support string views?
using StringType= const char*;
int StringCompare( const StringType& l, const StringType& r );

struct NullValue;
struct ObjectValue;
struct ArrayValue;
struct StringValue;
struct NumberValue;
struct BoolValue;

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

struct NullValue final : public ValueBase
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

	// WARNING! SubObjects must be sorted by key.
	uint32_t object_count;
	const ObjectEntry* sub_objects;

	constexpr ObjectValue( const ObjectEntry* const in_sub_objects, const uint32_t in_object_count ) noexcept
		: ValueBase(Type::Object)
		, object_count(in_object_count)
		, sub_objects(in_sub_objects)
	{}
};

struct ArrayValue final : public ValueBase
{
	uint32_t object_count;
	const ValueBase* const* objects;

	constexpr ArrayValue( const ValueBase* const* const in_objects, const uint32_t in_object_count ) noexcept
		: ValueBase(Type::Array)
		, object_count(in_object_count)
		, objects(in_objects)
	{}
};

struct StringValue final : public ValueBase
{
	StringType str;

	constexpr StringValue( const StringType& in_str ) noexcept
		: ValueBase(Type::String)
		, str(in_str)
	{}
};

struct NumberValue final : public ValueBase
{
	StringType str;
	int64_t int_value;
	double double_value;

	// Creator (script or parser) must store original str, and correctly convert to int/double values.
	constexpr NumberValue( const char* const str, const int64_t in_int_value, const double in_double_value ) noexcept
		: ValueBase(Type::Number)
		, str(str)
		, int_value(in_int_value)
		, double_value(in_double_value)
	{}
};

struct BoolValue final : public ValueBase
{
	bool value;

	constexpr BoolValue( const bool in_value ) noexcept
		: ValueBase(Type::Bool)
		, value(in_value)
	{}
};

// Class for hight-level json access.
// TODO - maybe addd some constexpr?
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
	// Returns string representation for numbers. TODO - does this need?
	// Returns "true" or "false" for bool values.
	StringType AsString() const noexcept;

	// Simple serialization.
	// TODO - does this needs in this library?
	template<class Stream>
	void Serialize( Stream& stream );

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

	static_assert( sizeof(UniversalIterator) <= sizeof(void*) * 2u, "Universal iterator is too large." );
	static_assert( sizeof(ArrayIterator) == sizeof(void*), "Specialized iterator must have pointer size." );
	static_assert( sizeof(ObjectIterator) == sizeof(void*), "Specialized iterator must have pointer size." );

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

private:
	const ValueBase* SearchObject( const ObjectValue& object, const StringType& key ) const noexcept;

	template<class Stream>
	static void SerializeString( Stream& stream, StringType str );

	template<class Stream>
	static void Serialize_r( Stream& stream, const ValueBase& value, size_t tab );

private:
	const ValueBase* value_;
};

} // namespace PanzerJson

#include "value.inl"
