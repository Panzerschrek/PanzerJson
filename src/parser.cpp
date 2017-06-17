#include <cstring>

#include "../PanzerJson/parser.hpp"

namespace PanzerJson
{

Parser::Parser()
{}

Parser::~Parser()
{
}

const ValueBase* Parser::Parse_r()
{
	SkipWhitespaces();
	if( result_.error != Result::Error::NoError )
		return nullptr;

	switch(*cur_)
	{

	// Object
	case '{':
	{
		++cur_;
		if( cur_ == end_ )
		{
			result_.error= Result::Error::UnexpectedEndOfFile;
			return nullptr;
		}

		std::vector<ObjectValue::ObjectEntry> entries;

		while(true)
		{
			SkipWhitespaces();
			if( result_.error != Result::Error::NoError )
				return nullptr;
			if( *cur_ == '"' )
			{
				const StringType key= ParseString();
				if( result_.error != Result::Error::NoError )
					return nullptr;

				SkipWhitespaces();
				if( result_.error != Result::Error::NoError )
					return nullptr;
				if( *cur_ != ':' )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
				++cur_;

				const ValueBase* const value= Parse_r();
				if( result_.error != Result::Error::NoError )
					return nullptr;

				entries.emplace_back( ObjectValue::ObjectEntry{ key, value } );

				SkipWhitespaces();
				if( result_.error != Result::Error::NoError )
					return nullptr;
				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				if( *cur_ == ',' )
				{
					++cur_;
					continue;
				}
			}
			else if( *cur_ == '}' )
			{
				++cur_;
				break;
			}
			else
			{
				result_.error= Result::Error::UnexpectedLexem;
				return nullptr;
			}
		}
		// TODO - sort object members.

		const size_t entries_offset= result_.storage.size();
		result_.storage.resize( result_.storage.size() + sizeof(ObjectValue::ObjectEntry) * entries.size() );
		ObjectValue::ObjectEntry* const result_entries=
			reinterpret_cast<ObjectValue::ObjectEntry*>( result_.storage.data() + entries_offset );
		std::memcpy( result_entries, entries.data(), sizeof(ObjectValue::ObjectEntry) * entries.size() );

		const size_t object_offset= result_.storage.size();
		result_.storage.resize( result_.storage.size() + sizeof(ObjectValue) );
		ObjectValue* const object_value= reinterpret_cast<ObjectValue*>( result_.storage.data() + object_offset );

		object_value->type= ValueBase::Type::Object;
		object_value->sub_objects= reinterpret_cast<ObjectValue::ObjectEntry*>( static_cast<char*>(nullptr) + entries_offset );
		object_value->object_count= entries.size();

		return reinterpret_cast<ObjectValue*>( static_cast<char*>(nullptr) + object_offset );
	}
	break;

	// Array
	case '[':
	{
		++cur_;
		if( cur_ == end_ )
		{
			result_.error= Result::Error::UnexpectedEndOfFile;
			return nullptr;
		}

		std::vector<const ValueBase*> values;

		while(true)
		{
			SkipWhitespaces();
			if( result_.error != Result::Error::NoError )
				return nullptr;
			if( *cur_ == ']' )
			{
				++cur_;
				break;
			}
			else
			{
				const ValueBase* const value= Parse_r();
				if( result_.error != Result::Error::NoError )
					return nullptr;

				values.push_back(value);

				SkipWhitespaces();
				if( result_.error != Result::Error::NoError )
					return nullptr;
				if( *cur_ == ',' )
				{
					++cur_;
					continue;
				}
			}
		} // while true

		const size_t values_offset= result_.storage.size();
		result_.storage.resize( result_.storage.size() + sizeof(const ValueBase*) * values.size() );
		const ValueBase* * const array_values=
			reinterpret_cast<const ObjectValue::ValueBase**>( result_.storage.data() + values_offset );
		std::memcpy( array_values, values.data(), sizeof(const ValueBase*) * values.size() );

		const size_t array_offset= result_.storage.size();
		result_.storage.resize( result_.storage.size() + sizeof(ArrayValue) );
		ArrayValue* const array_value= reinterpret_cast<ArrayValue*>( result_.storage.data() + array_offset );

		array_value->type= ValueBase::Type::Array;
		array_value->objects= reinterpret_cast<const ObjectValue::ValueBase**>( static_cast<char*>(nullptr) + values_offset );
		array_value->object_count= values.size();

		return reinterpret_cast<ArrayValue*>( static_cast<char*>(nullptr) + array_offset );
	}
	break;

	// String
	case '"':
		{
			const StringType str= ParseString();
			if( result_.error != Result::Error::NoError )
				return nullptr;

			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + sizeof(StringValue) );
			StringValue* const string_value=
				reinterpret_cast<StringValue*>( result_.storage.data() + offset );

			string_value->type= ValueBase::Type::String;
			string_value->str= str;

			return reinterpret_cast<StringValue*>( static_cast<char*>(nullptr) + offset );

		}
		break;

	default:
		// Numbers.
		if( ( *cur_ >= '0' && *cur_ <= '9' ) || *cur_ == '-' )
		{
			int64_t integer_part= 0;
			int64_t exponent= 0;
			int64_t fractional_part= 0;
			size_t fractional_part_digits= 0;
			int64_t sign= 1;
			int64_t exponent_sign= 1;

			const auto parse_decimal=
			[&]() -> int64_t
			{
				int64_t result= 0;
				while( cur_ < end_ )
				{
					if( *cur_ >= '0' && *cur_ <= '9' )
					{
						result= result * 10 + int64_t( *cur_ - '0' );
						++cur_;
					}
					else
						break;
				}
				return result;
			};

			const char* const num_start= cur_;
			if( *cur_ == '-' )
			{
				++cur_;
				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				if( !( *cur_ >= '0' && *cur_ <= '9' ) )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}

				sign= -1;
			}

			integer_part= parse_decimal();

			if( cur_ == end_ )
				goto num_parse_end;
			if( *cur_ == '.' )
			{
				++cur_;
				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				if( !( *cur_ >= '0' && *cur_ <= '9' ) )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}

				const char* const fractional_start_digit= cur_;
				fractional_part= parse_decimal();
				fractional_part_digits= cur_ - fractional_start_digit;
			}
			if( cur_ == end_ )
				goto num_parse_end;
			if( *cur_ == 'e' || *cur_ == 'E' )
			{
				++cur_;
				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}

				if( *cur_ == '+' || *cur_ == '-' )
				{
					if( *cur_ == '-' ) exponent_sign= -1;
					++cur_;
				}

				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				if( !( *cur_ >= '0' && *cur_ <= '9' ) )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}

				exponent= parse_decimal() * exponent_sign;
			}

			num_parse_end:

			const auto ten_power=
			[]( const int64_t power ) -> int64_t
			{
				int64_t result= 1;
				for( int64_t i= 0; i < power; i++ )
					result*= 10;
				return result;
			};

			int64_t result_int_val;
			double result_double_val;
			{
				result_int_val= integer_part * ten_power( exponent );

				int64_t fractional_part_to_result_int_part= fractional_part;
				for( int64_t i= 0; i < std::max( 0ll, fractional_part_digits - exponent ); i++ )
					fractional_part_to_result_int_part/= 10;

				result_int_val+= fractional_part_to_result_int_part;

				result_int_val*= sign;
			}
			{
				result_double_val= static_cast<double>(integer_part) * static_cast<double>(ten_power( exponent ));

				double fractional_part_power= 1.0;
				for( int64_t i= 0; i < std::max( 0ll, fractional_part_digits - exponent ); i++ )
					fractional_part_power/= 10.0;

				result_double_val+= fractional_part_power * static_cast<double>(fractional_part);

				result_double_val*= static_cast<double>(sign);
			}

			// Allocate string value.
			const size_t str_size= cur_ - num_start;
			const size_t str_offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + str_size + 1u );
			std::memcpy( result_.storage.data() + str_offset, num_start, str_size );
			result_.storage[ str_offset + str_size ]= '\0';

			// Allocate number value.
			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + sizeof(NumberValue) );
			NumberValue* const value= reinterpret_cast<NumberValue*>( result_.storage.data() + offset );

			// Fill data.
			value->type= ValueBase::Type::Number;
			value->str= str_offset + static_cast<const char*>(nullptr);
			value->int_value= result_int_val;
			value->double_value= result_double_val;

			return reinterpret_cast<NumberValue*>( static_cast<char*>(nullptr) + offset );
		}
		// Null
		else if( *cur_ == 'n' )
		{
			if( end_ - cur_ < 4 || std::strncmp( cur_, "null", 4 ) != 0 )
			{
				result_.error= Result::Error::UnexpectedLexem;
				return nullptr;
			}
			cur_+= 4;

			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + sizeof(NullValue) );
			NullValue* const value= reinterpret_cast<NullValue*>( result_.storage.data() + offset );

			value->type= ValueBase::Type::Null;

			return reinterpret_cast<NullValue*>( static_cast<char*>(nullptr) + offset );

		}
		else if( *cur_ == 't' || *cur_ == 'f' )
		{
			bool bool_value;
			if( *cur_ == 't' )
			{
				if( end_ - cur_ < 4 || std::strncmp( cur_, "true", 4 ) != 0 )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
				cur_+= 4u;
				bool_value= true;
			}
			else
			{
				if( end_ - cur_ < 5 || std::strncmp( cur_, "false", 5 ) != 0 )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
				cur_+= 5u;
				bool_value= false;
			}

			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + sizeof(BoolValue) );
			BoolValue* const value= reinterpret_cast<BoolValue*>( result_.storage.data() + offset );

			value->type= ValueBase::Type::Bool;
			value->value= bool_value;

			return reinterpret_cast<BoolValue*>( static_cast<char*>(nullptr) + offset );
		}
		break;
	};

	return nullptr;
}

StringType Parser::ParseString()
{
	if( *cur_ != '"' )
	{
		result_.error= Result::Error::UnexpectedLexem;
		return nullptr;
	}
	++cur_;

	const size_t offset= result_.storage.size();

	while(true)
	{
		if( cur_ == end_ )
		{
			result_.error= Result::Error::UnexpectedEndOfFile;
			return nullptr;
		}

		if( *cur_ == '"' )
		{
			++cur_;
			result_.storage.push_back('\0');
			return static_cast<StringType>(nullptr) + offset;
		}
		else if( *cur_ == '\\' )
		{
			// TODO
		}
		else
		{
			result_.storage.push_back( static_cast<unsigned char>(*cur_) );
			++cur_;
		}
	}
}

void Parser::SkipWhitespaces()
{
	while( cur_ < end_ &&
		( *cur_ == ' ' || *cur_ == '\t' || *cur_ == '\r' || *cur_ == '\n' ) )
		++cur_;

	if( cur_ == end_ )
		result_.error= Result::Error::UnexpectedEndOfFile;
}

void Parser::CorrectPointers_r( ValueBase& value )
{

	switch( value.type )
	{
	case ValueBase::Type::Null:
		break;

	case ValueBase::Type::Object:
		{
			ObjectValue& object_value= static_cast<ObjectValue&>(value);

			const size_t entries_offset=
				reinterpret_cast<const unsigned char*>(object_value.sub_objects) - static_cast<const unsigned char*>(nullptr);
			object_value.sub_objects= reinterpret_cast<const ObjectValue::ObjectEntry*>( entries_offset + result_.storage.data() );

			for( size_t i= 0u; i < object_value.object_count; i++ )
			{
				ObjectValue::ObjectEntry& entry= const_cast<ObjectValue::ObjectEntry&>(object_value.sub_objects[i]);
				entry.key= CorrectStringPointer( entry.key );

				const size_t offset=
					reinterpret_cast<const unsigned char*>(entry.value) - static_cast<const unsigned char*>(nullptr);
				entry.value= reinterpret_cast<const ValueBase*>( offset + result_.storage.data() );

				CorrectPointers_r( const_cast<ValueBase&>(*entry.value) );
			}
		}
		return;

	case ValueBase::Type::Array:
		{
			ArrayValue& array_value= static_cast<ArrayValue&>(value);

			const size_t objects_offset=
				reinterpret_cast<const unsigned char*>(array_value.objects) - static_cast<const unsigned char*>(nullptr);
			array_value.objects= reinterpret_cast<const ValueBase* const*>( objects_offset + result_.storage.data() );

			for( size_t i= 0u; i < array_value.object_count; i++ )
			{
				const size_t offset=
					reinterpret_cast<const unsigned char*>(array_value.objects[i]) - static_cast<const unsigned char*>(nullptr);
				const_cast<const ValueBase*&>(array_value.objects[i])= reinterpret_cast<const ValueBase*>( offset + result_.storage.data() );

				CorrectPointers_r( const_cast<ValueBase&>(*array_value.objects[i]) );
			}
		}
		break;

	case ValueBase::Type::String:
		{
			StringValue& string_value= static_cast<StringValue&>(value);
			string_value.str= CorrectStringPointer( string_value.str );
		}
		break;

	case ValueBase::Type::Number:
		{
			NumberValue& number_value= static_cast<NumberValue&>(value);
			number_value.str= CorrectStringPointer( number_value.str );
		}
		break;

	case ValueBase::Type::Bool:
		break;
	};
}

StringType Parser::CorrectStringPointer( const StringType str )
{
	const size_t offset=
		reinterpret_cast<const unsigned char*>(str) - static_cast<const unsigned char*>(nullptr);
	return reinterpret_cast<const char*>( offset + result_.storage.data() );
}

Parser::Result Parser::Parse( const char* const json_text, const size_t json_text_length )
{
	start_= json_text;
	end_= json_text + json_text_length;
	cur_= start_;

	result_.error= Result::Error::NoError;
	result_.error_pos= 0u;
	result_.storage.clear();

	const ValueBase* root= Parse_r();
	if( result_.error == Result::Error::NoError )
	{
		const size_t offset=
			reinterpret_cast<const unsigned char*>(root) - static_cast<const unsigned char*>(nullptr);
		root= reinterpret_cast<const ValueBase*>( offset + result_.storage.data() );

		CorrectPointers_r( const_cast<ValueBase&>(*root) );
		result_.root= Value( root );
	}
	else
	{
		result_.error_pos= cur_ - start_;
		result_.root= Value();
	}

	return std::move(result_);
}

} // namespace PanzerJson
