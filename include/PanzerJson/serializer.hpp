#pragma once
#include "value.hpp"

namespace PanzerJson
{

// Simple serializer. Uses stl-like streams.
class Serializer final
{
public:
	template<class Stream>
	void Serialize( Value value, Stream& stream );

private:
	template<class Stream>
	void SerializeString( Stream& stream, StringType str );

	template<class Stream>
	void Serialize_r( Stream& stream, const ValueBase& value );

	void GenNumberValueString( const NumberValue& number_value );
	void GenDoubleValueString( double val );
	void GenIntValueString ( int64_t  val );
	void GenUintValueString( uint64_t val );

private:
	char num_str_[64u];
};

} // namespace PanzerJson

#include "serializer.inl"
