#pragma once
#include "value.hpp"
#include "../../src/serializers_common.hpp"

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
	void Serialize_r( Stream& stream, const ValueBase& value );

	void GenNumberValueString( const NumberValue& number_value );

private:
	NumberStringStorage num_str_;
};

} // namespace PanzerJson

#include "serializer.inl"
