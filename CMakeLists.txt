cmake_minimum_required(VERSION 2.8)

project(PanzerJson)

set( CMAKE_CXX_STANDARD 11 )

if( ${CMAKE_BUILD_TYPE} STREQUAL Debug )
	add_definitions( -DDEBUG )
else()
endif()

set( PANZER_JSON_SOURCES 
	src/panzer_json_assert.hpp
	src/parser.cpp
	src/serializer.cpp
	src/serializers_common.cpp
	src/serializers_common.hpp
	src/serializers_common.inl
	src/value.cpp
)

set( PANZER_JSON_HEADERS
	include/PanzerJson/fwd.hpp
	include/PanzerJson/value.hpp
	include/PanzerJson/value.inl
	include/PanzerJson/parser.hpp
	include/PanzerJson/serializer.hpp
	include/PanzerJson/serializer.inl
	include/PanzerJson/streamed_serializer.hpp
	include/PanzerJson/streamed_serializer.inl
)

add_library( PanzerJsonLib ${PANZER_JSON_SOURCES} ${PANZER_JSON_HEADERS} )

# Disable tests, if "-DPANZER_JSON_TESTS=no"
if( NOT ${PANZER_JSON_TESTS} )
	return()
endif()
message( STATUS "Generate PanzerJson tests\n" )

include_directories( "include" )
include_directories( ${CMAKE_BINARY_DIR} )
include_directories( ${CMAKE_BINARY_DIR}/gen )

set( TEST_JSONS 
	"tests/test_jsons/complex_object.json"
	"tests/test_jsons/int_convert_test.json"
	"tests/test_jsons/simple_object.json"
	"tests/test_jsons/sort_test.json"
	"tests/test_jsons/utf8_test.json"
	"tests/test_jsons/null_and_bool_values_pooling_test.json"
	"tests/test_jsons/numbers_pooling_test.json"
	"tests/test_jsons/strings_pooling_test.json"
	"tests/test_jsons/objects_and_arrays_pooling_test.json"
	"tests/test_jsons/string_values_as_key_reuse_test.json" )

foreach( F ${TEST_JSONS} )
	get_filename_component( FILE_NAME ${F} NAME_WE )
	set( SCRIPT_COMMAND python3 ${CMAKE_SOURCE_DIR}/gen_panzer_json.py )
	set( OUT_FILE_BASE gen/gen_${FILE_NAME} )
	set( OUT_CPP_FILE ${OUT_FILE_BASE}.cpp )
	set( SRC_FILE ${CMAKE_SOURCE_DIR}/${F} )
	add_custom_command(
		OUTPUT ${OUT_CPP_FILE}
		DEPENDS ${SRC_FILE}
		COMMAND ${SCRIPT_COMMAND} -o ${OUT_FILE_BASE} -i ${SRC_FILE} -n ${FILE_NAME} )
	list( APPEND COMPILED_JSONS ${OUT_CPP_FILE} )
endforeach()

set( PANZER_JSON_TESTS
	"tests/constexpr_init_test.cpp"
	"tests/parser_errors_test.cpp"
	"tests/parser_test.cpp"
	"tests/parsers_equality_test.cpp"
	"tests/tests.hpp"
	"tests/tests_main.cpp"
	"tests/value_test.cpp"
)

add_executable( PanzerJson_Tests ${PANZER_JSON_TESTS} ${COMPILED_JSONS} )
target_link_libraries( PanzerJson_Tests PanzerJsonLib )

add_custom_target(
	PanzerJson_Tests_Run ALL
	COMMAND PanzerJson_Tests
	DEPENDS PanzerJson_Tests
	COMMENT "Run tests"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
