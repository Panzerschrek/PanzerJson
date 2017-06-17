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

	const char c= *cur_;
	++cur_;

	switch(c)
	{

	// Object
	case '{':
	{
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
		result_.storage.resize( sizeof(ObjectValue::ObjectEntry) * entries.size() );
		ObjectValue::ObjectEntry* const result_entries=
			reinterpret_cast<ObjectValue::ObjectEntry*>( result_.storage.data() + entries_offset );
		std::memcpy( result_entries, entries.data(), sizeof(ObjectValue::ObjectEntry) * entries.size() );

		const size_t object_offset= result_.storage.size();
		result_.storage.resize( sizeof(ObjectValue) );
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
		std::vector<const ValueBase*> values;

		while(true)
		{
			SkipWhitespaces();
			if( result_.error != Result::Error::NoError )
				return nullptr;
			if( *cur_ == ']' )
			{
				++cur_;
				continue;
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
		result_.storage.resize( sizeof(const ValueBase*) * values.size() );
		const ValueBase* * const array_values=
			reinterpret_cast<const ObjectValue::ValueBase**>( result_.storage.data() + values_offset );
		std::memcpy( array_values, values.data(), sizeof(const ValueBase*) * values.size() );

		const size_t array_offset= result_.storage.size();
		result_.storage.resize( sizeof(ArrayValue) );
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
			result_.storage.resize( sizeof(StringValue) );
			StringValue* const string_value=
				reinterpret_cast<StringValue*>( result_.storage.data() + offset );

			string_value->type= ValueBase::Type::String;
			string_value->str= str;

			return reinterpret_cast<StringValue*>( static_cast<char*>(nullptr) + offset );

		}
		break;

	default:
		// Number
		if( c >= '0' && c <= '9' )
		{
		}
		// Null
		else if( c == 'n' )
		{
		}
		// True
		else if( c == 't' )
		{
		}
		// False
		else if( c == 'f' )
		{
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

	while(1)
	{
		if( cur_ == end_ )
		{
			result_.error= Result::Error::UnexpectedEndOfFile;
			return nullptr;
		}

		if( *cur_ == '"' )
		{
			result_.storage.push_back('\0');
			return static_cast<StringType>(nullptr) + offset;
		}
		else if( *cur_ == '\\' )
		{
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

			for( size_t i= 0u; i < object_value.sub_objects; i++ )
			{

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
			const size_t offset=
				reinterpret_cast<const unsigned char*>(string_value.str) - static_cast<const unsigned char*>(nullptr);
			string_value.str= reinterpret_cast<const char*>( offset + result_.storage.data() );
		}
		break;

	case ValueBase::Type::Number:
		{
		}
		break;

	case ValueBase::Type::Bool:
		break;
	};
}

Parser::Result Parser::Parse( const char* const json_text, const size_t json_text_length )
{
	start_= json_text;
	end_= json_text + json_text_length;

	result_.error= Result::Error::NoError;

	const ValueBase* const root= Parse_r();
	CorrectPointers_r( const_cast<ValueBase&>(*root) );
	result_.root= Value( root );

	return result_;
}

} // namespace PanzerJson
