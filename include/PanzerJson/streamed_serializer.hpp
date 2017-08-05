#pragma once
#include "value.hpp"

namespace PanzerJson
{

enum class SerializationFormatting
{
	Compact,
	TabIndents,
};

// Serializer, that generate json structure "on-fly".
// Input sream class must supports operator<<(const char*).

// Usage:
// Create object with stream. Call, call "AddObject" or "AddArray" method and
// save returned value.
// Than, use returned value to serialize object or array elements.
// When you finish serializing object or array value, than, you must destroy
// ObjectSerializer/ArraySerializer.
// Note - when child object/array is alive, addition of elements to parent
 // (and parent of parent, etc.) object/array is forbidden.

// Example:
/*
	StreamedSerializer<std::istringstream> serializer(stream);
	{
		auto object= serializer.AddObject();
		object.AddNumber( "one", 1 );
		{
			auto sub_object= object.AddObject( "two" );
			sub_object.AddString( "bar", "baz" );
			sub_object.AddBool( "false_object", false );
			{
				auto arr= sub_object.AddArray( "arr" );
				arr.AddNumber( 3.1415926535f );
				arr.AddString( "str" );
				arr.AddObject(); // empty object
				arr.AddNull();
			}
		}
	}
*/

template<
	class StreamT,
	SerializationFormatting formatting= SerializationFormatting::Compact>
class StreamedSerializer final
{
public:
	class ObjectSerializer;
	class ArraySerializer;

	class ArraySerializer final
	{
	public:
		ArraySerializer( const ArraySerializer& )= delete;
		ArraySerializer& operator=( const ArraySerializer& )= delete;

		ArraySerializer( ArraySerializer&& other ) noexcept;
		~ArraySerializer();

		void AddNull();
		ObjectSerializer AddObject();
		ArraySerializer AddArray();
		void AddString( const StringType& string );
		void AddBool( bool val );

		template<class T>
		typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, void >::type
		AddNumber( T number );

		template<class T>
		typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, void >::type
		AddNumber( T number );

		template<class T>
		typename std::enable_if< std::is_floating_point<T>::value, void >::type
		AddNumber( T number );

	private:
		void AddNumberInternal(   double number );
		void AddNumberInternal(  int64_t number );
		void AddNumberInternal( uint64_t number );

		void StartNewElement( bool new_element_is_composite= false );
		void PrintIndents();

	private:
		explicit ArraySerializer( StreamT& stream, size_t parent_indent );
		friend class StreamedSerializer;
		friend class ObjectSerializer;

	private:
		StreamT* stream_; // null means moved
		size_t element_count_= 0u;
		size_t indent_;
	};

	class ObjectSerializer final
	{
	public:
		ObjectSerializer( const ArraySerializer& )= delete;
		ObjectSerializer& operator=( const ArraySerializer& )= delete;

		ObjectSerializer( ObjectSerializer&& other ) noexcept;
		~ObjectSerializer();

		void AddNull( const StringType& key );
		ObjectSerializer AddObject( const StringType& key );
		ArraySerializer AddArray( const StringType& key );
		void AddString( const StringType& key, const StringType& string );
		void AddBool( const StringType& key, bool val );

		template<class T>
		typename std::enable_if< std::is_integral<T>::value && std::is_signed<T>::value, void >::type
		AddNumber( const StringType& key, T number );

		template<class T>
		typename std::enable_if< std::is_integral<T>::value && std::is_unsigned<T>::value, void >::type
		AddNumber( const StringType& key, T number );

		template<class T>
		typename std::enable_if< std::is_floating_point<T>::value, void >::type
		AddNumber( const StringType& key, T number );

	private:
		void AddNumberInternal( const StringType& key,   double number );
		void AddNumberInternal( const StringType& key,  int64_t number );
		void AddNumberInternal( const StringType& key, uint64_t number );

		void WriteKey( const StringType& key );
		void StartNewElement();
		void PrintIndents();

	private:
		explicit ObjectSerializer( StreamT& stream, size_t parent_indent );
		friend class StreamedSerializer;
		friend class ArraySerializer;

	private:
		StreamT* stream_; // null means moved
		size_t element_count_= 0u;
		size_t indent_;
	};

public:
	// Takes reference to stream. Serializer must live longer, than stream.
	explicit StreamedSerializer( StreamT& stream );

	StreamedSerializer( const StreamedSerializer& )= delete;
	StreamedSerializer& operator=(const StreamedSerializer& )= delete;
	StreamedSerializer( StreamedSerializer&& other )= default;

	ObjectSerializer AddObject();
	ArraySerializer AddArray();

private:
	StreamT& stream_;
};

} // namespace PanzerJson

#include "streamed_serializer.inl"
