#include "../include/PanzerJson/value.hpp"

namespace PanzerJson
{

namespace SimpleValueTest
{

static constexpr ValueBase simple_value( ValueBase::Type::Null );
static constexpr NullValue simple_null_value;

static constexpr ObjectValueWithEntriesStorage<1u> simple_object_storage0
{
	ObjectValue(1u),
	{
		{ "foo", &simple_null_value }
	}
};

static constexpr ObjectValueWithEntriesStorage<2u> simple_object_storage1
{
	ObjectValue(2u),
	{
		{ "bar", &simple_null_value },
		{ "foo", &simple_object_storage0.value },
	}
};

static constexpr StringValue strings[]
{
	StringValue( "Foo" ), StringValue( "Bar" ), StringValue( "Baz" ),
};

static constexpr BoolValue bool_value_false(false);
static constexpr BoolValue bool_value_true(true);

static constexpr const ValueBase* strings_arr[]{ &strings[0], &strings[1], &strings[2] };
static constexpr const ValueBase* objects_arr[]{ &simple_object_storage0.value };
static constexpr const ValueBase* null_values_arr[]{ &simple_null_value };

static constexpr ArrayValue simple_array_value0( strings_arr, 3u );
static constexpr ArrayValue simple_array_value1( objects_arr, 1u );
static constexpr ArrayValue simple_array_value2( null_values_arr, 1u );

static constexpr NumberValue number_value( "42", 42, 42.0 );

}

} // namespace PanzerJson
