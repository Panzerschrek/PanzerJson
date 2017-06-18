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

static void UniversalIteratorTest0()
{
	// Universal iterator over object.

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

	const auto it_begin= value.begin();
	const auto it_end= value.end();

	test_assert(it_end != it_begin);  // Must be not equal for nonempty object.

	// Dereference test.
	test_assert((*it_begin).AsInt() == 1);
	test_assert((*it_begin).IsBool());

	{ // Iteration over test.
		Value iterated_values[16];
		unsigned int iteration_count= 0u;
		for( const Value sub_value : value )
		{
			iterated_values[ iteration_count ]= sub_value;
			++iteration_count;
		}

		test_assert(iteration_count == value.ElementCount());
		for( unsigned int i= 0u; i < iteration_count; i++ )
		{
			Value original_value(objects[i].value);
			test_assert(iterated_values[i].GetType() == original_value.GetType());
			test_assert(iterated_values[i].AsInt64() == original_value.AsInt64());
			test_assert(iterated_values[i].AsDouble() == original_value.AsDouble());
			test_assert(std::strcmp( iterated_values[i].AsString(), original_value.AsString() ) == 0 );
		}
	}

	{ // Increment/decrement, compare operations test.

		auto it= it_begin;
		auto next_it= ++it;
		auto prev_next_it = --next_it;
		test_assert( it == it );
		test_assert( next_it != it );
		test_assert( prev_next_it != it );
	}
	{ // postfix operator++ test
		auto it= it_begin;
		auto next_it= it++;
		test_assert( next_it == it_begin );
		test_assert( it != next_it );
	}
	{ // postfix operator-- test
		auto it= it_end;
		auto prev_it= it--;
		test_assert( prev_it == it_end );
		test_assert( it != prev_it );
	}
	{ // prefix operator++ test
		auto it= it_begin;
		auto next_it= ++it;
		test_assert( next_it == it );
		test_assert( it != it_begin );
	}
	{ // prefix operator-- test
		auto it= it_end;
		auto prev_it= --it;
		test_assert( prev_it == it );
		test_assert( it != it_end );
	}
}

static void UniversalIteratorTest1()
{
	// Universal iterator over array.

	static constexpr NumberValue number_value( "1458.4", 1458, 1458.4 );
	static constexpr StringValue string_value( "a" );
	static constexpr BoolValue bool_value( true );
	static constexpr ObjectValue object_value( nullptr, 0u );
	static constexpr const ValueBase* objects[4u]
	{
		&number_value,
		&string_value,
		&bool_value ,
		&object_value,
	};
	static constexpr ArrayValue array_value( objects, 4u );
	Value value(&array_value);

	const auto it_begin= value.begin();
	const auto it_end= value.end();

	test_assert(it_end != it_begin); // Must be not equal for nonempty array.

	// Dereference test.
	test_assert((*it_begin).IsNumber());
	test_assert((*it_begin).AsInt() == 1458);

	{ // Iteration over test.
		Value iterated_values[16];
		unsigned int iteration_count= 0u;
		for( const Value sub_value : value )
		{
			iterated_values[ iteration_count ]= sub_value;
			++iteration_count;
		}

		test_assert(iteration_count == value.ElementCount());
		for( unsigned int i= 0u; i < iteration_count; i++ )
		{
			Value original_value(objects[i]);
			test_assert(iterated_values[i].GetType() == original_value.GetType());
			test_assert(iterated_values[i].AsInt64() == original_value.AsInt64());
			test_assert(iterated_values[i].AsDouble() == original_value.AsDouble());
			test_assert(std::strcmp( iterated_values[i].AsString(), original_value.AsString() ) == 0 );
		}
	}

	{ // Increment/decrement, compare operations test.

		auto it= it_begin;
		auto next_it= ++it;
		auto prev_next_it = --next_it;
		test_assert( it == it );
		test_assert( next_it != it );
		test_assert( prev_next_it != it );
	}
	{ // postfix operator++ test
		auto it= it_begin;
		auto next_it= it++;
		test_assert( next_it == it_begin );
		test_assert( it != next_it );
	}
	{ // postfix operator-- test
		auto it= it_end;
		auto prev_it= it--;
		test_assert( prev_it == it_end );
		test_assert( it != prev_it );
	}
	{ // prefix operator++ test
		auto it= it_begin;
		auto next_it= ++it;
		test_assert( next_it == it );
		test_assert( it != it_begin );
	}
	{ // prefix operator-- test
		auto it= it_end;
		auto prev_it= --it;
		test_assert( prev_it == it );
		test_assert( it != it_end );
	}
}

static void UniversalIteratorTest2()
{
	// Iterate over empty object.
	ObjectValue empty_object_value(nullptr, 0u);
	Value value( &empty_object_value );

	const auto it_begin= value.begin();
	const auto it_end= value.end();

	test_assert(it_end == it_begin);// Must be equal for empty object.

	unsigned int iteration_count= 0u;
	for( const Value sub_value : value )
	{
		(void)sub_value;
		++iteration_count;
	}

	test_assert( iteration_count == 0u );
}

static void UniversalIteratorTest3()
{
	// Iterate over empty array.
	ArrayValue empty_array_value(nullptr, 0u);
	Value value( &empty_array_value );

	const auto it_begin= value.begin();
	const auto it_end= value.end();

	test_assert(it_end == it_begin); // Must be equal for empty array.

	unsigned int iteration_count= 0u;
	for( const Value sub_value : value )
	{
		(void)sub_value;
		++iteration_count;
	}

	test_assert( iteration_count == 0u );
}

static void UniversalIteratorTest4()
{
	// Iteration over nonobject or nonarray types.

	const auto test_iterate=
	[]( const Value value ) -> void
	{
		test_assert( value.begin() == value.end() );

		unsigned int iteration_count= 0u;
		for( const Value sub_value : value )
		{
			(void)sub_value;
			++iteration_count;
		}
		test_assert( iteration_count == 0u );
	};

	static constexpr NullValue null_value;
	test_iterate( Value( &null_value ) );

	static constexpr StringValue string_value( "wtf" );
	test_iterate( Value( &string_value ) );

	static constexpr NumberValue nuber_value( "4578.5", 4578, 4578.5 );
	test_iterate( Value( &nuber_value ) );

	static constexpr BoolValue bool_value( false );
	test_iterate( Value( &bool_value ) );
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
	UniversalIteratorTest0();
	UniversalIteratorTest1();
	UniversalIteratorTest2();
	UniversalIteratorTest3();
	UniversalIteratorTest4();
}