#include "../src/serializers_common.hpp"
#include "../src/panzer_json_assert.hpp"

namespace PanzerJson
{

// StreamedSerializer

template<class StreamT>
StreamedSerializer<StreamT>::StreamedSerializer( StreamT& stream ) noexcept
	: stream_(stream)
{}

template<class StreamT>
typename StreamedSerializer<StreamT>::ObjectSerializer
StreamedSerializer<StreamT>::AddObject() noexcept
{
	return ObjectSerializer( stream_ );
}

template<class StreamT>
typename StreamedSerializer<StreamT>::ArraySerializer
StreamedSerializer<StreamT>::AddArray() noexcept
{
	return ArraySerializer( stream_ );
}

// ArraySerializer

template<class StreamT>
StreamedSerializer<StreamT>::ArraySerializer::ArraySerializer( StreamT& stream ) noexcept
	: stream_( &stream )
{
	stream << '[';
}

template<class StreamT>
StreamedSerializer<StreamT>::ArraySerializer::ArraySerializer( ArraySerializer&& other ) noexcept
	: stream_( other.stream_ )
	, element_count_(other.element_count_)
{
	PJ_ASSERT( other.stream_ != nullptr );
	other.stream_= nullptr;
}

template<class StreamT>
StreamedSerializer<StreamT>::ArraySerializer::~ArraySerializer()
{
	if( stream_ != nullptr )
		*stream_ << ']';
}

template<class StreamT>
void StreamedSerializer<StreamT>::ArraySerializer::AddNull() noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	*stream_ << "null";
}

template<class StreamT>
typename StreamedSerializer<StreamT>::ObjectSerializer
StreamedSerializer<StreamT>::ArraySerializer::AddObject() noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	return ObjectSerializer( *stream_ );
}

template<class StreamT>
typename StreamedSerializer<StreamT>::ArraySerializer
StreamedSerializer<StreamT>::ArraySerializer::AddArray() noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		stream_ << ',';
	element_count_++;
	return ArraySerializer( *stream_ );
}

template<class StreamT>
void StreamedSerializer<StreamT>::ArraySerializer::AddString( const StringType& string ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	SerializeString( *stream_, string );
}

template<class StreamT>
void StreamedSerializer<StreamT>::ArraySerializer::AddBool( const bool val ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	*stream_ << val ? "true" : "false";
}

template<class StreamT>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, void >::type
StreamedSerializer<StreamT>::ArraySerializer::AddNumber( const T number )
{
	AddNumberInternal( static_cast<int64_t>( number ) );
}

template<class StreamT>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, void >::type
StreamedSerializer<StreamT>::ArraySerializer::AddNumber( const T number )
{
	AddNumberInternal( static_cast<uint64_t>( number ) );
}

template<class StreamT>
template<class T>
typename std::enable_if< std::is_floating_point<T>::value, void >::type
StreamedSerializer<StreamT>::ArraySerializer::AddNumber( const T number )
{
	AddNumberInternal( static_cast<double>( number ) );
}

template<class StreamT>
void StreamedSerializer<StreamT>::ArraySerializer::AddNumberInternal( const double number ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;

	NumberStringStorage num_str;
	GenDoubleValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT>
void StreamedSerializer<StreamT>::ArraySerializer::AddNumberInternal( const int64_t number ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;

	NumberStringStorage num_str;
	GenIntValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT>
void StreamedSerializer<StreamT>::ArraySerializer::AddNumberInternal( const uint64_t number ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;

	NumberStringStorage num_str;
	GenUintValueString( number, num_str );
	*stream_ << num_str.data();
}

// ObjectSerializer

template<class StreamT>
StreamedSerializer<StreamT>::ObjectSerializer::ObjectSerializer( StreamT& stream ) noexcept
	: stream_( &stream )
{
	*stream_ << '{';
}

template<class StreamT>
StreamedSerializer<StreamT>::ObjectSerializer::ObjectSerializer( ObjectSerializer&& other ) noexcept
	: stream_( other.stream_ )
	, element_count_(other.element_count_)
{
	PJ_ASSERT( other.stream_ != nullptr );
	other.stream_= nullptr;
}

template<class StreamT>
StreamedSerializer<StreamT>::ObjectSerializer::~ObjectSerializer()
{
	if( stream_ != nullptr )
		*stream_ << '}';
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::AddNull( const StringType& key ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey( key );
	*stream_ << "null";
}

template<class StreamT>
typename StreamedSerializer<StreamT>::ObjectSerializer
StreamedSerializer<StreamT>::ObjectSerializer::AddObject( const StringType& key ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey( key );
	return ObjectSerializer( *stream_ );
}

template<class StreamT>
typename StreamedSerializer<StreamT>::ArraySerializer
StreamedSerializer<StreamT>::ObjectSerializer::AddArray( const StringType& key ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey( key );
	return ArraySerializer( *stream_ );
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::AddString( const StringType& key, const StringType& string ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey(key);
	SerializeString( *stream_, string );
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::AddBool( const StringType& key, const bool val ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey(key);
	*stream_ << val ? "true" : "false";
}

template<class StreamT>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, void >::type
StreamedSerializer<StreamT>::ObjectSerializer::AddNumber( const StringType& key, const T number )
{
	AddNumberInternal( key, static_cast<int64_t>( number ) );
}

template<class StreamT>
template<class T>
typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, void >::type
StreamedSerializer<StreamT>::ObjectSerializer::AddNumber( const StringType& key, const T number )
{
	AddNumberInternal( key, static_cast<uint64_t>( number ) );
}

template<class StreamT>
template<class T>
typename std::enable_if< std::is_floating_point<T>::value, void >::type
StreamedSerializer<StreamT>::ObjectSerializer::AddNumber( const StringType& key, const T number )
{
	AddNumberInternal( key, static_cast<double>( number ) );
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::AddNumberInternal( const StringType& key, const double number ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey(key);

	NumberStringStorage num_str;
	GenDoubleValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::AddNumberInternal( const StringType& key, const int64_t number ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey(key);

	NumberStringStorage num_str;
	GenIntValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::AddNumberInternal( const StringType& key, const uint64_t number ) noexcept
{
	PJ_ASSERT( stream_ != nullptr );
	if( element_count_ != 0u )
		*stream_ << ',';
	element_count_++;
	WriteKey(key);

	NumberStringStorage num_str;
	GenUintValueString( number, num_str );
	*stream_ << num_str.data();
}

template<class StreamT>
void StreamedSerializer<StreamT>::ObjectSerializer::WriteKey( const StringType& key ) noexcept
{
	SerializeString( *stream_, key );
	*stream_ << ':';
}

} // namespace PanzerJson
