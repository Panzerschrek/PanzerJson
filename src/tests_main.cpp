#include <iostream>
#include <cstring>

#include <PanzerJson/value.hpp>
#include <PanzerJson/parser.hpp>


int main()
{
	const char json[]=
	R"(

		{
			"foo" : -44e5,
			"bar" : "baz",
			"simple_array" :
			[
				42,
				"42",
				{ "on_object" : 1488 },
				"42 is 42"
			]
		}
	)";

	PanzerJson::Parser parser;

	PanzerJson::Parser::Result result= parser.Parse( json, std::strlen(json) );

	result.root.Serialize( std::cout );
}
