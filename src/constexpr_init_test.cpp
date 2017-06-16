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

static constexpr StringValue bool_string_value_false(false);
static constexpr StringValue bool_string_value_true(true);

static constexpr const ValueBase* strings_arr[]{ &strings[0], &strings[1], &strings[2] };
static constexpr const ValueBase* objects_arr[]{ &simple_object_value0 };
static constexpr const ValueBase* null_values_arr[]{ &simple_null_value };

static constexpr ArrayValue simple_array_value0( strings_arr, 3u );
static constexpr ArrayValue simple_array_value1( objects_arr, 1u );
static constexpr ArrayValue simple_array_value2( null_values_arr, 1u );


}

} // namespace PanzerJson
