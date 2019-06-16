cmake_minimum_required(VERSION 2.8)

project(PanzerJson)

option( PANZER_JSON_TESTS NO "Build tests" )

set( CMAKE_CXX_STANDARD 11 )

if( ${CMAKE_BUILD_TYPE} STREQUAL Debug )
	add_definitions( -DDEBUG )
endif()

file( GLOB PANZER_JSON_SOURCES "src/*" )
file( GLOB PANZER_JSON_HEADERS "include/PanzerJson/*" )

add_library( PanzerJsonLib ${PANZER_JSON_SOURCES} ${PANZER_JSON_HEADERS} )

if( ${PANZER_JSON_TESTS} )
	message( STATUS "Generate PanzerJson tests\n" )

	file( GLOB TEST_JSONS "tests/test_jsons/*.json" )

	foreach( F ${TEST_JSONS} )
		get_filename_component( FILE_NAME ${F} NAME_WE )
		set( SCRIPT_COMMAND python3 ${CMAKE_SOURCE_DIR}/gen_panzer_json.py )
		set( OUT_FILE_BASE gen/gen_${FILE_NAME} )
		set( OUT_CPP_FILE ${OUT_FILE_BASE}.cpp )
		set( SRC_FILE ${F} )
		add_custom_command(
			OUTPUT ${OUT_CPP_FILE}
			DEPENDS ${SRC_FILE}
			COMMAND ${SCRIPT_COMMAND} -o ${OUT_FILE_BASE} -i ${SRC_FILE} -n ${FILE_NAME} )
		list( APPEND COMPILED_JSONS ${OUT_CPP_FILE} )
	endforeach()

	file( GLOB PANZER_JSON_TESTS "tests/*.cpp" )

	add_executable( PanzerJson_Tests ${PANZER_JSON_TESTS} ${COMPILED_JSONS} )
	target_include_directories( PanzerJson_Tests PRIVATE "include" ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/gen )
	target_link_libraries( PanzerJson_Tests PanzerJsonLib )

	add_custom_target(
		PanzerJson_Tests_Run ALL
		COMMAND PanzerJson_Tests
		DEPENDS PanzerJson_Tests
		COMMENT "Run tests"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )

endif()
