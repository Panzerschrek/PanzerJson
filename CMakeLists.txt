cmake_minimum_required(VERSION 3.8)

project(PanzerJson)

set (CMAKE_CXX_STANDARD 11)

set( PANZER_JSON_SOURCES 
	"src/panzer_json_assert.hpp"
	"src/parser.cpp"
	"src/serializer.cpp"
	"src/value.cpp" 
)

set( PANZER_JSON_HEADERS
	"include/PanzerJson/fwd.hpp"
	"include/PanzerJson/value.hpp"
	"include/PanzerJson/value.inl"
	"include/PanzerJson/parser.hpp"
	"include/PanzerJson/serializer.hpp"
)

add_library( PanzerJsonLib ${PANZER_JSON_SOURCES} ${PANZER_JSON_HEADERS} )

#tests stuff

include_directories( "include" )
include_directories( ${CMAKE_BINARY_DIR} )

set( TEST_JSONS 
	"../tests/test_jsons/complex_object.json"
	"../tests/test_jsons/int_convert_test.json"
	"../tests/test_jsons/simple_object.json"
	"../tests/test_jsons/sort_test.json"
	"../tests/test_jsons/utf8_test.json"
	"../tests/test_jsons/null_and_bool_values_pooling_test.json"
	"../tests/test_jsons/numbers_pooling_test.json"
	"../tests/test_jsons/strings_pooling_test.json"
	"../tests/test_jsons/objects_and_arrays_pooling_test.json"
	"../tests/test_jsons/string_values_as_key_reuse_test.json" )

foreach(F ${TEST_JSONS})
    get_filename_component( FILE_NAME ${F} NAME_WE )
    add_custom_command(
        OUTPUT gen_${FILE_NAME}.cpp
        COMMAND python ../gen_panzer_json.py -o gen_${FILE_NAME} -i ${F} -n ${FILE_NAME}
        COMMENT "Compiling ${FILE_NAME} to cpp file")
    list (APPEND COMPILED_JSONS gen_${FILE_NAME}.cpp )
endforeach()

#add_custom_target( BuildJsons ALL ${COMPILED_JSONS})

set( PANZER_JSON_TESTS
	"tests/constexpr_init_test.cpp"
	"tests/parser_errors_test.cpp"
	"tests/parser_test.cpp"
	"tests/parsers_equality_test.cpp"
	"tests/tests_main.cpp"
	"tests/value_test.cpp"
)

add_executable( PanzerJson_Tests ${PANZER_JSON_TESTS} ${COMPILED_JSONS} )
target_link_libraries( PanzerJson_Tests PanzerJsonLib )
