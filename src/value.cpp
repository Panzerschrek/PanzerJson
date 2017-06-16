#include <cstring>

#include "../PanzerJson/value.hpp"


namespace PanzerJson
{

static constexpr NullValue g_null_value_content;
static const Value g_null_value( & g_null_value_content );

int StringCompare( const StringType& l, const StringType& r )
{
	// Change this if string type changed.
	return std::strcmp( l, r );
}

Value::Value( const ValueBase* const value ) noexcept
	: value_(value)
{
	// TODO - assert, if input value is nullptr
}

Value::~Value()
{}

ValueBase::Type Value::GetType() const noexcept
{
	return value_->type;
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
	// TODO - make binary search.
	for( size_t i= 0u; i < object.object_count; i++ )
	{
		if( StringCompare( key, object.sub_objects[i].key ) == 0 )
			return object.sub_objects[i].value;
	}

	return nullptr;
}

} // namespace PanzerJson
