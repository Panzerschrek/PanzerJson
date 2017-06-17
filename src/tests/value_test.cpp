#include <cstring>
#include "../PanzerJson/value.hpp"
#include "tests.hpp"

using namespace PanzerJson;

static void SimpleNullValueTest()
{
	static constexpr NullValue null_value;
	Value value(&null_value);

	test_assert( value.GetType() == ValueBase::Type::Null );
	test_assert( value.AsInt() == 0 );
	test_assert( value.AsDouble() == 0.0 );
	test_assert( std::strcmp( value.AsString(), "" ) == 0 );
}

static void SimpleNumberValueTest()
{
	static constexpr NumberValue number_value( "test string", 42, -14.7 );
	Value value(&number_value);

	test_assert( value.GetType() == ValueBase::Type::Number );
	test_assert( value.AsInt() == 42 );
	test_assert( value.AsDouble() == -14.7 );
	test_assert( std::strcmp( value.AsString(), "test string" ) == 0 );
}

static void SimpleStringValueTest()
{
	static constexpr StringValue string_value( u8"Строковое значение\n." );
	Value value(&string_value);

	test_assert( value.GetType() == ValueBase::Type::String );
	test_assert( value.AsInt() == 0 );
	test_assert( value.AsDouble() == 0.0 );
	test_assert( std::strcmp( value.AsString(), u8"Строковое значение\n." ) == 0 );
}

static void SimpleBoolValueTest0()
{
	static constexpr BoolValue bool_value( false );
	Value value(&bool_value);

	test_assert( value.GetType() == ValueBase::Type::Bool );
	test_assert( value.AsInt() == 0 );
	test_assert( value.AsDouble() == 0.0 );
}

static void SimpleBoolValueTest1()
{
	static constexpr BoolValue bool_value( true );
	Value value(&bool_value);

	test_assert( value.GetType() == ValueBase::Type::Bool );
	test_assert( value.AsInt() == 1 );
	test_assert( value.AsDouble() == 1.0 );
}

static void SimpleObjectValueTest()
{
	static constexpr BoolValue bool_value( true );
	static constexpr StringValue string_value( "a" );
	static constexpr NumberValue number_value( "1458.4", 1458, 1458.4 );
	static constexpr ObjectValue::ObjectEntry objects[3u]
	{
		{ "foo", &bool_value },
		{ u8"Zwölf", &string_value },
		{ u8"1\n1", &number_value },
	};
	static constexpr ObjectValue object_value( objects, 3u );
	Value value(&object_value);

	test_assert( value.GetType() == ValueBase::Type::Object );
	test_assert( value.ElementCount() == 3u );
	test_assert( value.AsInt() == 0 );
	test_assert( value.AsDouble() == 0.0 );
	test_assert( value.IsMember( "foo" ) );
	test_assert( value[ "foo" ].AsInt() == 1 );
	test_assert( value.IsMember( u8"Zwölf" ) );
	test_assert( std::strcmp( value[ u8"Zwölf" ].AsString(), "a" ) == 0 );
	test_assert( value.IsMember( "1\n1" ) );
	test_assert( value[ "1\n1" ].AsDouble() == 1458.4 );

	// Out of bounds test.
	test_assert( !value.IsMember( "unexistent" ) );
	test_assert( value[ "unexistent" ].GetType() == ValueBase::Type::Null );
}

static void SimpleArrayValueTest()
{
	static constexpr BoolValue bool_value( true );
	static constexpr StringValue string_value( "a" );
	static constexpr NumberValue number_value( "1458.4", 1458, 1458.4 );
	static constexpr const ValueBase* objects[3u]
	{
		&bool_value ,
		&string_value,
		&number_value,
	};
	static constexpr ArrayValue array_value( objects, 3u );
	Value value(&array_value);

	test_assert( value.GetType() == ValueBase::Type::Array );
	test_assert( value.ElementCount() == 3u );
	test_assert( value.AsInt() == 0 );
	test_assert( value.AsDouble() == 0.0 );
	test_assert( value[0u].AsInt() == 1 );
	test_assert( std::strcmp( value[1u].AsString(), "a" ) == 0 );
	test_assert( value[2u].AsDouble() == 1458.4 );

	// Out of bounds test.
	test_assert( value[457u].GetType() == ValueBase::Type::Null );
}

void RunValueTests()
{
	SimpleNullValueTest();
	SimpleNumberValueTest();
	SimpleStringValueTest();
	SimpleBoolValueTest0();
	SimpleBoolValueTest1();
	SimpleObjectValueTest();
	SimpleArrayValueTest();
}
