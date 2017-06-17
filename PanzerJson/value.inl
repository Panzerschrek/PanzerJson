#pragma once

namespace PanzerJson
{


template<class Stream>
void Value::SerializeString( Stream& stream, StringType str )
{
	// Change this if string type chaged.

	// Produce escaped symbols, quotes.

	stream << '"';

	const char* s= str;
	while( *s != '\0' )
	{
		if( *s == '"' )
			stream << "\"";
		else if( *s == '\\' )
			stream << "\\\\";
		else if( *s == '/' )
			stream << "\\/";
		else if( *s == '\b' )
			stream << "\\b";
		else if( *s == '\f' )
			stream << "\\f";
		else if( *s == '\n' )
			stream << "\\n";
		else if( *s == '\r' )
			stream << "\\r";
		else if( *s == '\t' )
			stream << "\\t";
		else
			stream << *s;

		++s;
	}

	stream << '"';
}

template<class Stream>
void Value::Serialize_r( Stream& stream, const ValueBase& value, const size_t tab )
{
	(void)tab; // TODO - use tabs.

	switch(value.type)
	{
	case ValueBase::Type::Null:
		stream << "null";
		break;

	case ValueBase::Type::Object:
	{
		const ObjectValue& object= static_cast<const ObjectValue&>(value);

		stream << "{";

		for( size_t i= 0u; i < object.object_count; i++ )
		{
			SerializeString( stream, object.sub_objects[i].key );
			stream << ":";
			Serialize_r( stream, *object.sub_objects[i].value, tab + 1 );
			if( i < object.object_count - 1u )
				stream << ",";
		}

		stream << "}";
	}
	break;

	case ValueBase::Type::Array:
		{
			const ArrayValue& array= static_cast<const ArrayValue&>(value);

			stream << "[";

			for( size_t i= 0u; i < array.object_count; i++ )
			{
				Serialize_r( stream, *array.objects[i], tab + 1 );
				if( i < array.object_count - 1u )
					stream << ",";
			}

			stream << "]";
		}
		break;

	case ValueBase::Type::String:
		SerializeString( stream, static_cast<const StringValue&>(value).str );
		break;

	case ValueBase::Type::Number:
		stream << static_cast<const NumberValue&>(value).str;
		break;

	case ValueBase::Type::Bool:
		stream << ( static_cast<const BoolValue&>(value).value ? "true" : "false" );
		break;
	};
}

template<class Stream>
void Value::Serialize( Stream& stream )
{
	Serialize_r( stream, *value_, 0u );
}


} // namespace PanzerJson
