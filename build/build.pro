TEMPLATE= app
CONFIG-= qt
CONFIG+= c++11

INCLUDEPATH += "../include"
INCLUDEPATH += OBJECTS_DIR # For test we need read temporary files, generated from jsons.

DEFINES+= DEBUG

QMAKE_CXXFLAGS += -Wextra

# Put path to your python here.
PYTHON= python

SOURCES += \
	../src/parser.cpp \
	../src/serializer.cpp \
	../src/value.cpp \
	../tests/constexpr_init_test.cpp \
	../tests/parser_test.cpp \
	../tests/parser_errors_test.cpp \
	../tests/parsers_equality_test.cpp \
	../tests/tests_main.cpp \
	../tests/value_test.cpp \

HEADERS += \
	../include/PanzerJson/fwd.hpp \
	../include/PanzerJson/parser.hpp \
	../include/PanzerJson/serializer.hpp \
	../include/PanzerJson/serializer.inl \
	../include/PanzerJson/value.inl \
	../include/PanzerJson/value.hpp \
	../src/panzer_json_assert.hpp \
	../tests/tests.hpp \


# custom "comliler" for tests.
# Runs python script for generating of c++ files from jsons, than, run over result files c++ compiler.

CONVERT_SCRIPT= "../gen_panzer_json.py"
BuildConvertedJson.output= gen_${QMAKE_FILE_BASE}.o
BuildConvertedJson.commands= $$PYTHON $$CONVERT_SCRIPT -i ${QMAKE_FILE_NAME} -o gen_${QMAKE_FILE_BASE} -n ${QMAKE_FILE_BASE};
BuildConvertedJson.commands+= g++ -I"../include" -std=c++11 -c gen_${QMAKE_FILE_BASE}.cpp -o gen_${QMAKE_FILE_BASE}.o
BuildConvertedJson.input= JSON_TESTS

QMAKE_EXTRA_COMPILERS+= BuildConvertedJson

JSON_TESTS+= \
	../tests/test_jsons/complex_object.json \
	../tests/test_jsons/int_convert_test.json \
	../tests/test_jsons/simple_object.json \
	../tests/test_jsons/sort_test.json \
	../tests/test_jsons/utf8_test.json \
	../tests/test_jsons/null_and_bool_values_pooling_test.json \
	../tests/test_jsons/numbers_pooling_test.json \
	../tests/test_jsons/strings_pooling_test.json \
	../tests/test_jsons/objects_and_arrays_pooling_test.json \
	../tests/test_jsons/string_values_as_key_reuse_test.json \
