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

static void UnexpectedLexemTestObject0()
{
	// Expected ":".
	static const char json_text[]=
	u8R"(
			{ "foo" , }
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestObject1()
{
	// Expected something after ","
	static const char json_text[]=
	u8R"(
			{ "foo" : "bar" , }
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestObject2()
{
	// Expected "," or "}"
	static const char json_text[]=
	u8R"(
			{ "foo" : "baz" 42 }
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestObject3()
{
	// Expected "something", got "}"
	static const char json_text[]=
	u8R"(
			{ "foo" :  }
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestObject4()
{
	// Expected ":", got "}"
	static const char json_text[]=
	u8R"(
			{ "foo"  }
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestObject5()
{
	// Expected string key.
	static const char json_text[]=
	u8R"(
			{ non_string_key  }
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestArray0()
{
	// Unexpected ",".
	static const char json_text[]=
	u8R"(
			[ "foo" , ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestArray1()
{
	// Duplicated ",".
	static const char json_text[]=
	u8R"(
			[ "foo" ,, "bar" ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNumber0()
{
	// Unexpected ".".
	static const char json_text[]=
	u8R"(
			[ 568.. ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNumber1()
{
	// Unexpected "something" after "e".
	static const char json_text[]=
	u8R"(
			[ 568e. ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNumber2()
{
	// Unexpected "something" after "-".
	static const char json_text[]=
	u8R"(
			[ -a56 ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNumber3()
{
	// Redunant leading zero.
	static const char json_text[]=
	u8R"(
			[ 0547 ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestString0()
{
	// Unknown character after '\'.
	static const char json_text[]=
	u8R"(
			[ "string\Ütr" ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestBool0()
{
	// Invalid bool.
	static const char json_text[]=
	u8R"(
			[ trur ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestBool1()
{
	// Invalid bool.
	static const char json_text[]=
	u8R"(
			[ tttt ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestBool2()
{
	// Invalid bool.
	static const char json_text[]=
	u8R"(
			[ flase ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestBool3()
{
	// Invalid bool.
	static const char json_text[]=
	u8R"(
			[ f, 0, 0, 0 ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestBool4()
{
	// Invalid bool.
	static const char json_text[]=
	u8R"(
			[ fal ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNull0()
{
	// Invalid null.
	static const char json_text[]=
	u8R"(
			[ n, 0, 0 ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNull1()
{
	// Invalid null.
	static const char json_text[]=
	u8R"(
			[ nul ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedLexemTestNull2()
{
	// Invalid null.
	static const char json_text[]=
	u8R"(
			[ nuul ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedSomething0()
{
	static const char json_text[]=
	u8R"(
			[ whath_is_shis? ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedSomething1()
{
	static const char json_text[]=
	u8R"(
			[ ? ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedSomething2()
{
	// Capital "N" does not allowed.
	static const char json_text[]=
	u8R"(
			[ Null ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedSomething3()
{
	// Capital "T" does not allowed.
	static const char json_text[]=
	u8R"(
			[ True ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedSomething4()
{
	// Capital "F" does not allowed.
	static const char json_text[]=
	u8R"(
			[ False ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
}

static void UnexpectedSomething5()
{
	// Unexpected <> shit. We are not in html.
	static const char json_text[]=
	u8R"(
			[ <body> / <body> ]
		)";

	const Parser::Result result= Parser().Parse( json_text );
	test_assert(result.error == Parser::Result::Error::UnexpectedLexem );
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
	UnexpectedLexemTestObject0();
	UnexpectedLexemTestObject1();
	UnexpectedLexemTestObject2();
	UnexpectedLexemTestObject3();
	UnexpectedLexemTestObject4();
	UnexpectedLexemTestObject5();
	UnexpectedLexemTestArray0();
	UnexpectedLexemTestArray1();
	UnexpectedLexemTestNumber0();
	UnexpectedLexemTestNumber1();
	UnexpectedLexemTestNumber2();
	UnexpectedLexemTestNumber3();
	UnexpectedLexemTestString0();
	UnexpectedLexemTestBool0();
	UnexpectedLexemTestBool1();
	UnexpectedLexemTestBool2();
	UnexpectedLexemTestBool3();
	UnexpectedLexemTestBool4();
	UnexpectedLexemTestNull0();
	UnexpectedLexemTestNull1();
	UnexpectedLexemTestNull2();
	UnexpectedSomething0();
	UnexpectedSomething1();
	UnexpectedSomething2();
	UnexpectedSomething3();
	UnexpectedSomething4();
	UnexpectedSomething5();
}
