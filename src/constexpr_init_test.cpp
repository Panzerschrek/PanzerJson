#include "../PanzerJson/value.hpp"

namespace PanzerJson
{

namespace SimpleValueTest
{

static constexpr ValueBase simple_value( ValueBase::Type::Null );
static constexpr NullValue simple_null_value;

static constexpr ObjectValue::ObjectEntry object_entry0{ "foo", &simple_null_value };
static constexpr ObjectValue simple_object_value0( &object_entry0, 1u );

static constexpr ObjectValue::ObjectEntry objects_entry1[]
{
	{ "bar", &simple_null_value },
	{ "foo", &simple_object_value0 },
};
static constexpr ObjectValue simple_object_value1( objects_entry1, 2u );

static constexpr StringValue strings[]
{
	StringValue( "Foo" ), StringValue( "Bar" ), StringValue( "Baz" ),
};

static constexpr ArrayValue simple_array_value0( strings, 3u );
static constexpr ArrayValue simple_array_value1( &simple_object_value0, 1u );
static constexpr ArrayValue simple_array_value2( &simple_null_value, 1u );

}

} // namespace PanzerJson
