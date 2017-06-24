#include <cmath>
#include <limits>

#include "../include/PanzerJson/serializer.hpp"

namespace PanzerJson
{

void Serializer::GenNumberValueString( const NumberValue& number_value )
{
	if( std::floor( number_value.double_value ) != number_value.double_value )
	{
		// Has nonzero fractional part - write as double.
		GenDoubleValueString( number_value.double_value );
		return;
	}

	// Try reconstruct original number value.
	// Double value allways saves sign of original value.
	if( number_value.double_value >= 0.0 )
	{
		const uint64_t int_val= static_cast<uint64_t>( number_value.int_value );
		if( number_value.double_value > static_cast<double>( std::numeric_limits<uint64_t>::max() ) )
		{
			// Double value is bigger, than int range, write as double.
			GenDoubleValueString( number_value.double_value );
			return;
		}
		else
		{
			GenUintValueString( int_val );
			return;
		}
	}
	else
	{
		if( number_value.double_value < static_cast<double>( std::numeric_limits<int64_t>::min() ) )
		{
			// Absolute double value is bigger, than int range, write as double.
			GenDoubleValueString( number_value.double_value );
			return;
		}
		else
		{
			GenIntValueString( number_value.int_value );
			return;
		}
	}
}

void Serializer::GenDoubleValueString( const double val )
{
	std::snprintf( num_str_, sizeof(num_str_), "%1.22e", val );
}

void Serializer::GenIntValueString ( const int64_t  val )
{
	std::snprintf( num_str_, sizeof(num_str_), "%lld", val );
}

void Serializer::GenUintValueString( const uint64_t val )
{
	std::snprintf( num_str_, sizeof(num_str_), "%llu", val );
}

} // namespace PanzerJson
