#include <iostream>

#include <PanzerJson/value.hpp>
#include "gen_complex_object.hpp"


int main()
{
	PanzerJson::Value val(&complex_object);
	val.Serialize( std::cout );
}
