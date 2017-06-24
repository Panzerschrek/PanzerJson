import argparse
import json
import os
import sys

def LoadFile( file_name ):
	with open( file_name, "r", encoding='utf-8' ) as file:
		file.seek( 0, os.SEEK_END )
		file_sise= file.tell()
		file.seek( 0, os.SEEK_SET )
		return file.read( file_sise )

def WriteFile( file_name, file_content ):
	with open( file_name, "w", encoding='utf-8' ) as file:
		file.write( file_content )

def ParseJson( json_content_str ):
	return json.loads( json_content_str )

named_values_counter= 0
def NextCounter():
	global named_values_counter
	result= named_values_counter
	named_values_counter= named_values_counter + 1
	return str(result)


def PrepareIntValue( int_value ):

	int_value = int(int_value)

	# Clamp to common range of int64_t and uint64_t
	if int_value > 0xFFFFFFFFFFFFFFFF:
		int_value= 0xFFFFFFFFFFFFFFFF
	elif int_value < -0x8000000000000000:
		int_value=   -0x8000000000000000

	# Convert big unsigned value to int64_t
	if int_value >  0x7FFFFFFFFFFFFFFF:
		int_value= int_value - 0x10000000000000000

	# Hack for most negative value.
	if int_value == -0x8000000000000000:
		return "0x8000000000000000ll"

	# Finally, convert to string
	return str(int_value) + "ll"


#produce valid c++ string literal
def MakeQuotedEscapedString( s ):
	result= "u8\""

	for c in s:
		if c == '"' :
			result= result + "\\\""
		elif c == '\0':
			result= result + "\0"
		elif c == '\\':
			result= result + "\\\\"
		elif c == '\b':
			result= result + "\\b"
		elif c == '\f':
			result= result + "\\f"
		elif c == '\n':
			result= result + "\\n"
		elif c == '\r':
			result= result + "\\r"
		elif c == '\t':
			result= result + "\\t"
		else:
			result= result + c

	result= result + "\""
	return result

# Global flags for values pooling.
null_value_emitted= False
false_bool_value_emitted= False
true_bool_value_emitted = False
number_values_pool= dict()
string_values_pool= dict()
object_values_pool= dict()
array_values_pool= dict()
string_values_data_pool= dict()

# Some params
save_string_for_numbers= False

# Returns pair of strings.
# First string - preinitializers, second string - name.
def WritePanzerJsonValue( json_struct ):
	global string_values_data_pool

	if type(json_struct) is dict:
		keys_sorted= sorted( json_struct )
		result_object_storage= ""
		result_preinitializer= ""

		for object_key in keys_sorted :
			member_value= WritePanzerJsonValue( json_struct[object_key] )
			result_preinitializer= result_preinitializer + member_value[0]

			# Try reuse storage of existent string value as key.
			pool_key_data= string_values_data_pool.get( object_key, None )
			if pool_key_data is not None:
				key_str= pool_key_data
			else:
				key_str= MakeQuotedEscapedString(object_key)

			result_object_storage= result_object_storage + "\t\t{ " + key_str + ", &" + member_value[1] + " },\n"

		# We use pooling for all values. So, if storage is equal to previous objects storage, then, objects are equal.
		global object_values_pool
		pool_object= object_values_pool.get( result_object_storage, None )
		if pool_object is not None:
			return [ "", pool_object ]

		obj_storage_name= "object_storage" + NextCounter()
		obj_value_name= obj_storage_name + ".value"

		object_values_pool[ result_object_storage ]= obj_value_name

		object_count= str(len(json_struct)) + "u"
		result_object_storage= "constexpr const ObjectValueWithEntriesStorage<" + object_count + "> " + obj_storage_name + \
		"\n{\n" + "\tObjectValue(" + object_count + "),\n" + "\t{\n" + result_object_storage + "\t}\n" + "};\n\n"

		return [ result_preinitializer + result_object_storage, obj_value_name ]

	if type(json_struct) is list:
		result_array_storage= ""
		result_preinitializer= ""

		for array_member in json_struct :
			member_value= WritePanzerJsonValue( array_member )
			result_preinitializer= result_preinitializer + member_value[0]
			result_array_storage= result_array_storage  + "\t\t&" + member_value[1] + ",\n"

		# We use pooling for all values. So, if storage is equal to previous objects storage, then, objects are equal.
		global array_values_pool
		pool_array= array_values_pool.get( result_array_storage, None )
		if pool_array is not None:
			return [ "", pool_array ]

		arr_storage_name= "array_storage" + NextCounter()
		arr_value_name= arr_storage_name + ".value"

		array_values_pool[ result_array_storage ]= arr_value_name

		object_count= str(len(json_struct)) + "u"
		result_array_storage= "constexpr const ArrayValueWithElementsStorage<" + object_count + "> " + arr_storage_name + \
		"\n{\n" + "\tArrayValue(" + object_count + "),\n" + "\t{\n" + result_array_storage + "\t}\n" + "};\n\n"

		return [ result_preinitializer + result_array_storage, arr_value_name ]

	if type(json_struct) is str:
		# Use strings pooling - emit same value once.
		global string_values_pool
		pool_value = string_values_pool.get( json_struct, None )

		if pool_value is None:

			storage_name= "string_storage" + NextCounter()
			var_name= storage_name + ".value"
			string_values_pool[ json_struct ]= var_name
			quoted_string= MakeQuotedEscapedString(json_struct)
			str_length= str(len(json_struct.encode("utf-8")) + 1) + "u"
			result_string_storage= "constexpr StringValueWithStorage<" + str_length + "> " + storage_name + \
			"\n{\n" + "\tStringValue(),\n" + "\t" + quoted_string + "\n};\n\n"

			# Also, save pointer to storage of string value.
			# We can use this pointer, also, for objects keys.
			string_values_data_pool[ json_struct ]= storage_name + ".string"

			return [ result_string_storage, var_name ]

		else:
			return [ "", pool_value ]

	if type(json_struct) is int or type(json_struct) is float:
		# Use numbers pooling - emit same value once.
		global number_values_pool
		# Make combined key - for value as int and float. Because, we can lost precision, if we will use only int or float keys.
		pool_key = str(float(json_struct)) + "___" + str(int(json_struct))
		pool_value = number_values_pool.get( pool_key, None )
		if pool_value is None:

			storage_name= "number_storage" + NextCounter()
			var_name= storage_name + ".value"
			number_values_pool[ pool_key ]= var_name

			global save_string_for_numbers
			if save_string_for_numbers:
				num_str= str(json_struct)
				num_str_quoted= "\"" + num_str + "\""
				result_number_storage= "constexpr NumberValueWithStringStorage<" + str(len(num_str) + 1) + "u> " + storage_name + \
				"\n{\n" + "\tNumberValue( " + PrepareIntValue(json_struct) + ", " + str(float(json_struct)) + ", true ),\n" + "\t" + num_str_quoted + "\n};\n\n"
			else:
				result_number_storage= "constexpr NumberValueWithStringStorage<0u> " + storage_name + \
				"\n{\n" + "\tNumberValue( " + PrepareIntValue(json_struct) + ", " + str(float(json_struct)) + ", false )" + "\n};\n\n"

			return [ result_number_storage, var_name ]

		else:
			return [ "", pool_value ]

	if type(json_struct) is bool:
		global false_bool_value_emitted
		global true_bool_value_emitted
		#Emit both bool values once.

		if bool(json_struct):
			var_name= "bool_value_true"
			if true_bool_value_emitted:
				return [ "", var_name ]
			else:
				true_bool_value_emitted= True
				return [ "constexpr BoolValue " + var_name + "(" + str(json_struct).lower() + ");\n\n", var_name ]
		else:
			var_name= "bool_value_false"
			if false_bool_value_emitted:
				return [ "", var_name ]
			else:
				false_bool_value_emitted= True
				return [ "constexpr BoolValue " + var_name + "(" + str(json_struct).lower() + ");\n\n", var_name ]


	if json_struct is None:
		#Emit null value once, because all null values are same.
		global null_value_emitted

		var_name= "null_value"
		if null_value_emitted:
			return [ "", var_name ]
		else:
			null_value_emitted= True
			return [ "constexpr NullValue " + var_name + ";\n\n", var_name ]

	return [ "", "" ]

def WritePanzerJsonCpp( json_struct, h_file_name, variable_name ):
	root_value= WritePanzerJsonValue( json_struct )

	result= "#include <PanzerJson/value.hpp>\n\n"
	result= result + "#include \"" + h_file_name + "\"\n\n"
	result= result + "namespace\n{\n\n"
	result= result + "using namespace PanzerJson;\n\n"
	result= result + root_value[0]
	result= result + "} //namespace\n\n"
	result= result + "const PanzerJson::ValueBase& " + variable_name + "= " + root_value[1] + ";\n"
	return result

def WritePanzerJsonHpp( variable_name ):
	result= "#pragma once\n"
	result= result + "#include <PanzerJson/fwd.hpp>\n"
	result= result + "extern const PanzerJson::ValueBase& " + variable_name + ";\n"
	return result

def main():
	parser = argparse.ArgumentParser(description='Process some integers.')
	parser.add_argument( "-i", help= "input json file", type=str )
	parser.add_argument( "-o", help= "output cpp/hpp file name base", type=str )
	parser.add_argument( "-n", help= "name of result variable", type=str )
	parser.add_argument( "-s", help= "save or not strings for numbers", action="store_true" )

	args= parser.parse_args()

	save_string_for_numbers= args.s

	print( "Convert \"" + args.i + "\" to \"" + args.o + "\"" )
	if save_string_for_numbers:
		print( "Save numbers strings" )
	else:
		print( "Do not save numbers strings" )

	cpp_file= args.o + ".cpp"
	hpp_file= args.o + ".hpp"

	file_content= LoadFile( args.i )
	json_struct= ParseJson( file_content )
	cpp_result= WritePanzerJsonCpp( json_struct, hpp_file, args.n )
	hpp_result= WritePanzerJsonHpp( args.n )

	WriteFile( cpp_file, cpp_result )
	WriteFile( hpp_file, hpp_result )

if __name__ == "__main__":
	sys.exit(main())
