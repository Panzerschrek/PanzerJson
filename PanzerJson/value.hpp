#pragma once
#include <cstdint>

namespace PanzerJson
{

using StringType= const char*;
int StringCompare( const StringType& l, const StringType& r );

struct NullValue;
struct ObjectValue;
struct ArrayValue;
struct StringValue;
	
struct ValueBase
{
	enum class Type
	{
		Null,
		Object,
		Array,
		String, // Also, numbers and booleans
	};
	const Type type;

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
	const ObjectEntry* sub_objects;
	size_t object_count;

	constexpr ObjectValue( const ObjectEntry* const in_sub_objects, const size_t in_object_count ) noexcept
		: ValueBase(Type::Object)
		, sub_objects(in_sub_objects)
		, object_count(in_object_count)
	{}
};

struct ArrayValue final : public ValueBase
{
	const ValueBase* const* objects;
	size_t object_count;

	constexpr ArrayValue( const ValueBase* const* const in_objects, const size_t in_object_count ) noexcept
		: ValueBase(Type::Object)
		, objects(in_objects)
		, object_count(in_object_count)
	{}
};

struct StringValue final : public ValueBase
{
	StringType str;

	constexpr StringValue( const StringType& in_str ) noexcept
		: ValueBase(Type::String)
		, str(in_str)
	{}

	constexpr StringValue( const bool bool_value ) noexcept
		: ValueBase(Type::String)
		, str( bool_value ? "true" : "false" )
	{}
};

// Class for hight-level json access.
// TODO - maybe addd some constexpr?
class Value final
{
public:
	Value( const ValueBase* value ) noexcept;
	~Value();

	ValueBase::Type GetType() const noexcept;

	// Returns element count for object/array types. Returns 0 for others.
	size_t ElementCount() const noexcept;

	// Returns true if type is object and it have member.
	bool IsMember( const StringType& key ) const noexcept;

	// Member access for arrays.
	// Returns NullValue, if value is not array or if index out of bounds.
	Value operator[]( size_t array_index ) const noexcept;

	// Member access for objects. Returns NullValue, if value does not containt key,
	Value operator[]( const StringType& key ) const noexcept;

private:
	const ValueBase* SearchObject( const ObjectValue& object, const StringType& key ) const noexcept;

private:
	const ValueBase* const value_;
};

} // namespace PanzerJson
