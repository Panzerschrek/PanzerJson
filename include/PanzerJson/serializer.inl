#include "../../src/serializers_common.hpp"

namespace PanzerJson
{

template<class Stream>
void Serializer::Serialize_r( Stream& stream, const ValueBase& value )
{
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
			SerializeString( stream, object.GetEntries()[i].key );
			stream << ":";
			Serialize_r( stream, *object.GetEntries()[i].value );
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
				Serialize_r( stream, *array.GetElements()[i] );
				if( i < array.object_count - 1u )
					stream << ",";
			}

			stream << "]";
		}
		break;

	case ValueBase::Type::String:
		SerializeString( stream, static_cast<const StringValue&>(value).GetString() );
		break;

	case ValueBase::Type::Number:
		{
			const NumberValue& number_value= static_cast<const NumberValue&>(value);
			if( number_value.has_string )
				stream << number_value.GetString();
			else
			{
				GenNumberValueString( number_value );
				stream << num_str_.data();
			}
		}
		break;

	case ValueBase::Type::Bool:
		stream << ( static_cast<const BoolValue&>(value).value ? "true" : "false" );
		break;
	};
}

template<class Stream>
void Serializer::Serialize( const Value value, Stream& stream )
{
	Serialize_r( stream, *value.GetInternalValue() );
}

} // namespace PanzerJson
