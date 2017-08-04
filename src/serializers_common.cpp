#include "serializers_common.hpp"

namespace PanzerJson
{

void GenDoubleValueString( const double val, NumberStringStorage& out_str )
{
	std::snprintf( out_str.data(), out_str.size(), "%1.22e", val );
}

void GenIntValueString ( const int64_t  val, NumberStringStorage& out_str )
{
	std::snprintf( out_str.data(), out_str.size(), "%lld", val );
}

void GenUintValueString( const uint64_t val, NumberStringStorage& out_str )
{
	std::snprintf( out_str.data(), out_str.size(), "%llu", val );
}

} // namespace PanzerJson
