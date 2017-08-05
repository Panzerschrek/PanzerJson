#include "../src/serializers_common.hpp"
#include "../src/panzer_json_assert.hpp"

namespace PanzerJson
{

// StreamedSerializer

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::StreamedSerializer( StreamT& stream )
	: stream_(stream)
{}

template<class StreamT, SerializationFormatting formatting>
typename StreamedSerializer<StreamT, formatting>::ObjectSerializer
StreamedSerializer<StreamT, formatting>::AddObject()
{
	return ObjectSerializer( stream_, 0u );
}

template<class StreamT, SerializationFormatting formatting>
typename StreamedSerializer<StreamT, formatting>::ArraySerializer
StreamedSerializer<StreamT, formatting>::AddArray()
{
	return ArraySerializer( stream_, 0u );
}

// ArraySerializer

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::ArraySerializer::ArraySerializer( StreamT& stream, const size_t parent_indent )
	: stream_( &stream )
	, indent_( parent_indent )
{
	if( formatting == SerializationFormatting::TabIndents )
	{
		PrintIndents();
		stream << '[';
		PrintIndents();
		stream << '\n';
		indent_++;
	}
	else
		stream << '[';
}

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::ArraySerializer::ArraySerializer( ArraySerializer&& other ) noexcept
	: stream_( other.stream_ )
	, element_count_(other.element_count_)
	, indent_(other.indent_)
{
	PJ_ASSERT( other.stream_ != nullptr );
	other.stream_= nullptr;
}

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::ArraySerializer::~ArraySerializer()
{
	if( stream_ != nullptr )
	{
		if( formatting == SerializationFormatting::TabIndents )
		{
			indent_--;
			if( element_count_ > 0u )
				*stream_ << '\n';
			PrintIndents();
		}
		*stream_ << ']';
	}
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNull()
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement();
	*stream_ << "null";
}

template<class StreamT, SerializationFormatting formatting>
typename StreamedSerializer<StreamT, formatting>::ObjectSerializer
StreamedSerializer<StreamT, formatting>::ArraySerializer::AddObject()
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement( true );
	if( formatting == SerializationFormatting::TabIndents )
		*stream_ << '\n';
	return ObjectSerializer( *stream_, indent_ );
}

template<class StreamT, SerializationFormatting formatting>
typename StreamedSerializer<StreamT, formatting>::ArraySerializer
StreamedSerializer<StreamT, formatting>::ArraySerializer::AddArray()
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement( true );
	if( formatting == SerializationFormatting::TabIndents )
		*stream_ << '\n';
	return ArraySerializer( *stream_, indent_ );
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::AddString( const StringType& string )
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement();
	SerializeString( *stream_, string );
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::AddBool( const bool val )
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement();
	*stream_ << val ? "true" : "false";
}

template<class StreamT, SerializationFormatting formatting>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, void >::type
StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNumber( const T number )
{
	AddNumberInternal( static_cast<int64_t>( number ) );
}

template<class StreamT, SerializationFormatting formatting>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, void >::type
StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNumber( const T number )
{
	AddNumberInternal( static_cast<uint64_t>( number ) );
}

template<class StreamT, SerializationFormatting formatting>
template<class T>
typename std::enable_if< std::is_floating_point<T>::value, void >::type
StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNumber( const T number )
{
	AddNumberInternal( static_cast<double>( number ) );
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNumberInternal( const double number )
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement();

	NumberStringStorage num_str;
	GenDoubleValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNumberInternal( const int64_t number )
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement();

	NumberStringStorage num_str;
	GenIntValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::AddNumberInternal( const uint64_t number )
{
	PJ_ASSERT( stream_ != nullptr );
	StartNewElement();

	NumberStringStorage num_str;
	GenUintValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::StartNewElement( const bool new_element_is_composite )
{
	PJ_ASSERT( stream_ != nullptr );

	if( element_count_ != 0u )
	{
		if( formatting == SerializationFormatting::TabIndents && !new_element_is_composite )
			*stream_ << ",\n";
		else
			*stream_ << ',';
	}
	element_count_++;

	PrintIndents();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ArraySerializer::PrintIndents()
{
	PJ_ASSERT( stream_ != nullptr );

	if( formatting == SerializationFormatting::TabIndents )
		for( size_t i= 0u; i < indent_; i++ )
			*stream_ << '\t';
}

// ObjectSerializer

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::ObjectSerializer::ObjectSerializer( StreamT& stream, const size_t parent_indent )
	: stream_( &stream )
	, indent_( parent_indent )
{
	if( formatting == SerializationFormatting::TabIndents )
	{
		PrintIndents();
		stream << '{';
		PrintIndents();
		stream << '\n';
		indent_++;
	}
	else
		stream << '{';
}

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::ObjectSerializer::ObjectSerializer( ObjectSerializer&& other ) noexcept
	: stream_( other.stream_ )
	, element_count_(other.element_count_)
	, indent_(other.indent_)
{
	PJ_ASSERT( other.stream_ != nullptr );
	other.stream_= nullptr;
}

template<class StreamT, SerializationFormatting formatting>
StreamedSerializer<StreamT, formatting>::ObjectSerializer::~ObjectSerializer()
{
	if( stream_ != nullptr )
	{
		if( formatting == SerializationFormatting::TabIndents )
		{
			indent_--;
			if( element_count_ > 0u )
				*stream_ << '\n';
			PrintIndents();
		}
		*stream_ << '}';
	}
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNull( const StringType& key )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey( key );
	*stream_ << "null";
}

template<class StreamT, SerializationFormatting formatting>
typename StreamedSerializer<StreamT, formatting>::ObjectSerializer
StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddObject( const StringType& key )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey( key );
	if( formatting == SerializationFormatting::TabIndents )
		*stream_ << '\n';
	return ObjectSerializer( *stream_, indent_ );
}

template<class StreamT, SerializationFormatting formatting>
typename StreamedSerializer<StreamT, formatting>::ArraySerializer
StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddArray( const StringType& key )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey( key );
	if( formatting == SerializationFormatting::TabIndents )
		*stream_ << '\n';
	return ArraySerializer( *stream_, indent_ );
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddString( const StringType& key, const StringType& string )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey(key);
	SerializeString( *stream_, string );
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddBool( const StringType& key, const bool val )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey(key);
	*stream_ << val ? "true" : "false";
}

template<class StreamT, SerializationFormatting formatting>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, void >::type
StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNumber( const StringType& key, const T number )
{
	AddNumberInternal( key, static_cast<int64_t>( number ) );
}

template<class StreamT, SerializationFormatting formatting>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, void >::type
StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNumber( const StringType& key, const T number )
{
	AddNumberInternal( key, static_cast<uint64_t>( number ) );
}

template<class StreamT, SerializationFormatting formatting>
template<class T>
typename std::enable_if< std::is_floating_point<T>::value, void >::type
StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNumber( const StringType& key, const T number )
{
	AddNumberInternal( key, static_cast<double>( number ) );
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNumberInternal( const StringType& key, const double number )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey(key);

	NumberStringStorage num_str;
	GenDoubleValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNumberInternal( const StringType& key, const int64_t number )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey(key);

	NumberStringStorage num_str;
	GenIntValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::AddNumberInternal( const StringType& key, const uint64_t number )
{
	PJ_ASSERT( stream_ != nullptr );
	WriteKey(key);

	NumberStringStorage num_str;
	GenUintValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::WriteKey( const StringType& key )
{
	StartNewElement();
	SerializeString( *stream_, key );
	if( formatting == SerializationFormatting::TabIndents )
		*stream_ << ": ";
	else
		*stream_ << ':';
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::StartNewElement()
{
	PJ_ASSERT( stream_ != nullptr );

	if( element_count_ != 0u )
	{
		if( formatting == SerializationFormatting::TabIndents )
			*stream_ << ",\n";
		else
			*stream_ << ',';
	}
	element_count_++;

	PrintIndents();
}

template<class StreamT, SerializationFormatting formatting>
void StreamedSerializer<StreamT, formatting>::ObjectSerializer::PrintIndents()
{
	PJ_ASSERT( stream_ != nullptr );

	if( formatting == SerializationFormatting::TabIndents )
		for( size_t i= 0u; i < indent_; i++ )
			*stream_ << '\t';
}

} // namespace PanzerJson
