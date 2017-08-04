#pragma once
#include "value.hpp"

namespace PanzerJson
{

// Serializer, that generate json structure "on-fly".
// Uses stl-like streams.
// Output is minified and is not human-readable.

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

template<class StreamT>
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

		void AddNull() noexcept;
		ObjectSerializer AddObject() noexcept;
		ArraySerializer AddArray() noexcept;
		void AddString( const StringType& string ) noexcept;
		void AddBool( bool val ) noexcept;

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
		void AddNumberInternal(   double number ) noexcept;
		void AddNumberInternal(  int64_t number ) noexcept;
		void AddNumberInternal( uint64_t number ) noexcept;

	private:
		explicit ArraySerializer( StreamT& stream ) noexcept;
		friend class StreamedSerializer;
		friend class ObjectSerializer;

	private:
		StreamT* stream_; // null means moved
		size_t element_count_= 0u;
	};

	class ObjectSerializer final
	{
	public:
		ObjectSerializer( const ArraySerializer& )= delete;
		ObjectSerializer& operator=( const ArraySerializer& )= delete;

		ObjectSerializer( ObjectSerializer&& other ) noexcept;
		~ObjectSerializer();

		void AddNull( const StringType& key ) noexcept;
		ObjectSerializer AddObject( const StringType& key ) noexcept;
		ArraySerializer AddArray( const StringType& key ) noexcept;
		void AddString( const StringType& key, const StringType& string ) noexcept;
		void AddBool( const StringType& key, bool val ) noexcept;

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
		void AddNumberInternal( const StringType& key,   double number ) noexcept;
		void AddNumberInternal( const StringType& key,  int64_t number ) noexcept;
		void AddNumberInternal( const StringType& key, uint64_t number ) noexcept;

		void WriteKey( const StringType& key ) noexcept;

	private:
		explicit ObjectSerializer( StreamT& stream ) noexcept;
		friend class StreamedSerializer;
		friend class ArraySerializer;

	private:
		StreamT* stream_; // null means moved
		size_t element_count_= 0u;
	};

public:
	// Takes reference to stream. Serializer must live longer, than stream.
	explicit StreamedSerializer( StreamT& stream ) noexcept;

	StreamedSerializer( const StreamedSerializer& )= delete;
	StreamedSerializer& operator=(const StreamedSerializer& )= delete;
	StreamedSerializer( StreamedSerializer&& other )= default;

	ObjectSerializer AddObject() noexcept;
	ArraySerializer AddArray() noexcept;

private:
	StreamT& stream_;
};

} // namespace PanzerJson

#include "streamed_serializer.inl"
