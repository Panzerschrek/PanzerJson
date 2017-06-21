#include "../include/PanzerJson/parser.hpp"
#include "tests.hpp"

using namespace PanzerJson;

static void UnexpectedEndOfFileTestObject0()
{
	// End of file in object.
	static const char json_text[]= "{  ";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestObject1()
{
	// End of file in object, after ",".
	static const char json_text[]=
	u8R"(
			{ "foo" : "bar",
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestObject2()
{
	// End of file in object, after ":".
	static const char json_text[]=
	u8R"(
			{ "foo" : "br", "baz" :
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestArray0()
{
	// End of file in array.
	static const char json_text[]= "[  ";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestArray1()
{
	// End of file in array, after ",".
	static const char json_text[]=
	u8R"(
			[ "bar",
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestString0()
{
	// End of file in string.
	static const char json_text[]=
	u8R"(
			"str
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestString1()
{
	// End of file in string after "\".
	static const char json_text[]= "\"str\\";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestNumber0()
{
	// End of file in number - after fractional point.
	static const char json_text[]= "{\"num\" : 4.";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestNumber1()
{
	// End of file in number - after "-".
	static const char json_text[]= "{\"num\" : -";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestNumber2()
{
	// End of file in number - after "e".
	static const char json_text[]= "{\"num\" : 42e";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestNull()
{
	// End of file in null.
	static const char json_text[]= "{\"val\" : nu";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestTrue()
{
	// End of file in true.
	static const char json_text[]= "{\"val\" : tru";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

static void UnexpectedEndOfFileTestFalse()
{
	// End of file in false.
	static const char json_text[]= "{\"val\" : f";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedEndOfFile );
}

void RunParserErrorsTests()
{
	UnexpectedEndOfFileTestObject0();
	UnexpectedEndOfFileTestObject1();
	UnexpectedEndOfFileTestObject2();
	UnexpectedEndOfFileTestArray0();
	UnexpectedEndOfFileTestArray1();
	UnexpectedEndOfFileTestString0();
	UnexpectedEndOfFileTestString1();
	UnexpectedEndOfFileTestNumber0();
	UnexpectedEndOfFileTestNumber1();
	UnexpectedEndOfFileTestNumber2();
	UnexpectedEndOfFileTestNull();
	UnexpectedEndOfFileTestTrue();
	UnexpectedEndOfFileTestFalse();
}
