#pragma once
#include <array>

#include "../include/PanzerJson/value.hpp"

namespace PanzerJson
{

template<class Stream>
void SerializeString( Stream& stream, const StringType str );

// String with length, enough for exac decimal number representation.
typedef std::array<char, 64> NumberStringStorage;

void GenDoubleValueString( double val, NumberStringStorage& out_str );
void GenIntValueString ( int64_t  val, NumberStringStorage& out_str );
void GenUintValueString( uint64_t val, NumberStringStorage& out_str );

} // namespace PanzerJson

#include "serializers_common.inl"
