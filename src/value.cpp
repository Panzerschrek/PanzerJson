#include <cstring>

#include "../include/PanzerJson/value.hpp"

namespace PanzerJson
{

// Some usable static_asserts

namespace StaticAsserts
{

constexpr bool is64bit= sizeof(void*) > 4u;
constexpr size_t ptr_size= sizeof(void*);

static_assert(
	sizeof(NullValue) <= sizeof(int32_t), // enum value
	"NullValue expected to be smaller");

static_assert(
	sizeof(ObjectValue) <= ( is64bit ? ( ptr_size * 2u ) : ( sizeof(int32_t) * 2u + ptr_size ) ), // enum value + uint32 + ptr
	"ObjectValue expected to be smaller");

static_assert(
	sizeof(ArrayValue ) <= ( is64bit ? ( ptr_size * 2u ) : ( sizeof(int32_t) * 2u + ptr_size ) ), // enum value + uint32 + ptr
	"Array expected to be smaller");

static_assert(
	sizeof(StringValue) <= ptr_size * 2u, // enum value + ptr
	"StringValue expected to be smaller");

static_assert(
	sizeof(NumberValue) <= ( is64bit ? ( 2u * ptr_size + 2u * sizeof(int64_t) ) : ( sizeof(int32_t) + ptr_size + 2u * sizeof(int64_t) ) ), // enum value + ptr + int64 + int64
	"NumberValue expected to be smaller");

static_assert(
	sizeof(BoolValue) <= sizeof(int32_t) * 2u, // enum value + bool
	"BoolValue expected to be smaller");

static_assert( sizeof(Value::UniversalIterator) <= sizeof(void*) * 2u, "Universal iterator is too large." );
static_assert( sizeof(Value::ArrayIterator) == sizeof(void*), "Specialized iterator must have pointer size." );
static_assert( sizeof(Value::ObjectIterator) == sizeof(void*), "Specialized iterator must have pointer size." );

}

static constexpr NullValue g_null_value_content;
static const Value g_null_value( & g_null_value_content );

int StringCompare( const StringType& l, const StringType& r )
{
	// Change this if string type changed.

	// Currently, we have UTF-8 encoding.
	// For UTF-8 comparison result is equal to comparison result for UTF-32.
	return std::strcmp( l, r );
}

Value::Value() noexcept
	: value_( &g_null_value_content )
{}

Value::Value( const ValueBase* const value ) noexcept
	: value_(value)
{
	// TODO - assert, if input value is nullptr
}

size_t Value::ElementCount() const noexcept
{
	if( value_->type == ValueBase::Type::Object )
		return static_cast<const ObjectValue&>(*value_).object_count;
	if( value_->type == ValueBase::Type::Array )
		return static_cast<const ArrayValue&>(*value_).object_count;
	return 0u;
}

bool Value::IsMember( const StringType& key ) const noexcept
{
	(void)key;
	if( value_->type == ValueBase::Type::Object )
		return SearchObject( static_cast<const ObjectValue&>(*value_), key );

	return false;
}

Value Value::operator[]( const size_t array_index ) const noexcept
{
	if( value_->type == ValueBase::Type::Array )
	{
		const ArrayValue& array_value= static_cast<const ArrayValue&>(*value_);
		if( array_index < array_value.object_count )
			return Value( array_value.objects[ array_index ] );
	}

	return g_null_value;
}

Value Value::operator[]( const StringType& key ) const noexcept
{
	if( value_->type == ValueBase::Type::Object )
	{
		const ValueBase* const member= SearchObject( static_cast<const ObjectValue&>(*value_), key );
		if( member != nullptr )
			return Value( member );
	}
	return g_null_value;
}

const ValueBase* Value::SearchObject( const ObjectValue& object, const StringType& key ) const noexcept
{
	/*
	for( size_t i= 0u; i < object.object_count; i++ )
	{
		if( StringCompare( key, object.sub_objects[i].key ) == 0 )
			return object.sub_objects[i].value;
	}*/

	// Make binary search here.
	// WARNING! Keys must be sorted. Python script or parser must sort keys.
	const ObjectValue::ObjectEntry* start= object.sub_objects;
	const ObjectValue::ObjectEntry* end= object.sub_objects + object.object_count;
	if( start == end )
		return nullptr;

	if( StringCompare( key, start->key ) < 0 )
		return nullptr;
	if( StringCompare( key, (end-1u)->key ) > 0 )
		return nullptr;

	while(true)
	{
		if( start == end )
			return nullptr;

		const ObjectValue::ObjectEntry* const middle= start + size_t( end - start ) / 2u;
		const int comp= StringCompare( key, middle->key );
		if( comp < 0 )
			end= middle;
		else if( comp > 0 )
			start= middle + 1u;
		else
			return middle->value;
	}

	return nullptr;
}

double Value::AsDouble() const noexcept
{
	if( value_->type == ValueBase::Type::Number )
		return static_cast<const NumberValue&>(*value_).double_value;
	if( value_->type == ValueBase::Type::Bool )
		return static_cast<const BoolValue&>(*value_).value ? 1.0 : 0.0;
	return 0.0;
}

int64_t Value::AsInt64() const noexcept
{
	if( value_->type == ValueBase::Type::Number )
		return static_cast<const NumberValue&>(*value_).int_value;
	if( value_->type == ValueBase::Type::Bool )
		return static_cast<const BoolValue&>(*value_).value ? 1 : 0;
	return 0;
}

uint64_t Value::AsUint64() const noexcept
{
	return static_cast<uint64_t>(AsInt64());
}

int32_t Value::AsInt() const noexcept
{
	return static_cast<int32_t>(AsInt64());
}

uint32_t Value::AsUint() const noexcept
{
	return static_cast<uint32_t>(AsInt64());
}

StringType Value::AsString() const noexcept
{
	switch(value_->type)
	{
	case ValueBase::Type::Null:
		return "";
	case ValueBase::Type::Object:
	case ValueBase::Type::Array:
		return "";
	case ValueBase::Type::String:
		return static_cast<const StringValue&>(*value_).str;
	case ValueBase::Type::Number:
		return static_cast<const NumberValue&>(*value_).str;
	case ValueBase::Type::Bool:
		return static_cast<const BoolValue&>(*value_).value ? "true" : "false";
	};

	// TODO - assert here.
	return "";
}

Value::UniversalIterator Value::begin() const noexcept
{
	UniversalIterator::Ptr ptr;
	ptr.array_value= nullptr;
	ValueBase::Type type= ValueBase::Type::Array;

	switch(value_->type)
	{
	case ValueBase::Type::Object:
		ptr.object_entry= static_cast<const ObjectValue&>(*value_).sub_objects;
		type= ValueBase::Type::Object;
		break;

	case ValueBase::Type::Array:
		ptr.array_value= static_cast<const ArrayValue&>(*value_).objects;
		type= ValueBase::Type::Array;
		break;

	case ValueBase::Type::Null:
	case ValueBase::Type::String:
	case ValueBase::Type::Number:
	case ValueBase::Type::Bool:
		break;
	};

	return UniversalIterator( type, ptr );
}

Value::UniversalIterator Value::end() const noexcept
{
	UniversalIterator::Ptr ptr;
	ptr.array_value= nullptr;
	ValueBase::Type type= ValueBase::Type::Array;

	switch(value_->type)
	{
	case ValueBase::Type::Object:
		{
			const ObjectValue& object_value= static_cast<const ObjectValue&>(*value_);
			ptr.object_entry= object_value.sub_objects + object_value.object_count;
			type= ValueBase::Type::Object;
		}
		break;

	case ValueBase::Type::Array:
		{
			const ArrayValue& array_value= static_cast<const ArrayValue&>(*value_);
			ptr.array_value= array_value.objects + array_value.object_count;
			type= ValueBase::Type::Array;
		}
		break;

	case ValueBase::Type::Null:
	case ValueBase::Type::String:
	case ValueBase::Type::Number:
	case ValueBase::Type::Bool:
		break;
	};

	return UniversalIterator( type, ptr );
}

Value::ArrayIterator Value::array_begin() const noexcept
{
	if( value_->type == ValueBase::Type::Array )
		return ArrayIterator( static_cast<const ArrayValue&>(*value_).objects );
	else
		return ArrayIterator( nullptr );
}

Value::ArrayIterator Value::array_end() const noexcept
{
	if( value_->type == ValueBase::Type::Array )
	{
		const ArrayValue& array_value= static_cast<const ArrayValue&>(*value_);
		return ArrayIterator( array_value.objects + array_value.object_count );
	}
	else
		return ArrayIterator( nullptr );
}

Value::ObjectIterator Value::object_begin() const noexcept
{
	if( value_->type == ValueBase::Type::Object )
		return ObjectIterator( static_cast<const ObjectValue&>(*value_).sub_objects );
	else
		return ObjectIterator( nullptr );
}

Value::ObjectIterator Value::object_end() const noexcept
{
	if( value_->type == ValueBase::Type::Object )
	{
		const ObjectValue& object_value= static_cast<const ObjectValue&>(*value_);
		return ObjectIterator( object_value.sub_objects + object_value.object_count );
	}
	else
		return ObjectIterator( nullptr );
}

Value::IteratorRange<Value::ArrayIterator> Value::array_elements() const noexcept
{
	return IteratorRange<Value::ArrayIterator>( array_begin(), array_end() );
}

Value::IteratorRange<Value::ObjectIterator> Value::object_elements() const noexcept
{
	return IteratorRange<Value::ObjectIterator>( object_begin(), object_end() );
}

} // namespace PanzerJson
