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
	const ValueBase* objects;
	size_t object_count;

	constexpr ArrayValue( const ValueBase* const in_objects, const size_t in_object_count ) noexcept
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
};

} // namespace PanzerJson
