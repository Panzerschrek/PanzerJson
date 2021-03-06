#include <algorithm>
#include <cstring>
#include "../include/PanzerJson/value.hpp"
#include "tests.hpp"

using namespace PanzerJson;

#define STRING_STORAGE( name, value )\
static constexpr StringValueWithStorage< sizeof(value) > name\
{\
	StringValue(),\
	value\
};

static_assert( sizeof(u8"str") == 4u, "invalid sizeof for utf-8 string literals" );
static_assert( sizeof(u8"абв") == 7u, "invalid sizeof for utf-8 string literals" );

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
	static constexpr NumberValueWithStringStorage<12u> number_storage
	{
		NumberValue( 42, -14.7, true ),
		"test string"
	};
	Value value( &number_storage.value );

	test_assert( value.GetType() == ValueBase::Type::Number );
	test_assert( value.AsInt() == 42 );
	test_assert( value.AsDouble() == -14.7 );
	test_assert( std::strcmp( value.AsString(), "test string" ) == 0 );
}

static void SimpleStringValueTest()
{
	STRING_STORAGE( string_storage, u8"Строковое значение\n." );
	Value value( &string_storage.value );

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
	STRING_STORAGE( string_storage, "a" );
	static constexpr NumberValue number_value( 1458, 1458.4 );
	static constexpr ObjectValueWithEntriesStorage<3u> object_torage
	{
		ObjectValue(3u),
		{
			{ "1foo", &bool_value },
			{ u8"2Zwölf", &string_storage.value },
			{ u8"3\n1", &number_value },
		}
	};
	Value value(&object_torage.value);

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
	STRING_STORAGE( string_storage, "a" );
	static constexpr NumberValue number_value( 1458, 1458.4 );
	static constexpr ArrayValueWithElementsStorage<3u> array_storage
	{
		ArrayValue(3u),
		{
			&bool_value,
			&string_storage.value,
			&number_value,
		}
	};
	Value value(&array_storage.value);

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
	STRING_STORAGE( string_storage0, "a" );
	static constexpr NumberValue number_value0( 1458, 1458.4 );
	static constexpr BoolValue bool_value1( false );
	STRING_STORAGE( string_storage1, "wtf" );
	static constexpr NumberValue number_value1( -25, -25.0);
	static constexpr ArrayValueWithElementsStorage<3u> array_storage
	{
		ArrayValue(3u),
		{
			&bool_value0 ,
			&string_storage1.value,
			&number_value1,
		}
	};
	static constexpr ObjectValueWithEntriesStorage<7u> object_storage
	{
		ObjectValue(7u),
		{
			{ "apple", &bool_value0 },
			{ "beer", &string_storage0.value },
			{ "candy", &number_value0 },
			{ "death", &bool_value1 },
			{ "element", &string_storage1.value },
			{ "fruit", &number_value1 },
			{ "g not gay", &array_storage.value },
		}
	};
	Value value( &object_storage.value );

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
	STRING_STORAGE( string_storage0, "a" );
	static constexpr NumberValueWithStringStorage<7u> number_storage0
	{
		NumberValue( 1458, 1458.4 ),
		"1458.4"
	};
	static constexpr BoolValue bool_value1( false );
	STRING_STORAGE( string_storage1, "wtf" );
	static constexpr NumberValueWithStringStorage<6u> number_storage1
	{
		NumberValue( -25, -25.0 ),
		"-25.0"
	};
	static constexpr ArrayValueWithElementsStorage<3u> array_storage
	{
		ArrayValue(3u),
		{
			&bool_value0 ,
			&string_storage1.value,
			&number_storage1.value,
		}
	};
	static constexpr ObjectValueWithEntriesStorage<7u> object_storage
	{
		ObjectValue(7u),
		{
			{ "apple", &bool_value0 },
			{ "beer", &string_storage0.value },
			{ "candy", &number_storage0.value },
			{ "death", &bool_value1 },
			{ "element", &string_storage1.value },
			{ "fruit", &number_storage1.value },
			{ "g not gay", &array_storage.value },
		}
	};
	Value value( &object_storage.value );

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
			Value original_value(object_storage.entries[i].value);
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

	static constexpr NumberValueWithStringStorage<7u> number_storage
	{
		NumberValue( 1458, 1458.4 ),
		"1458.4",
	};
	STRING_STORAGE( string_storage, "a" );
	static constexpr BoolValue bool_value( true );
	static constexpr ObjectValue object_value( 0u );
	static constexpr ArrayValueWithElementsStorage<4u> array_storage
	{
		ArrayValue(4u),
		{
			&number_storage.value,
			&string_storage.value,
			&bool_value ,
			&object_value,
		}
	};
	Value value(&array_storage.value);

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
			Value original_value(array_storage.elements[i]);
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
	ObjectValue empty_object_value(0u);
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
	ArrayValue empty_array_value(0u);
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

	STRING_STORAGE( string_storage, "wtf" );
	test_iterate( Value( &string_storage.value ) );

	static constexpr NumberValue nuber_value( 4578, 4578.5 );
	test_iterate( Value( &nuber_value ) );

	static constexpr BoolValue bool_value( false );
	test_iterate( Value( &bool_value ) );
}

static void ArrayIteratorTest0()
{
	static constexpr NumberValueWithStringStorage<7u> number_storage
	{
		NumberValue( 1458, 1458.4 ),
		"1458.4"
	};
	STRING_STORAGE( string_storage, "a" );
	static constexpr BoolValue bool_value( true );
	static constexpr ObjectValue object_value( 0u );
	static constexpr ArrayValueWithElementsStorage<4u> array_storage
	{
		ArrayValue(4u),
		{
			&number_storage.value,
			&string_storage.value,
			&bool_value ,
			&object_value,
		}
	};
	Value value(&array_storage.value);

	const auto it_begin= value.array_begin();
	const auto it_end= value.array_end();

	test_assert(it_end != it_begin); // Must be not equal for nonempty array.

	// Dereference test.
	test_assert((*it_begin).IsNumber());
	test_assert((*it_begin).AsInt() == 1458);

	{ // Iteration over test.
		Value iterated_values[16];
		unsigned int iteration_count= 0u;
		for( const Value sub_value : value.array_elements() )
		{
			iterated_values[ iteration_count ]= sub_value;
			++iteration_count;
		}

		test_assert(iteration_count == value.ElementCount());
		for( unsigned int i= 0u; i < iteration_count; i++ )
		{
			Value original_value(array_storage.elements[i]);
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

static void ArrayIteratorTest1()
{
	// Iterate over empty array.
	ArrayValue empty_array_value(0u);
	Value value( &empty_array_value );

	const auto it_begin= value.array_begin();
	const auto it_end= value.array_end();

	test_assert(it_end == it_begin); // Must be equal for empty array.

	unsigned int iteration_count= 0u;
	for( const Value sub_value : value.array_elements() )
	{
		(void) sub_value;
		++iteration_count;
	}

	test_assert( iteration_count == 0u );
}

static void ArrayIteratorTest2()
{
	// Iteration over nonarray values.

	const auto test_iterate=
	[]( const Value value ) -> void
	{
		test_assert( value.array_begin() == value.array_end() );

		unsigned int iteration_count= 0u;
		for( const Value sub_value : value.array_elements() )
		{
			(void) sub_value;
			++iteration_count;
		}
		test_assert( iteration_count == 0u );
	};

	static constexpr NullValue null_value;
	test_iterate( Value( &null_value ) );

	STRING_STORAGE( string_storage, "wtf" );
	test_iterate( Value( &string_storage.value ) );

	static constexpr NumberValue nuber_value( 4578, 4578.5 );
	test_iterate( Value( &nuber_value ) );

	static constexpr BoolValue bool_value( false );
	test_iterate( Value( &bool_value ) );

	static constexpr ObjectValueWithEntriesStorage<2u> object_storage
	{
		ObjectValue(2u),
		{
			{ "first", &nuber_value },
			{ "seconds", &string_storage.value },
		}
	};
	test_iterate( Value( &object_storage.value ) );
}

static void ObjectIteratorTest0()
{
	// Iterator over object.

	static constexpr BoolValue bool_value0( true );
	STRING_STORAGE( string_storage0, "a" );
	static constexpr NumberValueWithStringStorage<7u> number_storage0
	{
		NumberValue( 1458, 1458.4 ),
		"1458.4"
	};
	static constexpr BoolValue bool_value1( false );
	STRING_STORAGE( string_storage1, "wtf" );
	static constexpr NumberValueWithStringStorage<7u> number_storage1
	{
		NumberValue( -25, -25.0 ),
		"-25"
	};
	static constexpr ArrayValueWithElementsStorage<3u> array_storage
	{
		ArrayValue(3u),
		{
			&bool_value0 ,
			&string_storage1.value,
			&number_storage1.value,
		}
	};
	static constexpr ObjectValueWithEntriesStorage<7u> object_storage
	{
		ObjectValue(7u),
		{
			{ "apple", &bool_value0 },
			{ "beer", &string_storage0.value },
			{ "candy", &number_storage0.value },
			{ "death", &bool_value1 },
			{ "element", &string_storage1.value },
			{ "fruit", &number_storage1.value },
			{ "g not gay", &array_storage.value },
		}
	};
	Value value( &object_storage.value );

	const auto it_begin= value.object_begin();
	const auto it_end= value.object_end();

	test_assert(it_end != it_begin); // Must be not equal for nonempty object.

	// Dereference test.
	test_assert( std::strcmp( (*it_begin).first, "apple" ) == 0 );
	test_assert((*it_begin).second.AsInt() == 1);
	test_assert((*it_begin).second.IsBool());

	{ // Iteration over test.
		unsigned int iteration_count= 0u;
		for( const Value::ObjectIterator::value_type subobject_value : value.object_elements() )
		{
			Value original_value(object_storage.entries[iteration_count].value);

			test_assert( std::strcmp( subobject_value.first, object_storage.entries[iteration_count].key ) == 0 );
			test_assert(subobject_value.second.GetType() == original_value.GetType());
			test_assert(subobject_value.second.AsInt64() == original_value.AsInt64());
			test_assert(subobject_value.second.AsDouble() == original_value.AsDouble());
			test_assert(std::strcmp( subobject_value.second.AsString(), original_value.AsString() ) == 0 );

			++iteration_count;
		}

		test_assert( iteration_count == value.ElementCount() );
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

static void ObjectIteratorTest1()
{
	// Iterate over empty object.
	ObjectValue empty_object_value(0u);
	Value value( &empty_object_value );

	const auto it_begin= value.object_begin();
	const auto it_end= value.object_end();

	test_assert(it_end == it_begin); // Must be equal for empty object.

	unsigned int iteration_count= 0u;
	for( const Value::ObjectIterator::value_type subobject_value : value.object_elements() )
	{
		(void)subobject_value;
		++iteration_count;
	}

	test_assert( iteration_count == 0u );
}

static void ObjectIteratorTest2()
{
	// Iteration over nonobject values.
	const auto test_iterate=
	[]( const Value value ) -> void
	{
		test_assert( value.object_begin() == value.object_end() );

		unsigned int iteration_count= 0u;
		for( const Value::ObjectIterator::value_type subobject_value : value.object_elements() )
		{
			(void)subobject_value;
			++iteration_count;
		}
		test_assert( iteration_count == 0u );
	};

	static constexpr NullValue null_value;
	test_iterate( Value( &null_value ) );

	STRING_STORAGE( string_storage, "wtf" );
	test_iterate( Value( &string_storage.value ) );

	static constexpr NumberValue nuber_value( 4578, 4578.5 );
	test_iterate( Value( &nuber_value ) );

	static constexpr BoolValue bool_value( false );
	test_iterate( Value( &bool_value ) );

	static constexpr ArrayValueWithElementsStorage<2u> array_storage
	{
		ArrayValue(2u),
		{
			&nuber_value,
			&string_storage.value,
		}
	};
	test_iterate( Value( &array_storage.value ) );
}

static void IteratorsAlgorithms()
{
	// Test for compile-time checking of iterator traits.

	static constexpr NumberValue number_values[3]
	{
		NumberValue( 158, 158.0 ),
		NumberValue( -14, -14.0 ),
		NumberValue( 25, 25.0 ),
	};
	static constexpr ArrayValueWithElementsStorage<3u> array_storage
	{
		ArrayValue(3u),
		{
			&number_values[0],
			&number_values[1],
			&number_values[2],
		}
	};
	Value value( &array_storage.value );

	const auto pred= []( Value v ) { return v.AsDouble() >= 0.0f; };
	const auto object_pred= []( Value::ObjectIterator::value_type v ) { return v.second.AsDouble() >= 0.0f;; };
	const auto comp = []( Value l, Value r ){ return l.AsDouble() < r.AsDouble(); };
	const auto object_comp =
	[]( Value::ObjectIterator::value_type l, Value::ObjectIterator::value_type r )
	{ return l.second.AsDouble() < r.second.AsDouble(); };

	// Find if - InputIterator
	test_assert( std::find_if( value.begin(), value.end(), pred ) != value.end() );
	test_assert( std::find_if( value.array_begin(), value.array_end(), pred ) != value.array_end() );
	test_assert( std::find_if( value.object_begin(), value.object_end(), object_pred ) == value.object_end() );

	// Is Sorted - ForwardIterator
	test_assert( !std::is_sorted( value.begin(), value.end(), comp ) );
	test_assert( !std::is_sorted( value.array_begin(), value.array_end(), comp ) );
	test_assert(  std::is_sorted( value.object_begin(), value.object_end(), object_comp ) ); // Object range is empty -> it is sorted

	// TODO - add tests for BidirectionalIterator
	// Now - there are no std::algorithm, using BidirectionalIterator and no modifying input sequence.
}

static void ValueEqualityTest0()
{
	static constexpr NullValue null_value0;
	static constexpr NullValue null_value1;

	test_assert( Value( &null_value0 ) == Value( &null_value1 ) );
}

static void ValueEqualityTest1()
{
	static constexpr NullValue null_value;
	const Value value( &null_value );

	test_assert( value  == value );
	test_assert( !( value != value ) );
}

static void ValueEqualityTest2()
{
	// Basic numbers equality test.

	static constexpr NumberValue number0( 3, 3.14 );
	static constexpr NumberValue number1( 3, 3.14 );
	static constexpr NumberValue number2( 2, 2.718281828 );
	const Value number_value0( &number0 );
	const Value number_value1( &number1 );
	const Value number_value2( &number2 );

	test_assert( number_value0 == number_value0 );
	test_assert( !( number_value0 != number_value0 ) );

	test_assert( number_value2 == number_value2 );
	test_assert( !( number_value2 != number_value2 ) );

	test_assert( number_value0 == number_value1 );
	test_assert( !( number_value0 != number_value1 ) );

	test_assert( number_value0 != number_value2 );
	test_assert( !( number_value0 == number_value2 ) );
}

static void ValueEqualityTest3()
{
	// Numbers with same integer part but with dirrerent double part are not equal.
	static constexpr NumberValue number0( 3, 3.14 );
	static constexpr NumberValue number1( 3, 3.15 );
	const Value number_value0( &number0 );
	const Value number_value1( &number1 );

	test_assert( number_value0 != number_value1 );
	test_assert( !( number_value0 == number_value1 ) );
}

static void ValueEqualityTest4()
{
	// Strings equality test.

	STRING_STORAGE( string0, u8"Quick brown fox jumps over the lazy dog" );
	STRING_STORAGE( string1, u8"Quick brown fox jumps over the lazy dog" );
	STRING_STORAGE( string2, u8"Quick brown fox is dead" );
	STRING_STORAGE( string3, u8"" );
	STRING_STORAGE( string4, u8"Not so fast fox jumps over the lazy dog" );
	STRING_STORAGE( string5, u8"A" );
	STRING_STORAGE( string6, u8"a" );

	const Value string_value0( &string0.value );
	const Value string_value1( &string1.value );
	const Value string_value2( &string2.value );
	const Value string_value3( &string3.value );
	const Value string_value4( &string4.value );
	const Value string_value5( &string5.value );
	const Value string_value6( &string6.value );

	// Same string
	test_assert( string_value0 == string_value0 );
	test_assert( !( string_value0 != string_value0 ) );

	// Strings with same content
	test_assert( string_value0 == string_value1 );
	test_assert( !( string_value0 != string_value1 ) );

	// Strings with different end
	test_assert( string_value0 != string_value2 );
	test_assert( !( string_value0 == string_value2 ) );

	// Strungs with different start
	test_assert( string_value1 != string_value4 );
	test_assert( !( string_value1 == string_value4 ) );

	// Same exmpty string
	test_assert( string_value3 == string_value3 );
	test_assert( !( string_value3 != string_value3 ) );

	// Empty string not equal no nonempty string
	test_assert( string_value3 != string_value2 );
	test_assert( !( string_value3 == string_value2 ) );

	// Different case strings
	test_assert( string_value5 != string_value6 );
	test_assert( !( string_value5 == string_value6 ) );
}

static void ValueEqualityTest5()
{
	// Arrays.

	STRING_STORAGE( string0, u8"Quick brown fox jumps over the lazy dog" );
	STRING_STORAGE( string1, u8"A" );
	STRING_STORAGE( string2, u8"" );
	STRING_STORAGE( string3, u8"" );
	static constexpr NumberValue number0( 3, 3.14 );
	static constexpr NumberValue number1( 2, 2.718281828 );
	static constexpr NumberValue number2( 2, 2.718281828 );
	static constexpr NullValue null_value;

	static constexpr ArrayValue array0( 0u );
	static constexpr ArrayValue array1( 0u );

	static constexpr ArrayValueWithElementsStorage<4u> array2
	{
		ArrayValue(4u),
		{ &string0.value, &null_value, &number0, &string2.value, }
	};

	static constexpr ArrayValueWithElementsStorage<4u> array3
	{
		ArrayValue(4u),
		{ &string1.value, &null_value, &number1, &string2.value, }
	};

	static constexpr ArrayValueWithElementsStorage<4u> array4
	{
		ArrayValue(4u),
		{ &string1.value, &null_value, &number2, &string3.value, }
	};

	static constexpr ArrayValueWithElementsStorage<2u> array5
	{
		ArrayValue(2u),
		{ &number0, &string3.value, }
	};

	// Empty array equal to itself
	test_assert( Value(&array0) == Value(&array0) );
	test_assert( !( Value(&array0) != Value(&array0) ) );

	// Empty array equal to another empty array
	test_assert( Value(&array0) == Value(&array1) );
	test_assert( !( Value(&array0) != Value(&array1) ) );

	// Empty array not equal to nonempty array
	test_assert( Value(&array0) != Value(&array2.value) );
	test_assert( !( Value(&array0) == Value(&array2.value) ) );

	// Nonempty array equal to itself
	test_assert( Value(&array2.value) == Value(&array2.value) );
	test_assert( !( Value(&array2.value) != Value(&array2.value) ) );

	// Different arrays with same element count is not equal
	test_assert( Value(&array2.value) != Value(&array3.value) );
	test_assert( !( Value(&array2.value) == Value(&array3.value) ) );

	// Arrays with same size and elements are equal
	test_assert( Value(&array3.value) == Value(&array4.value) );
	test_assert( !( Value(&array3.value) != Value(&array4.value) ) );

	// Nonempty arrays with different size are not equal
	test_assert( Value(&array4.value) != Value(&array5.value) );
	test_assert( !( Value(&array4.value) == Value(&array5.value) ) );
}

static void ValueEqualityTest6()
{
	// Objects.

	STRING_STORAGE( string0, u8"Quick brown fox jumps over the lazy dog" );
	STRING_STORAGE( string1, u8"" );
	static constexpr NumberValue number0( 3, 3.14 );
	static constexpr NumberValue number1( 2, 2.718281828 );
	static constexpr NumberValue number2( 2, 2.718281828 );
	static constexpr NullValue null_value;

	static constexpr ObjectValue object0( 0u );
	static constexpr ObjectValue object1( 0u );

	static constexpr ObjectValueWithEntriesStorage<3u> object2
	{
		ObjectValue(3u),
		{ { "A", &string0.value }, { "BB", &number1 }, { "Ctror", &null_value }, }
	};
	static constexpr ObjectValueWithEntriesStorage<3u> object3
	{
		ObjectValue(3u),
		{ { "A", &string0.value }, { "BB", &number2 }, { "Ctror", &null_value }, }
	};
	static constexpr ObjectValueWithEntriesStorage<4u> object4
	{
		ObjectValue(4u),
		{ { "A", &string0.value }, { "BB", &number2 }, { "Ctror", &null_value }, { "Delta", &string1.value }, }
	};
	static constexpr ObjectValueWithEntriesStorage<4u> object5
	{
		ObjectValue(4u),
		{ { "A", &string0.value }, { "BB", &number2 }, { "Ctror", &null_value }, { "Delta2", &string1.value }, }
	};
	static constexpr ObjectValueWithEntriesStorage<4u> object6
	{
		ObjectValue(4u),
		{ { "A", &string0.value }, { "BB", &number0 }, { "Ctror", &null_value }, { "Delta", &string1.value }, }
	};

	// Empty object equal to itself
	test_assert( Value(&object0) == Value(&object0) );
	test_assert( !( Value(&object0) != Value(&object0) ) );

	// Empty object equal to another empty object
	test_assert( Value(&object0) == Value(&object1) );
	test_assert( !( Value(&object0) != Value(&object1) ) );

	// Empty object not equal to nonempty object
	test_assert( Value(&object0) != Value(&object2.value) );
	test_assert( !( Value(&object0) == Value(&object2.value) ) );

	// Nonempty object equal to itself
	test_assert( Value(&object2.value) == Value(&object2.value) );
	test_assert( !( Value(&object2.value) != Value(&object2.value) ) );

	// Nonempty object equal to other nonempty object with same content
	test_assert( Value(&object2.value) == Value(&object3.value) );
	test_assert( !( Value(&object2.value) != Value(&object3.value) ) );

	// Nonempty object not equal to object with different size
	test_assert( Value(&object2.value) != Value(&object4.value) );
	test_assert( !( Value(&object2.value) == Value(&object4.value) ) );

	// Nonempty object not equal to object with same size but different keys
	test_assert( Value(&object4.value) != Value(&object5.value) );
	test_assert( !( Value(&object4.value) == Value(&object5.value) ) );

	// Nonempty object not equal to object with same keys but different values
	test_assert( Value(&object4.value) != Value(&object6.value) );
	test_assert( !( Value(&object4.value) == Value(&object6.value) ) );
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
	ArrayIteratorTest0();
	ArrayIteratorTest1();
	ArrayIteratorTest2();
	ObjectIteratorTest0();
	ObjectIteratorTest1();
	ObjectIteratorTest2();
	IteratorsAlgorithms();
	ValueEqualityTest0();
	ValueEqualityTest1();
	ValueEqualityTest2();
	ValueEqualityTest3();
	ValueEqualityTest4();
	ValueEqualityTest5();
	ValueEqualityTest6();
}
