#include <cmath>
#include <cstring>
#include "../PanzerJson/parser.hpp"
#include "tests.hpp"

using namespace PanzerJson;

static void SimpleObjectParseTest()
{
	static const char json_text[]= "{}";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Object );
	test_assert( result.root.ElementCount() == 0u );
}

static void SimpleArrayParseTest()
{
	static const char json_text[]= "[]";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Array );
	test_assert( result.root.ElementCount() == 0u );
}

static void SimpleStringParseTest()
{
	static const char json_text[]= u8"\"Ü\"";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::String );
	test_assert( std::strcmp( result.root.AsString(), u8"Ü" ) == 0 );
}

static void SimpleNumberParseTest()
{
	static const char json_text[]= "88667";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt() == 88667 );
}

static void SimpleNegativeNumberParseTest()
{
	static const char json_text[]= "-886600";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt() == -886600 );
}

static void LongIntegerParseTest()
{
	static const char json_text[]= "7652154277405721596"; // Integer with 64bit width.

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 7652154277405721596ll );
}

static void LongNegativeIntegerParseTest()
{
	static const char json_text[]= "-8652144277705421598"; // Negative integer with 64bit width.

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == -8652144277705421598ll );
}

static void LongExponentialIntegerTest()
{
	static const char json_text[]= "5e18";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 5000000000000000000ll );
}

static void PositiveExponentialNumberTest()
{
	// Should preserve fractional part in integer.
	static const char json_text[]= "54.76e+5";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 5476000 );
	test_assert( result.root.AsDouble() == 5476000.0 );
}

static void NegativeExponentialNumberTest()
{
	// Should discard fractional part int integer representation, should preserve all as Double.
	static const char json_text[]= "54587e-4";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 5 );
	test_assert_near( result.root.AsDouble(), 5.4587, 1e6 );
}

static void ZeroExponentNumberTest0()
{
	static const char json_text[]= "-58687e0";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == -58687 );
	test_assert( result.root.AsDouble() == -58687.0 );
}

static void ZeroExponentNumberTest1()
{
	static const char json_text[]= "-58687e+0";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == -58687 );
	test_assert( result.root.AsDouble() == -58687.0 );
}

static void ZeroExponentNumberTest2()
{
	static const char json_text[]= "-58687e-0";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == -58687 );
	test_assert( result.root.AsDouble() == -58687.0 );
}

static void ZeroNumberParseTest0()
{
	static const char json_text[]= "0"; // Simple zero.

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 0 );
	test_assert( result.root.AsDouble() == 0.0 );
}

static void ZeroNumberParseTest1()
{
	static const char json_text[]= "0.0"; // Fractional zero

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 0 );
	test_assert( result.root.AsDouble() == 0.0 );
}

static void ZeroNumberParseTest2()
{
	static const char json_text[]= "0000.0000"; // Surplus '0' zero

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 0 );
	test_assert( result.root.AsDouble() == 0.0 );
}

static void ZeroNumberParseTest3()
{
	static const char json_text[]= "0e12"; // Exponential zero

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert( result.root.AsInt64() == 0 );
	test_assert( result.root.AsDouble() == 0.0 );
}

static void BigDoubleParseTest0()
{
	static const char json_text[]= "375.2e145";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert_near( result.root.AsDouble(), 375.2e145, 1e139 );
}

static void BigDoubleParseTest1()
{
	static const char json_text[]= "-4.45897e87";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert_near( result.root.AsDouble(), -4.45897e87, 1e78 );
}

static void BigDoubleParseTest2()
{
	static const char json_text[]= "78.478e-245";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Number );
	test_assert_near( result.root.AsDouble(), 78.478e-245, 1e-236 );
}

static void ComplexObjectParseTest()
{
	static const char json_text[]=
	u8R"(
			{
				"a" : 42,
				"bww" : {},
				"wdwdawdwd" : []
			}
		)";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Object );
	test_assert( result.root.ElementCount() == 3u );
	test_assert( result.root.IsMember( "bww" ) );
	test_assert( result.root[ "bww" ].GetType() == ValueBase::Type::Object );
	test_assert( result.root.IsMember( "a" ) );
	test_assert( result.root[ "a" ].AsInt() == 42 );
}

static void ComplexArrayParseTest()
{
	static const char json_text[]=
	u8R"(
			[
				42,
				{},
				"wtf"
			]
		)";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::Array );
	test_assert( result.root.ElementCount() == 3u );
	test_assert( result.root[0u].AsInt() == 42 );
	test_assert( result.root[1u].GetType() == ValueBase::Type::Object );
	test_assert( std::strcmp( result.root[2u].AsString(), "wtf" ) == 0 );
}

static void ComplexStrigParseTest()
{
	static const char json_text[]=
	u8R"(
			"ö  Ö  да---\n next line \t\t \\  \r"
		)";

	const Parser::Result result= Parser().Parse( json_text, sizeof(json_text ) );

	test_assert( result.error == Parser::Result::Error::NoError );
	test_assert( result.root.GetType() == ValueBase::Type::String );
	test_assert( std::strcmp( result.root.AsString(), u8"ö  Ö  да---\n next line \t\t \\  \r" ) == 0 );
}

void RunParserTests()
{
	SimpleObjectParseTest();
	SimpleArrayParseTest();
	SimpleStringParseTest();
	SimpleNumberParseTest();
	SimpleNegativeNumberParseTest();
	LongIntegerParseTest();
	LongNegativeIntegerParseTest();
	LongExponentialIntegerTest();
	PositiveExponentialNumberTest();
	NegativeExponentialNumberTest();
	ZeroExponentNumberTest0();
	ZeroExponentNumberTest1();
	ZeroExponentNumberTest2();
	ZeroNumberParseTest0();
	ZeroNumberParseTest1();
	ZeroNumberParseTest2();
	ZeroNumberParseTest3();
	BigDoubleParseTest0();
	BigDoubleParseTest1();
	BigDoubleParseTest2();
	ComplexObjectParseTest();
	ComplexArrayParseTest();
	ComplexStrigParseTest();
}