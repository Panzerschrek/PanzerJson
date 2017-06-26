#include <algorithm>
#include <cstring>

#include "../include/PanzerJson/parser.hpp"

namespace PanzerJson
{

// For big powers.
// Method is faster for big powers - complexity is about O(log2(power)).
static double TenPowerDouble( const unsigned int power ) noexcept
{
	if( power == 0u )
		return 1.0;
	if( power == 1u )
		return 10.0;

	const double half_power= TenPowerDouble( power / 2u );
	double result= half_power * half_power;
	if( ( power & 1u ) != 0u )
		result*= 10.0;

	return result;
}

static constexpr size_t PtrAlignedSize( const size_t size ) noexcept
{
	return ( size + ( sizeof(void*) - 1u ) ) & ~( sizeof(void*) - 1u );
}

static constexpr size_t NumberAlignedSize( const size_t size ) noexcept
{
	return ( size + ( sizeof(double) - 1u ) ) & ~( sizeof(double) - 1u );
}


template<class T>
static constexpr size_t PtrAlignedSize() noexcept
{
	return PtrAlignedSize( sizeof(T) );
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

		const size_t object_entries_stack_pos= object_entries_stack_.size();

		if( *cur_ == '}' )
			++cur_;
		else
		{
			while(true)
			{
				SkipWhitespaces();
				if( result_.error != Result::Error::NoError )
					return nullptr;
				if( *cur_ != '"' )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
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

				object_entries_stack_.emplace_back( ObjectValue::ObjectEntry{ key, value } );

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
		}

		const size_t entries_count= object_entries_stack_.size() - object_entries_stack_pos;
		const size_t offset= result_.storage.size();
		result_.storage.resize(
			result_.storage.size() +
			sizeof(ObjectValue) +
			sizeof(ObjectValue::ObjectEntry) * entries_count );

		ObjectValue* const object_value= reinterpret_cast<ObjectValue*>( result_.storage.data() + offset );
		object_value->type= ValueBase::Type::Object;
		object_value->object_count= static_cast<uint32_t>(entries_count);

		std::memcpy(
			result_.storage.data() + offset + sizeof(ObjectValue),
			object_entries_stack_.data() + object_entries_stack_pos,
			sizeof(ObjectValue::ObjectEntry) * entries_count );

		object_entries_stack_.resize(object_entries_stack_pos);

		return reinterpret_cast<ObjectValue*>( static_cast<char*>(nullptr) + offset );
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

		const size_t array_elements_stack_pos= array_elements_stack_.size();

		if( *cur_ == ']' )
		{
			++cur_;
		}
		else
		{
			while(true)
			{
				SkipWhitespaces();
				if( result_.error != Result::Error::NoError )
					return nullptr;

				const ValueBase* const value= Parse_r();
				if( result_.error != Result::Error::NoError )
					return nullptr;

				array_elements_stack_.push_back(value);

				SkipWhitespaces();
				if( result_.error != Result::Error::NoError )
					return nullptr;
				if( *cur_ == ',' )
				{
					++cur_;
					continue;
				}
				else if( *cur_ == ']' )
				{
					++cur_;
					break;
				}
				else
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
			} // while true
		}

		const size_t element_count= array_elements_stack_.size() - array_elements_stack_pos;
		const size_t offset= result_.storage.size();
		result_.storage.resize(
			result_.storage.size() +
			sizeof(ArrayValue) +
			sizeof(const ValueBase*) * element_count );

		ArrayValue* const array_value= reinterpret_cast<ArrayValue*>( result_.storage.data() + offset );
		array_value->type= ValueBase::Type::Array;
		array_value->object_count= static_cast<uint32_t>(element_count);

		std::memcpy(
			result_.storage.data() + offset + sizeof(ArrayValue),
			array_elements_stack_.data() + array_elements_stack_pos,
			sizeof(const ValueBase*) * element_count );

		array_elements_stack_.resize(array_elements_stack_pos);

		return reinterpret_cast<ArrayValue*>( static_cast<char*>(nullptr) + offset );
	}
	break;

	// String
	case '"':
		{
			// Allocate StringValue, then parse string.
			// In result, string storage will be exactly after StringValue.
			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + sizeof(StringValue) );

			ParseString();
			if( result_.error != Result::Error::NoError )
				return nullptr;

			StringValue* const string_value=
				reinterpret_cast<StringValue*>( result_.storage.data() + offset );
			string_value->type= ValueBase::Type::String;

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
			constexpr unsigned int c_max_exponent= 65536u; // Max reasonable exponent.

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

			// Integer part.
			if ( *cur_ == '0')
			{
				// If leading digit is 0, than number contains no other integer part digits.
				++cur_;
			}
			else
				extract_digits();
			int decimal_point_pos= static_cast<int>(number_digits_.size());

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

				unsigned int exponent= 0u;
				while( cur_ < end_ && *cur_ >= '0' && *cur_ <= '9' )
				{
					const unsigned int mul10= exponent * 10u;
					if( mul10 / 10u != exponent || mul10 < exponent ) // Detect overflow.
					{
						exponent= c_max_exponent;
						break;
					}

					const unsigned int add_digit= mul10 + static_cast<unsigned int>( *cur_ - '0' );
					if( add_digit < mul10 ) // Detect overflow.
					{
						exponent= c_max_exponent;
						break;
					}

					exponent= add_digit;
					++cur_;
				}

				const int final_expoent= static_cast<int>( std::min( exponent, c_max_exponent ) );
				if( exponent_is_negative )
					decimal_point_pos-= final_expoent;
				else
					decimal_point_pos+= final_expoent;
			}

			num_parse_end:

			int64_t result_int_val;
			double result_double_val;
			{
				uint64_t result_val= 0u;
				for( int i= 0; i < std::min( decimal_point_pos, static_cast<int>(number_digits_.size()) ); i++ )
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
				for( int i= static_cast<int>(number_digits_.size()); i < decimal_point_pos; i++ )
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

				const int exponent= decimal_point_pos - static_cast<int>(number_digits_.size());
				if( exponent >= 0 )
					result_double_val*= TenPowerDouble(static_cast<unsigned int>(+exponent));
				else
					result_double_val/= TenPowerDouble(static_cast<unsigned int>(-exponent));

				if( is_negative )
					result_double_val*= -1.0;
			}

			// Allocate number value.
			// All storage is pointer-aligned, but numbers requires double-alignment, which can be bigger, than pointer alignment.
			result_.storage.resize( NumberAlignedSize( result_.storage.size() ) );
			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + sizeof(NumberValue) );
			NumberValue* const value= reinterpret_cast<NumberValue*>( result_.storage.data() + offset );

			// Fill data.
			value->type= ValueBase::Type::Number;
			value->int_value= result_int_val;
			value->double_value= result_double_val;

			// Allocate string value.
			if( save_number_strings_ )
			{
				const size_t str_size= cur_ - num_start;
				const size_t str_offset= result_.storage.size();
				result_.storage.resize( result_.storage.size() + PtrAlignedSize( str_size + 1u ) );
				std::memcpy( result_.storage.data() + str_offset, num_start, str_size );
				result_.storage[ str_offset + str_size ]= '\0';
			}

			return reinterpret_cast<NumberValue*>( static_cast<char*>(nullptr) + offset );
		}
		// Null
		else if( *cur_ == 'n' )
		{
			if( end_ - cur_ < 4 )
			{
				result_.error= Result::Error::UnexpectedEndOfFile;
				return nullptr;
			}
			if( std::strncmp( cur_, "null", 4 ) != 0 )
			{
				result_.error= Result::Error::UnexpectedLexem;
				return nullptr;
			}
			cur_+= 4;

			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + PtrAlignedSize<NullValue>() );
			NullValue* const value= reinterpret_cast<NullValue*>( result_.storage.data() + offset );

			value->type= ValueBase::Type::Null;

			return reinterpret_cast<NullValue*>( static_cast<char*>(nullptr) + offset );

		}
		else if( *cur_ == 't' || *cur_ == 'f' )
		{
			bool bool_value;
			if( *cur_ == 't' )
			{
				if( end_ - cur_ < 4 )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				if( std::strncmp( cur_, "true", 4 ) != 0 )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
				cur_+= 4u;
				bool_value= true;
			}
			else
			{
				if( end_ - cur_ < 5 )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return nullptr;
				}
				if( std::strncmp( cur_, "false", 5 ) != 0 )
				{
					result_.error= Result::Error::UnexpectedLexem;
					return nullptr;
				}
				cur_+= 5u;
				bool_value= false;
			}

			const size_t offset= result_.storage.size();
			result_.storage.resize( result_.storage.size() + PtrAlignedSize<BoolValue>() );
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
			 // Reconstruct alignment.
			// Stringrs are only objects in storage, which is not pointer-aligned.
			result_.storage.resize( PtrAlignedSize( result_.storage.size() ) );
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
				{
					if( end_ - cur_ < 5 )
					{
						result_.error= Result::Error::UnexpectedEndOfFile;
						return nullptr;
					}
					++cur_;

					// Parse hex number.
					size_t char_code= 0u;
					for( size_t i= 0u; i < 4u; i++ )
					{
						size_t digit;
						if( cur_[i] >= '0' && cur_[i] <= '9' )
							digit= size_t( cur_[i] - '0' );
						else if( cur_[i] >= 'a' && cur_[i] <= 'f' )
							digit= size_t( cur_[i] - 'a' + 10 );
						else if( cur_[i] >= 'A' && cur_[i] <= 'F' )
							digit= size_t( cur_[i] - 'A' + 10 );
						else
						{
							result_.error= Result::Error::UnexpectedLexem;
							return nullptr;
						}
						char_code|= digit << ( ( 3u - i ) * 4u );
					}

					// Convert to UTF-8.
					// Change this, if string format changed.
					if( char_code <= 0x7Fu )
						result_.storage.push_back( static_cast<unsigned char>( char_code ) );
					else if( char_code <= 0x7FFu )
					{
						const size_t b0= 0xC0u | ( char_code >> 6u );
						const size_t b1= 0x80u | ( char_code & 0x3Fu );
						result_.storage.push_back( static_cast<unsigned char>(b0) );
						result_.storage.push_back( static_cast<unsigned char>(b1) );
					}
					else// if( char_code <= 0xFFFFu )
					{
						const size_t b0= 0xE0u | (         ( char_code >> 12u ) );
						const size_t b1= 0x80u | ( 0x3Fu & ( char_code >>  6u ) );
						const size_t b2= 0x80u | ( 0x3Fu & ( char_code >>  0u ) );
						result_.storage.push_back( static_cast<unsigned char>(b0) );
						result_.storage.push_back( static_cast<unsigned char>(b1) );
						result_.storage.push_back( static_cast<unsigned char>(b2) );
					}

					cur_+= 4u;
				}
				break;

			default:
				result_.error= Result::Error::UnexpectedLexem;
				return nullptr;
			};
		}
		else
		{
			// Check, if control character.
			// TODO - maybe change this criteria?
			if( ( *cur_ >= 0x00 && *cur_ < 0x20 ) || *cur_ == 0x7f )
			{
				result_.error= Result::Error::ControlCharacterInsideString;
				return nullptr;
			}

			result_.storage.push_back( static_cast<unsigned char>(*cur_) );
			++cur_;
		}
	}
}

void Parser::SkipWhitespaces()
{
	if( !enable_comments_ )
	{
		while( cur_ < end_ &&
			( *cur_ == ' ' || *cur_ == '\t' || *cur_ == '\r' || *cur_ == '\n' ) )
			++cur_;
	}
	else
	{
		while(true)
		{
			// Whitespaces first.
			while( cur_ < end_ &&
				( *cur_ == ' ' || *cur_ == '\t' || *cur_ == '\r' || *cur_ == '\n' ) )
				++cur_;

			if( cur_ == end_ )
				break;

			// Then comments.
			if( *cur_ == '/' )
			{
				++cur_;
				if( cur_ == end_ )
				{
					result_.error= Result::Error::UnexpectedEndOfFile;
					return;
				}

				// Comment
				if( *cur_ == '/' )
				{
					++cur_;
					while( cur_ < end_ && *cur_ != '\n')
						++cur_;
				}
				else if( *cur_ == '*' ) /* comment */
				{
					++cur_;
					while( cur_ < end_minus_one_ )
					{
						if( cur_[0] == '*' && cur_[1] == '/' )
						{
							cur_+= 2u;
							break;
						}
						++cur_;
					}
					if( cur_ >= end_minus_one_ )
					{
						result_.error= Result::Error::UnexpectedEndOfFile;
						return;
					}
				}
				else
				{
					result_.error= Result::Error::UnexpectedLexem;
					return;
				}
			}
			else
				break;
		}
	}

	if( cur_ == end_ )
		result_.error= Result::Error::UnexpectedEndOfFile;
}

void Parser::SkipWhitespacesAtEnd()
{
	SkipWhitespaces();

	if (cur_ == end_)
		result_.error= Result::Error::NoError;
	else
		result_.error= Result::Error::ExtraCharactersAfterJsonRoot;
}

void Parser::CorrectPointers_r( ValueBase& value )
{
	switch( value.type )
	{
	case ValueBase::Type::Null:
	case ValueBase::Type::String:
	case ValueBase::Type::Number:
	case ValueBase::Type::Bool:
		break;

	case ValueBase::Type::Object:
		{
			ObjectValue& object_value= static_cast<ObjectValue&>(value);

			for( size_t i= 0u; i < object_value.object_count; i++ )
			{
				ObjectValue::ObjectEntry& entry= const_cast<ObjectValue::ObjectEntry&>(object_value.GetEntries()[i]);
				entry.key= CorrectStringPointer( entry.key );

				const size_t offset=
					reinterpret_cast<const unsigned char*>(entry.value) - static_cast<const unsigned char*>(nullptr);
				entry.value= reinterpret_cast<const ValueBase*>( offset + result_.storage.data() );

				CorrectPointers_r( const_cast<ValueBase&>(*entry.value) );
			}

			std::sort(
				const_cast<ObjectValue::ObjectEntry*>(object_value.GetEntries()),
				const_cast<ObjectValue::ObjectEntry*>(object_value.GetEntries()) + object_value.object_count,
				[](const ObjectValue::ObjectEntry& l, const ObjectValue::ObjectEntry& r ) -> bool
				{
					return StringCompare( l.key, r.key ) < 0;
				} );
		}
		return;

	case ValueBase::Type::Array:
		{
			ArrayValue& array_value= static_cast<ArrayValue&>(value);

			for( size_t i= 0u; i < array_value.object_count; i++ )
			{
				const size_t offset=
					reinterpret_cast<const unsigned char*>(array_value.GetElements()[i]) - static_cast<const unsigned char*>(nullptr);
				const_cast<const ValueBase*&>(array_value.GetElements()[i])= reinterpret_cast<const ValueBase*>( offset + result_.storage.data() );

				CorrectPointers_r( const_cast<ValueBase&>( *array_value.GetElements()[i]) );
			}
		}
		break;
	};
}

StringType Parser::CorrectStringPointer( const StringType str )
{
	const size_t offset=
		reinterpret_cast<const unsigned char*>(str) - static_cast<const unsigned char*>(nullptr);
	return reinterpret_cast<const char*>( offset + result_.storage.data() );
}

Parser::ResultPtr Parser::Parse( const char* const json_text_null_teriminated )
{
	return Parse( json_text_null_teriminated, size_t(std::strlen(json_text_null_teriminated)) );
}

Parser::ResultPtr Parser::Parse( const char* const json_text, const size_t json_text_length )
{
	start_= json_text;
	end_= json_text + json_text_length;
	end_minus_one_= end_ - 1u;
	cur_= start_;

	result_.error= Result::Error::NoError;
	result_.error_pos= 0u;

	array_elements_stack_.clear();
	object_entries_stack_.clear();

	const ValueBase* root= Parse_r();
	bool all_ok= false;

	if( result_.error == Result::Error::NoError )
	{
		SkipWhitespacesAtEnd();
		if( result_.error == Result::Error::NoError )
		{
			const size_t offset=
				reinterpret_cast<const unsigned char*>(root) - static_cast<const unsigned char*>(nullptr);
			root= reinterpret_cast<const ValueBase*>( offset + result_.storage.data() );
			CorrectPointers_r( const_cast<ValueBase&>(*root) );

			if( enable_noncomposite_json_root_ ||
				root->type == ValueBase::Type::Array || root->type == ValueBase::Type::Object )
			{
				result_.root= Value( root );
				all_ok= true;
			}
			else
				result_.error= Result::Error::RootIsNotObjectOrArray;
		}
	}

	if( !all_ok )
	{
		result_.error_pos= cur_ - start_;
		result_.root= Value();
	}

	std::unique_ptr<Result> result( new Result );
	result->error= result_.error;
	result->error_pos= result_.error_pos;
	result->root= result_.root;
	result->storage.swap( result_.storage );

	return std::move(result);
}

void Parser::SetEnableNoncompositeJsonRoot( const bool enable ) noexcept
{
	enable_noncomposite_json_root_= enable;
}

bool Parser::GetEnableNoncompositeJsonRoot() const noexcept
{
	return enable_noncomposite_json_root_;
}

void Parser::SetEnableComments( const bool enable ) noexcept
{
	enable_comments_= enable;
}

bool Parser::GetEnableCommetns() const noexcept
{
	return enable_comments_;
}

void Parser::SetSaveNumberStrings( const bool save ) noexcept
{
	save_number_strings_= save;
}

bool Parser::GetSaveNumberStrings() const noexcept
{
	return save_number_strings_;
}

void Parser::ResetCaches()
{
	number_digits_.clear();
	number_digits_.shrink_to_fit();

	array_elements_stack_.clear();
	array_elements_stack_.shrink_to_fit();
	object_entries_stack_.clear();
	object_entries_stack_.shrink_to_fit();
}

} // namespace PanzerJson
