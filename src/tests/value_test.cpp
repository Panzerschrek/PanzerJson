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
		{ "1foo", &bool_value },
		{ u8"2Zwölf", &string_value },
		{ u8"3\n1", &number_value },
	};
	static constexpr ObjectValue object_value( objects, 3u );
	Value value(&object_value);

	test_assert( value.GetType() == ValueBase::Type::Object );
	test_assert( value.ElementCount() == 3u );
	test_assert( value.AsInt() == 0 );
	test_assert( value.AsDouble() == 0.0 );
	test_assert( value.IsMember( "1foo" ) );
	test_assert( value[ "1foo" ].AsInt() == 1 );
	test_assert( value.IsMember( u8"2Zwölf" ) );
	test_assert( std::strcmp( value[ u8"2Zwölf" ].AsString(), "a" ) == 0 );
	test_assert( value.IsMember( "3\n1" ) );
	test_assert( value[ "3\n1" ].AsDouble() == 1458.4 );

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

static void ObjectValueSearchTest()
{
	static constexpr BoolValue bool_value0( true );
	static constexpr StringValue string_value0( "a" );
	static constexpr NumberValue number_value0( "1458.4", 1458, 1458.4 );
	static constexpr BoolValue bool_value1( false );
	static constexpr StringValue string_value1( "wtf" );
	static constexpr NumberValue number_value1( "-25", -25, -25.0);
	static constexpr const ValueBase* array_objects[3]
	{
		&bool_value0 ,
		&string_value1,
		&number_value1,
	};
	static constexpr ArrayValue array_value( array_objects, 3u );
	static constexpr ObjectValue::ObjectEntry objects[7u]
	{
		{ "apple", &bool_value0 },
		{ "beer", &string_value0 },
		{ "candy", &number_value0 },
		{ "death", &bool_value1 },
		{ "element", &string_value1 },
		{ "fruit", &number_value1 },
		{ "g not gay", &array_value },
	};
	static constexpr ObjectValue object_value( objects, 7u );
	Value value(&object_value);

	test_assert( value["apple"].IsBool() && value["apple"].AsInt() == 1 );
	test_assert( value["beer"].IsString() && std::strcmp( value["beer"].AsString(), "a" ) == 0 );
	test_assert( value["candy"].IsNumber() && value["candy"].AsDouble() == 1458.4 );
	test_assert( value["death"].IsBool() && value["death"].AsInt() == false );
	test_assert( value["element"].IsString() && std::strcmp( value["element"].AsString(), "wtf" ) == 0 );
	test_assert( value["fruit"].IsNumber() && value["fruit"].AsInt() == -25 );
	test_assert( value["g not gay"].IsArray() && value["g not gay"].ElementCount() == 3u );

	// Out of bounds test.
	test_assert( !value.IsMember( "aaa" ) );
	test_assert( !value.IsMember( "zzz" ) );

	// Nonexistent key test
	test_assert( !value.IsMember( "BIG" ) );
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
	ObjectValueSearchTest();
}
