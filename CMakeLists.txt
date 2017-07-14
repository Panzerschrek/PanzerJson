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

add_library(PanzerJson_lib, ${PANZER_JSON_SOURCES} ${PANZER_JSON_HEADERS} )
