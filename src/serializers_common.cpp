#include "serializers_common.hpp"

namespace PanzerJson
{

void GenDoubleValueString( const double val, NumberStringStorage& out_str )
{
	std::snprintf( out_str.data(), out_str.size(), "%1.22e", val );
}

void GenIntValueString ( const int64_t  val, NumberStringStorage& out_str )
{
	std::snprintf( out_str.data(), out_str.size(), "%ld", val );
}

void GenUintValueString( const uint64_t val, NumberStringStorage& out_str )
{
	std::snprintf( out_str.data(), out_str.size(), "%lu", val );
}

} // namespace PanzerJson
