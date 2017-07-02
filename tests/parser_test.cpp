#include <cmath>
#include <cstring>
#include <limits>
#include "../include/PanzerJson/parser.hpp"
#include "tests.hpp"

using namespace PanzerJson;

static void SimpleObjectParseTest()
{
	static const char json_text[]= "{}";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Object );
	test_assert( result->root.ElementCount() == 0u );
}

static void SimpleArrayParseTest()
{
	static const char json_text[]= "[]";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Array );
	test_assert( result->root.ElementCount() == 0u );
}

static void SimpleStringParseTest()
{
	static const char json_text[]= u8"\"Ü\"";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::String );
	test_assert( std::strcmp( result->root.AsString(), u8"Ü" ) == 0 );
}

static void SumpleNullParseTest()
{
	static const char json_text[]=
	u8R"(
		{
			"foo" : null,
			"bar" : {},
			"baz" : null,
			"wtf" : [ 5, "str", null ]
		}
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root["foo"].IsNull() );
	test_assert( result->root["baz"].IsNull() );
	test_assert( result->root["wtf"][2u].IsNull() );
}

static void SumpleBoolParseTest()
{
	static const char json_text[]=
	u8R"(
		{
			"must_be_false" : false,
			"foo" : true,
			"bar" : {},
			"baz" : true,
			"wtf" : [ 5, "str", false, "str2", true ],
			"must_be_true_inside" : [ true ]
		}
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root["must_be_false"].IsBool() && result->root["must_be_false"].AsInt() == 0 );
	test_assert( result->root["foo"].IsBool() && result->root["foo"].AsInt() == 1 );
	test_assert( result->root["baz"].IsBool() && result->root["baz"].AsInt() == 1 );
	test_assert( result->root["wtf"][2u].IsBool() && result->root["wtf"][2u].AsInt() == 0 );
	test_assert( result->root["wtf"][4u].IsBool() && result->root["wtf"][4u].AsInt() == 1 );
	test_assert( result->root["must_be_true_inside"][0u].IsBool() && result->root["must_be_true_inside"][0u].AsInt() == 1 );
}

static void SimpleNumberParseTest()
{
	static const char json_text[]= "88667";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt() == 88667 );
}

static void SimpleNegativeNumberParseTest()
{
	static const char json_text[]= "-886600";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt() == -886600 );
}

static void LongIntegerParseTest()
{
	static const char json_text[]= "7652154277405721596"; // Integer with 64bit width.

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 7652154277405721596ll );
}

static void OverflowingIntegerParseTest0()
{
	// Overflowing value must be clamped to limit.
	static const char json_text[]= "1e56";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsUint64() == std::numeric_limits<uint64_t>::max() );
	test_assert_near( result->root.AsDouble(), 1.0e56, 1.0e47 );
}

static void OverflowingIntegerParseTest1()
{
	// Overflowing value must be clamped to limit.
	static const char json_text[]= "-1e56";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == std::numeric_limits<int64_t>::min() );
	test_assert_near( result->root.AsDouble(), -1.0e56, 1.0e47 );
}

static void OverflowingIntegerParseTest2()
{
	// Overflowing value must be clamped to limit.
	static const char json_text[]= "1564578545525637437852786578527857852782782814522742828";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsUint64() == std::numeric_limits<uint64_t>::max() );
	test_assert_near( result->root.AsDouble(), +1564578545525637437852786578527857852782782814522742828.0, 1.0e45 );
}

static void OverflowingIntegerParseTest3()
{
	// Overflowing value must be clamped to limit.
	static const char json_text[]= "-1564578545525637437852786578527857852782782814522742828";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == std::numeric_limits<int64_t>::min() );
	test_assert_near( result->root.AsDouble(), -1564578545525637437852786578527857852782782814522742828.0, 1.0e45 );
}

static void LongNegativeIntegerParseTest()
{
	static const char json_text[]= "-8652144277705421598"; // Negative integer with 64bit width.

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == -8652144277705421598ll );
}

static void LongExponentialIntegerTest()
{
	static const char json_text[]= "5e18";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 5000000000000000000ll );
}

static void PositiveExponentialNumberTest()
{
	// Should preserve fractional part in integer.
	static const char json_text[]= "54.76e+5";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 5476000 );
	test_assert( result->root.AsDouble() == 5476000.0 );
}

static void NegativeExponentialNumberTest()
{
	// Should discard fractional part int integer representation, should preserve all as Double.
	static const char json_text[]= "54587e-4";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 5 );
	test_assert_near( result->root.AsDouble(), 5.4587, 1e6 );
}

static void ZeroExponentNumberTest0()
{
	static const char json_text[]= "-58687e0";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == -58687 );
	test_assert( result->root.AsDouble() == -58687.0 );
}

static void ZeroExponentNumberTest1()
{
	static const char json_text[]= "-58687e+0";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == -58687 );
	test_assert( result->root.AsDouble() == -58687.0 );
}

static void ZeroExponentNumberTest2()
{
	static const char json_text[]= "-58687e-0";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == -58687 );
	test_assert( result->root.AsDouble() == -58687.0 );
}

static void ZeroNumberParseTest0()
{
	static const char json_text[]= "0"; // Simple zero.

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 0 );
	test_assert( result->root.AsDouble() == 0.0 );
}

static void ZeroNumberParseTest1()
{
	static const char json_text[]= "0.0"; // Fractional zero

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 0 );
	test_assert( result->root.AsDouble() == 0.0 );
}

static void ZeroNumberParseTest2()
{
	static const char json_text[]= "0.0000"; // Surplus '0' zero

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 0 );
	test_assert( result->root.AsDouble() == 0.0 );
}

static void ZeroNumberParseTest3()
{
	static const char json_text[]= "0e12"; // Exponential zero

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert( result->root.AsInt64() == 0 );
	test_assert( result->root.AsDouble() == 0.0 );
}

static void BigDoubleParseTest0()
{
	static const char json_text[]= "375.2e145";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), 375.2e145, 1e139 );
}

static void BigDoubleParseTest1()
{
	static const char json_text[]= "-4.45897e87";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), -4.45897e87, 1e78 );
}

static void BigDoubleParseTest2()
{
	static const char json_text[]= "78.478e-245";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), 78.478e-245, 1e-236 );
}

static void ZeroLeadingDoubleParseTest0()
{
	static const char json_text[]= "0.00005458";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), 0.00005458, 1.0e-16 );
}

static void ZeroLeadingDoubleParseTest1()
{
	static const char json_text[]= "0.00005458e25";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), 0.00005458e25, 1.0e9 );
}

static void ZeroLeadingDoubleParseTest2()
{
	static const char json_text[]= "0.00005458e-25";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), 0.00005458e-25, 1.0e-41 );
}

static void ZeroLeadingDoubleParseTest3()
{
	static const char json_text[]= "5467582475765";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Number );
	test_assert_near( result->root.AsDouble(), 5467582475765.0, 1.0e1 );
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

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Object );
	test_assert( result->root.ElementCount() == 3u );
	test_assert( result->root.IsMember( "bww" ) );
	test_assert( result->root[ "bww" ].GetType() == ValueBase::Type::Object );
	test_assert( result->root.IsMember( "a" ) );
	test_assert( result->root[ "a" ].AsInt() == 42 );
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

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Array );
	test_assert( result->root.ElementCount() == 3u );
	test_assert( result->root[0u].AsInt() == 42 );
	test_assert( result->root[1u].GetType() == ValueBase::Type::Object );
	test_assert( std::strcmp( result->root[2u].AsString(), "wtf" ) == 0 );
}

static void ComplexStrigParseTest()
{
	static const char json_text[]=
	u8R"(
			"ö  Ö  да---\n next line \t\t \\  \r"
		)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::String );
	test_assert( std::strcmp( result->root.AsString(), u8"ö  Ö  да---\n next line \t\t \\  \r" ) == 0 );
}

static void StringWithSymbolsCodesParseTest()
{
	// 'ж' - 0x0436, 2 utf-8 characters
	// '→' - 0x2192, 3 utf-8 characters
	static const char json_text[]=
	u8R"(
			"\u0436ало \u2192"
		)";

	const Parser::ResultPtr result= Parser().Parse( json_text );
	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::String );
	test_assert( std::strcmp( result->root.AsString(), u8"жало →" ) == 0 );
}

static void DepthHierarchyTest0()
{
	static const char json_text[]=
	u8R"(
		{
			"a" : 42,
			"megaarray" :
			[[[[[[[[[[[[[[[
			{
				"c" : {},
				"d" : "wtf",
				"megaobject" :
				{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{
					"foo" : "foo",
					"bar" : "bar"
				}}}}}}}}}}}}}}}}}}}}}}
			}
			]]]]]]]]]]]]]]]

		}
		)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.GetType() == ValueBase::Type::Object );
}

static void DepthHierarchyTest1()
{
	// Depth objects.
	static const char json_text[]=
	u8R"(
		{
			"a" : 42,
			"b" :
			{
					"d" : 42,
					"e" :
					{
						"HHH" : 1488,
						"KKKKK" :
						{
								"0" : { "a" : 5, "b" : 6, "c" : 7 },
								"1" : { "A" : 8, "B" : 9, "C" : 10 }
						}
					},
					"f" : "lol",
					"g" : null
			},
			"c" : "lol"
		}
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.IsObject() );
	test_assert( result->root.ElementCount() == 3u );
	test_assert( result->root["b"].IsObject() );
	test_assert( result->root["b"].ElementCount() == 4u );
	test_assert( result->root["b"]["e"].IsObject() );
	test_assert( result->root["b"]["e"].ElementCount() == 2u );
	test_assert( result->root["b"]["e"]["HHH"].AsInt() == 1488 );
	test_assert( result->root["b"]["e"]["KKKKK"].IsObject() );
	test_assert( result->root["b"]["e"]["KKKKK"].ElementCount() == 2u );
	test_assert( result->root["b"]["e"]["KKKKK"]["0"].IsObject() );
	test_assert( result->root["b"]["e"]["KKKKK"]["0"].ElementCount() == 3u );
	test_assert( result->root["b"]["e"]["KKKKK"]["0"]["a"].AsInt() == 5 );
	test_assert( result->root["b"]["e"]["KKKKK"]["0"]["b"].AsInt() == 6 );
	test_assert( result->root["b"]["e"]["KKKKK"]["0"]["c"].AsInt() == 7 );
	test_assert( result->root["b"]["e"]["KKKKK"]["1"].IsObject() );
	test_assert( result->root["b"]["e"]["KKKKK"]["1"].ElementCount() == 3u );
	test_assert( result->root["b"]["e"]["KKKKK"]["1"]["A"].AsInt() == 8 );
	test_assert( result->root["b"]["e"]["KKKKK"]["1"]["B"].AsInt() == 9 );
	test_assert( result->root["b"]["e"]["KKKKK"]["1"]["C"].AsInt() == 10 );
}

static void DepthHierarchyTest2()
{
	// Depth arrays.
	static const char json_text[]=
	u8R"(
			[
				"lol",
				"r2d2",
				-56,
				[
					[
						{},
						{
							"arr":
							[
								85, 86, 87, 88
							]
						},
						[
							"ein", "zwei", "drei", "vier", "fünf"
						],
						"str"
					],
					"some",
					568131819
				],
				"24",
				"SpongeBob"
			]
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.IsArray() );
	test_assert( result->root.ElementCount() == 6u );
	test_assert( std::strcmp( result->root[1u].AsString(), "r2d2" ) == 0 );
	test_assert( result->root[3u].IsArray() );
	test_assert( result->root[3u].ElementCount() == 3u );
	test_assert( result->root[3u][0u].IsArray() );
	test_assert( result->root[3u][0u].ElementCount() == 4u );
	test_assert( result->root[3u][0u][1u]["arr"].IsArray() );
	test_assert( result->root[3u][0u][1u]["arr"].ElementCount() == 4u );
	test_assert( result->root[3u][0u][2u].IsArray() );
	test_assert( result->root[3u][0u][2u].ElementCount() == 5u );
	test_assert( std::strcmp( result->root[3u][0u][2u][4u].AsString(), u8"fünf" ) == 0 );
}

static void ResultToSharedPtrCastTest()
{
	static const char json_text[]=
	u8R"(
			[
				"lol",
				"r2d2",
				-56,
				{
					"a" : 0, "b" : 1, "c" : 22
				}
			]
	)";

	const Parser::ResultSharedPtr result= Parser().Parse( json_text );
	const Parser::ResultSharedPtr result_copy= result;

	test_assert( result_copy->error == Parser::Result::Error::NoError );
	test_assert( result_copy->root.IsArray() );
	test_assert( result_copy->root.ElementCount() == 4u );
}

static void CommentsTest0()
{
	static const char json_text[]=
	u8R"(
		{  // line comment -1
			"foo" : "bar", // line comment 0"
			// Comment between lines
			"baz" : 42
		}  // line comment 1
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.IsObject() );
	test_assert( result->root.ElementCount() == 2u );
	test_assert( result->root["baz"].AsInt() == 42u );
}

static void CommentsTest1()
{
	static const char json_text[]=
	u8R"(
		{
			"foo" /* foo comment */ : "bar", /* comment after bar */
			/* comment before baz" */ "baz" /* just */ : /**/ 42
		}
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.IsObject() );
	test_assert( result->root.ElementCount() == 2u );
	test_assert( result->root["baz"].AsInt() == 42u );
}

static void CommentsTest2()
{
	static const char json_text[]=
	u8R"(
		{
			"foo" : "bar", /* multiple */
			// different
			// style
			// comments
			/* in json */
			"baz" /*0*//*1*//*2*/: 42
		}
	)";

	const Parser::ResultPtr result= Parser().Parse( json_text );

	test_assert( result->error == Parser::Result::Error::NoError );
	test_assert( result->root.IsObject() );
	test_assert( result->root.ElementCount() == 2u );
	test_assert( result->root["baz"].AsInt() == 42u );
}


void RunParserTests()
{
	SimpleObjectParseTest();
	SimpleArrayParseTest();
	SimpleStringParseTest();
	SumpleNullParseTest();
	SumpleBoolParseTest();
	SimpleNumberParseTest();
	SimpleNegativeNumberParseTest();
	LongIntegerParseTest();
	OverflowingIntegerParseTest0();
	OverflowingIntegerParseTest1();
	OverflowingIntegerParseTest2();
	OverflowingIntegerParseTest3();
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
	ZeroLeadingDoubleParseTest3();
	ZeroNumberParseTest3();
	BigDoubleParseTest0();
	BigDoubleParseTest1();
	BigDoubleParseTest2();
	ZeroLeadingDoubleParseTest0();
	ZeroLeadingDoubleParseTest1();
	ZeroLeadingDoubleParseTest2();
	ComplexObjectParseTest();
	ComplexArrayParseTest();
	ComplexStrigParseTest();
	StringWithSymbolsCodesParseTest();
	DepthHierarchyTest0();
	DepthHierarchyTest1();
	DepthHierarchyTest2();
	ResultToSharedPtrCastTest();
	CommentsTest0();
	CommentsTest1();
	CommentsTest2();
}
