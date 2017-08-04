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
		GenDoubleValueString( number_value.double_value, num_str_ );
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
			GenDoubleValueString( number_value.double_value, num_str_ );
			return;
		}
		else
		{
			GenUintValueString( int_val, num_str_ );
			return;
		}
	}
	else
	{
		if( number_value.double_value < static_cast<double>( std::numeric_limits<int64_t>::min() ) )
		{
			// Absolute double value is bigger, than int range, write as double.
			GenDoubleValueString( number_value.double_value, num_str_ );
			return;
		}
		else
		{
			GenIntValueString( number_value.int_value, num_str_ );
			return;
		}
	}
}

} // namespace PanzerJson
