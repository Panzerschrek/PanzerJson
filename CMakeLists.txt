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


set( PANZER_JSON_TESTS
	"tests/constexpr_init_test.cpp"
	"tests/parser_errors_test.cpp"
	"tests/parser_test.cpp"
	"tests/tests_main.cpp"
	"tests/value_test.cpp"
)

add_executable( PanzerJson_Tests ${PANZER_JSON_TESTS} )
target_link_libraries( PanzerJson_Tests PanzerJsonLib )
