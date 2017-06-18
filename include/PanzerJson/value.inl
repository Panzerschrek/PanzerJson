#pragma once
#include <cstring>

namespace PanzerJson
{

// Add some inline methods for builds without link-time optimization.

inline ValueBase::Type Value::GetType() const noexcept
{
	return value_->type;
}

inline bool Value::IsNull() const noexcept
{
	return GetType() == ValueBase::Type::Null;
}

inline bool Value::IsObject() const noexcept
{
	return GetType() == ValueBase::Type::Object;
}

inline bool Value::IsArray() const noexcept
{
	return GetType() == ValueBase::Type::Array;
}

inline bool Value::IsString() const noexcept
{
	return GetType() == ValueBase::Type::String;
}

inline bool Value::IsNumber() const noexcept
{
	return GetType() == ValueBase::Type::Number;
}

inline bool Value::IsBool() const noexcept
{
	return GetType() == ValueBase::Type::Bool;
}

inline Value::UniversalIterator Value::cbegin() const noexcept
{
	return begin();
}

inline Value::UniversalIterator Value::cend() const noexcept
{
	return end();
}

inline Value::ArrayIterator Value::array_cbegin() const noexcept
{
	return array_begin();
}

inline Value::ArrayIterator Value::array_cend() const noexcept
{
	return array_end();
}

inline Value::ObjectIterator Value::object_cbegin() const noexcept
{
	return object_begin();
}

inline Value::ObjectIterator Value::object_cend() const noexcept
{
	return object_end();
}

// UniversalIterator

inline Value::UniversalIterator::UniversalIterator( const ValueBase::Type type, const Ptr ptr ) noexcept
	: type_(type)
	, ptr_(ptr)
{
	// TODO - assert, if type is not array or object.
}

inline bool Value::UniversalIterator::operator==( const UniversalIterator& other ) const noexcept
{
	return type_ == other.type_ && std::memcmp( &ptr_, &other.ptr_, sizeof(Ptr) ) == 0;
}

inline bool Value::UniversalIterator::operator!=( const UniversalIterator& other ) const noexcept
{
	return !( *this == other );
}

inline Value::UniversalIterator& Value::UniversalIterator::operator++() noexcept
{
	if( type_ == ValueBase::Type::Array )
		++ptr_.array_value;
	else
		++ptr_.object_entry;
	return *this;
}

inline Value::UniversalIterator& Value::UniversalIterator::operator--() noexcept
{
	if( type_ == ValueBase::Type::Array )
		--ptr_.array_value;
	else
		--ptr_.object_entry;
	return *this;
}

inline Value::UniversalIterator Value::UniversalIterator::operator++(int) noexcept
{
	UniversalIterator result= *this;
	++*this;
	return result;
}

inline Value::UniversalIterator Value::UniversalIterator::operator--(int) noexcept
{
	UniversalIterator result= *this;
	--*this;
	return result;
}

inline Value Value::UniversalIterator::operator*() const noexcept
{
	if( type_ == ValueBase::Type::Array )
		return Value( *ptr_.array_value );
	else
		return Value( ptr_.object_entry->value );
}

// ArrayIterator

inline Value::ArrayIterator::ArrayIterator( const ValueBase* const* const ptr ) noexcept
	: ptr_(ptr)
{}

inline bool Value::ArrayIterator::operator==( const ArrayIterator& other ) const noexcept
{
	return ptr_ == other.ptr_;
}

inline bool Value::ArrayIterator::operator!=( const ArrayIterator& other ) const noexcept
{
	return !( *this == other );
}

inline Value::ArrayIterator& Value::ArrayIterator::operator++() noexcept
{
	++ptr_;
	return *this;
}

inline Value::ArrayIterator& Value::ArrayIterator::operator--() noexcept
{
	--ptr_;
	return *this;
}

inline Value::ArrayIterator Value::ArrayIterator::operator++(int) noexcept
{
	ArrayIterator result= *this;
	++*this;
	return result;
}

inline Value::ArrayIterator Value::ArrayIterator::operator--(int) noexcept
{
	ArrayIterator result= *this;
	--*this;
	return result;
}

inline Value Value::ArrayIterator::operator*() const noexcept
{
	return Value( *ptr_ );
}

// ObjectIterator

inline Value::ObjectIterator::ObjectIterator( const ObjectValue::ObjectEntry* const ptr ) noexcept
	: ptr_(ptr)
{}

inline bool Value::ObjectIterator::operator==( const ObjectIterator& other ) const noexcept
{
	return ptr_ == other.ptr_;
}

inline bool Value::ObjectIterator::operator!=( const ObjectIterator& other ) const noexcept
{
	return !( *this == other );
}

inline Value::ObjectIterator& Value::ObjectIterator::operator++() noexcept
{
	++ptr_;
	return *this;
}

inline Value::ObjectIterator& Value::ObjectIterator::operator--() noexcept
{
	--ptr_;
	return *this;
}

inline Value::ObjectIterator Value::ObjectIterator::operator++(int) noexcept
{
	ObjectIterator result= *this;
	++*this;
	return result;
}

inline Value::ObjectIterator Value::ObjectIterator::operator--(int) noexcept
{
	ObjectIterator result= *this;
	--*this;
	return result;
}

inline Value::ObjectIterator::value_type Value::ObjectIterator::operator*() const noexcept
{
	return value_type( ptr_->key, Value( ptr_->value ) );
}

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
			stream << "\\\"";
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
