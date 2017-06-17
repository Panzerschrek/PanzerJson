TEMPLATE= app
CONFIG-= qt
CONFIG+= c++11

INCLUDEPATH += ".."
INCLUDEPATH += OBJECTS_DIR # For test we need read temporary files, generated from jsons.

# Put path to your python here.
PYTHON= "C:/Documents And Settings/Panzerschrek/Python/Python.exe"

SOURCES += \
	../src/constexpr_init_test.cpp \
	../src/parser.cpp \
	../src/tests_main.cpp \
	../src/value.cpp \

HEADERS += \
	../PanzerJson/parser.hpp \
	../PanzerJson/value.inl \
	../PanzerJson/value.hpp \


# custom "comliler" for tests.
# Runs python script for generating of c++ files from jsons, than, run over result files c++ compiler.

CONVERT_SCRIPT= "../gen_panzer_json.py"
BuildConvertedJson.output= gen_${QMAKE_FILE_BASE}.o
BuildConvertedJson.commands= $$PYTHON $$CONVERT_SCRIPT -i ${QMAKE_FILE_NAME} -o gen_${QMAKE_FILE_BASE} -n ${QMAKE_FILE_BASE};
BuildConvertedJson.commands+= g++ -I".." -std=c++11 -c gen_${QMAKE_FILE_BASE}.cpp -o gen_${QMAKE_FILE_BASE}.o
BuildConvertedJson.input= JSON_TESTS

QMAKE_EXTRA_COMPILERS+= BuildConvertedJson

JSON_TESTS+= \
	../test_jsons/complex_object.json \
	../test_jsons/simple_object.json \
	../test_jsons/sort_test.json \
