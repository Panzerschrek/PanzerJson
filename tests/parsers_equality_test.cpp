#include <string>

#include "gen_complex_object.hpp"
#include "gen_int_convert_test.hpp"
#include "gen_simple_object.hpp"
#include "gen_sort_test.hpp"
#include "gen_null_and_bool_values_pooling_test.hpp"
#include "gen_numbers_pooling_test.hpp"
#include "gen_strings_pooling_test.hpp"
#include "gen_objects_and_arrays_pooling_test.hpp"
#include "gen_string_values_as_key_reuse_test.hpp"

#include "../include/PanzerJson/parser.hpp"
#include "tests.hpp"

std::string ReadTestJsonFile( const char* const test_json_name )
{
	const std::string file_path= std::string( "tests/test_jsons/" ) + test_json_name + ".json";
	std::FILE* const f= std::fopen( file_path.c_str(), "rb" );
	test_assert( f != nullptr );

	std::fseek( f, 0, SEEK_END );
	const size_t file_size= std::ftell(f);
	std::fseek( f, 0, SEEK_SET );

	std::string result;
	result.resize( file_size, 0 );

	std::fread( &result[0], 1, file_size, f );
	std::fclose( f );

	return result;
}

using namespace PanzerJson;

void RunParsersEqualityTests()
{
	// Equality test.
	// Result of runtime parser must be equal to result of python-script.

	Parser parser;
	#define CHECK_TEST_JSON( TEST_NAME ) \
	{\
		const std::string src_json= ReadTestJsonFile( #TEST_NAME );\
		const Parser::ResultPtr result= parser.Parse( src_json.data(), src_json.size() );\
		test_assert( result->root == Value( &TEST_NAME ) );\
	}

	CHECK_TEST_JSON( complex_object )
//	CHECK_TEST_JSON( int_convert_test )
	CHECK_TEST_JSON( simple_object )
	CHECK_TEST_JSON( sort_test )
	CHECK_TEST_JSON( null_and_bool_values_pooling_test )
//	CHECK_TEST_JSON( numbers_pooling_test )
	CHECK_TEST_JSON( strings_pooling_test )
	CHECK_TEST_JSON( objects_and_arrays_pooling_test )
	CHECK_TEST_JSON( string_values_as_key_reuse_test )
}
