#include <algorithm>
#include <cstring>

#include "../include/PanzerJson/parser.hpp"

namespace PanzerJson
{

// For big powers.
// Method is faster for big powers - complexity is about O(log2(power)).
static double TenPowerDouble( const uint64_t power )
{
	if( power == 0 )
		return 1.0;
	if( power == 1 )
		return 10.0;

	const double half_power= TenPowerDouble( power / 2u );
	double result= half_power * half_power;
	if( ( power & 1u ) != 0 )
		result*= 10.0;

	return result;
}

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
			bool is_negative= false;

			constexpr uint64_t c_max_uint64= std::numeric_limits<uint64_t>::max();
			constexpr uint64_t c_max_abs_int64= c_max_uint64 / 2u + 1u;

			number_digits_.clear();
			number_digits_.reserve(64u);

			const auto extract_digits=
			[&]()
			{
				while( cur_ < end_ && *cur_ >= '0' && *cur_ <= '9' )
				{
					number_digits_.push_back( *cur_ - '0' );
					++cur_;
				}
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

				is_negative= true;
			}

			extract_digits(); // Integer part.
			int64_t decimal_point_pos= number_digits_.size();

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

				extract_digits();
			}
			if( cur_ == end_ )
				goto num_parse_end;
			if( *cur_ == 'e' || *cur_ == 'E' )
			{
				bool exponent_is_negative= false;

				++cur_;
				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}

				if( *cur_ == '+' || *cur_ == '-' )
				{
					if( *cur_ == '-' ) exponent_is_negative= true;
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

				uint64_t exponent= 0u;
				while( cur_ < end_ && *cur_ >= '0' && *cur_ <= '9' )
				{
					const uint64_t mul10= exponent * 10u;
					if( mul10 / 10u != exponent || mul10 < exponent ) // Detect overflow.
					{
						exponent= c_max_uint64;
						break;
					}

					const uint64_t add_digit= mul10 + uint64_t( *cur_ - '0' );
					if( add_digit < mul10 ) // Detect overflow.
					{
						exponent= c_max_uint64;
						break;
					}

					exponent= add_digit;
					++cur_;
				}

				if( exponent_is_negative )
					decimal_point_pos-= static_cast<int64_t>(exponent);
				else
					decimal_point_pos+= static_cast<int64_t>(exponent);
			}

			num_parse_end:

			int64_t result_int_val;
			double result_double_val;
			{
				uint64_t result_val= 0u;
				for( int64_t i= 0u; i < std::min( decimal_point_pos, int64_t(number_digits_.size()) ); i++ )
				{
					const uint64_t mul10= result_val * 10u;
					if( mul10 / 10u != result_val || mul10 < result_val ) // Detect overflow.
					{
						result_val= c_max_uint64;
						break;
					}

					const uint64_t digit= int64_t(number_digits_[i]);
					const uint64_t add_digit= mul10 + digit;
					if( add_digit < mul10 ) // Detect overflow.
					{
						result_val= c_max_uint64;
						break;
					}

					result_val= add_digit;
				}
				for( int64_t i= int64_t(number_digits_.size()); i < decimal_point_pos; i++ )
				{
					const uint64_t mul10= result_val * 10u;
					if( mul10 / 10u != result_val || mul10 < result_val ) // Detect overflow.
					{
						result_val= c_max_uint64;
						break;
					}
					result_val= mul10;
				}

				if( is_negative )
					result_int_val= -static_cast<int64_t>( std::min( result_val, c_max_abs_int64 ) );
				else
					result_int_val= +static_cast<int64_t>( result_val );
			}
			{
				result_double_val= 0u;
				for( const unsigned char digit : number_digits_ )
					result_double_val= result_double_val * 10.0 + double(digit);

				const int64_t exponent= decimal_point_pos - static_cast<int64_t>(number_digits_.size());
				if( exponent >= 0 )
					result_double_val*= TenPowerDouble(static_cast<uint64_t>(+exponent));
				else
					result_double_val/= TenPowerDouble(static_cast<uint64_t>(-exponent));

				if( is_negative )
					result_double_val*= -1.0;
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
		else
		{
			result_.error= Result::Error::UnexpectedLexem;
			return nullptr;
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
			++cur_;
			if( cur_ == end_ )
			{
				result_.error= Result::Error::UnexpectedEndOfFile;
				return nullptr;
			}

			switch(*cur_)
			{
			case '"':
			case '\\':
			case '/':
				result_.storage.push_back(*cur_);
				++cur_;
				break;

			case 'b':
				result_.storage.push_back('\b');
				++cur_;
				break;
			case 'f':
				result_.storage.push_back('\f');
				++cur_;
				break;
			case 'n':
				result_.storage.push_back('\n');
				++cur_;
				break;
			case 'r':
				result_.storage.push_back('\r');
				++cur_;
				break;
			case 't':
				result_.storage.push_back('\t');
				++cur_;
				break;

			case 'u':
				if( end_ - cur_ < 5 )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				cur_+= 5;
				result_.storage.push_back('?'); // TODO - support unicode.
				break;

			default:
				result_.error= Result::Error::UnexpectedLexem;
				return nullptr;
			};
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

			std::sort(
				const_cast<ObjectValue::ObjectEntry*>(object_value.sub_objects),
				const_cast<ObjectValue::ObjectEntry*>(object_value.sub_objects) + object_value.object_count,
				[](const ObjectValue::ObjectEntry& l, const ObjectValue::ObjectEntry& r ) -> bool
				{
					return StringCompare( l.key, r.key ) < 0;
				} );
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
