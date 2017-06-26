#include <cstring>

#include "panzer_json_assert.hpp"

#include "../include/PanzerJson/value.hpp"

namespace PanzerJson
{

// Some usable static_asserts

namespace StaticAsserts
{

constexpr bool is64bit= sizeof(void*) > 4u;
constexpr size_t ptr_size= sizeof(void*);
constexpr size_t enum_size= sizeof(ValueBase::Type);

static_assert(
	enum_size == 1u,
	"Value::Type size must be 1");
static_assert(
	sizeof(ValueBase) == enum_size,
	"Unexpected size of ValueBase");

static_assert(
	sizeof(NullValue) == enum_size, // enum value
	"Unexpceted size of NullValue");

static_assert(
	sizeof(ObjectValue) == ( is64bit ? ( ptr_size ) : ( sizeof(int32_t) * 2u ) ), // enum value + padding + uint32
	"Unexpceted size of ObjectValue");

static_assert(
	sizeof(ArrayValue ) == ( is64bit ? ( ptr_size ) : ( sizeof(int32_t) * 2u ) ), // enum value + padding + uint32
	"Unexpceted size of ArrayValue");

static_assert(
	sizeof(StringValue) == enum_size, // enum value
	"Unexpceted size of StringValue");

static_assert(
	sizeof(NumberValue) == ( sizeof(int32_t) * 2u + sizeof(int64_t) + sizeof(double) ), // enum value + padding + int64 + double
	"Unexpceted size of NumberValue");

static_assert(
	sizeof(BoolValue) == enum_size + sizeof(bool), // enum value + bool
	"Unexpceted size of BoolValue");

// Nulls are not pointer-aligned.
static_assert( sizeof(ObjectValue) % ptr_size == 0u, "Value classes must have pointer-scaled size." );
static_assert( sizeof(ObjectValue::ObjectEntry) % ptr_size == 0u, "Value classes must have pointer-scaled size." );
static_assert( sizeof(ArrayValue) % ptr_size == 0u, "Value classes must have pointer-scaled size." );
// Strings are not pointer-aligned.
static_assert( sizeof(NumberValue) % ptr_size == 0u, "Value classes must have pointer-scaled size." );
// Bools are not pointer-aligned.

static_assert(
	sizeof(ObjectValueWithEntriesStorage<       0u>) == sizeof(ObjectValue) &&
	sizeof(ObjectValueWithEntriesStorage<       1u>) == sizeof(ObjectValue) + sizeof(ObjectValue::ObjectEntry) &&
	sizeof(ObjectValueWithEntriesStorage<10000000u>) == sizeof(ObjectValue) + sizeof(ObjectValue::ObjectEntry) * 10000000u,
	"Object`s entries storage must store entries just behind object and have no gaps between object and entries." );

static_assert(
	sizeof(ArrayValueWithElementsStorage<       0u>) == sizeof(ArrayValue) &&
	sizeof(ArrayValueWithElementsStorage<       1u>) == sizeof(ArrayValue) + sizeof(const ValueBase*) &&
	sizeof(ArrayValueWithElementsStorage<10000000u>) == sizeof(ArrayValue) + sizeof(const ValueBase*) * 10000000u,
	"Arrays`s elements storage must have no gaps between array value and elements." );

// String storage placed just after string value.
// Alignmnet of string storage must be "1".
static_assert(
	sizeof(StringValueWithStorage<       0u>) == sizeof(StringValue) &&
	sizeof(StringValueWithStorage<       1u>) == sizeof(StringValue) + 1u &&
	sizeof(StringValueWithStorage<       2u>) == sizeof(StringValue) + 2u &&
	sizeof(StringValueWithStorage<10000000u>) == sizeof(StringValue) + 10000000u,
	"Bad string storage" );

// String storage placed just after number value.
// But, string strorage can have size, bigger, than needed, because padding added for int64/double values.
static_assert(
	sizeof(NumberValueWithStringStorage<                   0u>) == sizeof(NumberValue) &&
	sizeof(NumberValueWithStringStorage<                   1u>) == sizeof(NumberValue) + sizeof(int64_t) &&
	sizeof(NumberValueWithStringStorage< sizeof(int64_t)     >) == sizeof(NumberValue) + sizeof(int64_t) &&
	sizeof(NumberValueWithStringStorage< sizeof(int64_t) + 1u>) == sizeof(NumberValue) + sizeof(int64_t) * 2u,
	"Bad number string storage" );

static_assert( sizeof(Value::UniversalIterator) <= ptr_size * 2u, "Universal iterator is too large." );
static_assert( sizeof(Value::ArrayIterator) == ptr_size, "Specialized iterator must have pointer size." );
static_assert( sizeof(Value::ObjectIterator) == ptr_size, "Specialized iterator must have pointer size." );

}

static constexpr NullValue g_null_value_content;
static const Value g_null_value( & g_null_value_content );

int StringCompare( const StringType& l, const StringType& r ) noexcept
{
	// Change this if string type changed.

	// Currently, we have UTF-8 encoding.
	// For UTF-8 comparison result is equal to comparison result for UTF-32.
	return std::strcmp( l, r );
}

static bool ValuesAreEqual_r( const ValueBase& l, const ValueBase& r ) noexcept
{
	if( l.type != r.type )
		return false;

	switch( l.type )
	{
	case ValueBase::Type::Null:
		return true;

	case ValueBase::Type::Object:
		{
			const ObjectValue& l_object= static_cast<const ObjectValue&>(l);
			const ObjectValue& r_object= static_cast<const ObjectValue&>(r);
			if( l_object.object_count != r_object.object_count )
				return false;

			for( uint32_t i= 0u; i < l_object.object_count; i++ )
			{
				if( StringCompare( l_object.GetEntries()[i].key, r_object.GetEntries()[i].key ) != 0 )
					return false;
				if( !ValuesAreEqual_r( *l_object.GetEntries()[i].value, *r_object.GetEntries()[i].value ) )
					return false;
			}
			return true;
		}

	case ValueBase::Type::Array:
		{
			const ArrayValue& l_array= static_cast<const ArrayValue&>(l);
			const ArrayValue& r_array= static_cast<const ArrayValue&>(r);
			if( l_array.object_count != r_array.object_count )
				return false;

			for( uint32_t i= 0u; i < l_array.object_count; i++ )
			{
				if( !ValuesAreEqual_r( *l_array.GetElements()[i], *r_array.GetElements()[i] ) )
					return false;
			}
			return true;
		}

	case ValueBase::Type::String:
		return
			StringCompare(
				static_cast<const StringValue&>(l).GetString(),
				static_cast<const StringValue&>(r).GetString() ) == 0;

	case ValueBase::Type::Number:
		{
			const NumberValue& l_number= static_cast<const NumberValue&>(l);
			const NumberValue& r_number= static_cast<const NumberValue&>(r);
			// TODO - maybe compare and string values too?
			return
				l_number.int_value == r_number.int_value &&
				l_number.double_value == r_number.double_value;
		}

		case ValueBase::Type::Bool:
			return static_cast<const BoolValue&>(l).value == static_cast<const BoolValue&>(r).value;
	}

	return false;
}

Value::Value() noexcept
	: value_( &g_null_value_content )
{}

Value::Value( const ValueBase* const value ) noexcept
	: value_(value)
{
	PJ_ASSERT( value_ != nullptr );
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
			return Value( array_value.GetElements()[ array_index ] );
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
	const ObjectValue::ObjectEntry* start= object.GetEntries();
	const ObjectValue::ObjectEntry* end= object.GetEntries() + object.object_count;
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
		return static_cast<const StringValue&>(*value_).GetString();
	case ValueBase::Type::Number:
		return static_cast<const NumberValue&>(*value_).GetString();
	case ValueBase::Type::Bool:
		return static_cast<const BoolValue&>(*value_).value ? "true" : "false";
	};

	PJ_ASSERT( false && "Unexpected value type" );
	return "";
}

bool Value::operator==( const Value& other ) const noexcept
{
	return ValuesAreEqual_r( *value_, *other.value_ );
}

bool Value::operator!=( const Value& other ) const noexcept
{
	return !( *this == other );
}

Value::UniversalIterator Value::begin() const noexcept
{
	UniversalIterator::Ptr ptr;
	ptr.array_value= nullptr;
	ValueBase::Type type= ValueBase::Type::Array;

	switch(value_->type)
	{
	case ValueBase::Type::Object:
		ptr.object_entry= static_cast<const ObjectValue&>(*value_).GetEntries();
		type= ValueBase::Type::Object;
		break;

	case ValueBase::Type::Array:
		ptr.array_value= static_cast<const ArrayValue&>(*value_).GetElements();
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
			ptr.object_entry= object_value.GetEntries() + object_value.object_count;
			type= ValueBase::Type::Object;
		}
		break;

	case ValueBase::Type::Array:
		{
			const ArrayValue& array_value= static_cast<const ArrayValue&>(*value_);
			ptr.array_value= array_value.GetElements() + array_value.object_count;
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
		return ArrayIterator( static_cast<const ArrayValue&>(*value_).GetElements() );
	else
		return ArrayIterator( nullptr );
}

Value::ArrayIterator Value::array_end() const noexcept
{
	if( value_->type == ValueBase::Type::Array )
	{
		const ArrayValue& array_value= static_cast<const ArrayValue&>(*value_);
		return ArrayIterator( array_value.GetElements() + array_value.object_count );
	}
	else
		return ArrayIterator( nullptr );
}

Value::ObjectIterator Value::object_begin() const noexcept
{
	if( value_->type == ValueBase::Type::Object )
		return ObjectIterator( static_cast<const ObjectValue&>(*value_).GetEntries() );
	else
		return ObjectIterator( nullptr );
}

Value::ObjectIterator Value::object_end() const noexcept
{
	if( value_->type == ValueBase::Type::Object )
	{
		const ObjectValue& object_value= static_cast<const ObjectValue&>(*value_);
		return ObjectIterator( object_value.GetEntries() + object_value.object_count );
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
