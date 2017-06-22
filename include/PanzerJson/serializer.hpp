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
};

} // namespace PanzerJson

#include "serializer.inl"
