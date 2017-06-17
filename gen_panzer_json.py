import argparse
import json
import os
import sys

def LoadFile( file_name ):
	with open( file_name ) as file:
		file.seek( 0, os.SEEK_END )
		file_sise= file.tell()
		file.seek( 0, os.SEEK_SET )
		return file.read( file_sise )

def ParseJson( json_content_str ):
	return json.loads( json_content_str )

named_values_counter= 0
def NextCounter():
	global named_values_counter
	named_values_counter= named_values_counter + 1
	return str(named_values_counter)

def Stringify( something ):
	return "\"" + str(something) + "\""


# Returns pair of strings.
# First string - preinitializers, second string - name.
def WritePanzerJsonCpp( json_struct ):

	if type(json_struct) is dict:
		obj_storage_name= "object_storage" + NextCounter()
		obj_value_name= "object_value" + NextCounter()
		result_object_storage= "constexpr const ObjectValue::ObjectEntry " + obj_storage_name + "[]\n{\n"
		result_object= "constexpr const ObjectValue " + obj_value_name + "( " + obj_storage_name + ", " + str(len(json_struct)) + " );\n\n"
		result_preinitializer= ""

		for object_key in json_struct :
			member_value= WritePanzerJsonCpp( json_struct[object_key] )
			result_preinitializer= result_preinitializer + member_value[0]
			result_object_storage= result_object_storage + "\t{ " + Stringify(object_key) + ", &" + member_value[1] + " },\n"

		result_object_storage+= "};\n";
		return [ result_preinitializer + result_object_storage + result_object, obj_value_name ]

	if type(json_struct) is list:
		arr_storage_name= "array_storage" + NextCounter()
		arr_value_name= "array_value" + NextCounter()
		result_array_storage= "constexpr const ValueBase* " + arr_storage_name + "[]\n{\n"
		result_array= "constexpr const ArrayValue " + arr_value_name + "( " + arr_storage_name + ", " + str(len(json_struct)) + " );\n\n"
		result_preinitializer= ""

		for array_member in json_struct :
			member_value= WritePanzerJsonCpp( array_member )
			result_preinitializer= result_preinitializer + member_value[0]
			result_array_storage= result_array_storage  + "\t&" + member_value[1] + ",\n"

		result_array_storage+= "};\n";
		return [ result_preinitializer + result_array_storage + result_array, arr_value_name ]

	if type(json_struct) is str:
		var_name= "string_value" + NextCounter()
		return [ "constexpr StringValue " + var_name + "(" + Stringify(json_struct) + ");\n\n", var_name ]

	if type(json_struct) is int:
		var_name= "number_value" + NextCounter()
		return [ "constexpr NumberValue " + var_name + "(" + Stringify(json_struct) + ", " + str(json_struct) + ", " + str(json_struct) + ".0" + ");\n\n", var_name ]

	if type(json_struct) is float:
		var_name= "number_value" + NextCounter()
		return [ "constexpr NumberValue " + var_name + "(" + Stringify(json_struct) + ", " + str(int(json_struct)) + ", " + str(json_struct) + ");\n\n", var_name ]

	if type(json_struct) is bool:
		var_name= "bool_value" + NextCounter()
		return [ "constexpr BoolValue " + var_name + "(" + str(json_struct).lower() + ");\n\n", var_name ]

	if json_struct is None:
		return [ "", "null" ]

	print( "unknown type: " + str(type(json_struct)) )

	return [ "", "" ]


def main():
	parser = argparse.ArgumentParser(description='Process some integers.')
	parser.add_argument( "-i", help= "input json file", type=str )
	parser.add_argument( "-o", help= "output cpp file", type=str )

	args= parser.parse_args()

	print( "Convert \"" + args.i + "\" to \"" + args.o + "\"" )

	file_content= LoadFile( args.i )
	json_struct= ParseJson( file_content )
	result= WritePanzerJsonCpp( json_struct )
	print( result[0] )

if __name__ == "__main__":
	sys.exit(main())
