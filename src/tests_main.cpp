#include <iostream>
#include <cstring>

#include <PanzerJson/value.hpp>
#include <PanzerJson/parser.hpp>


int main()
{
	const char json[]=
	u8R"(

		{
			"foo" : -44e5,
			"bar" : "baz",
			"simple\narray" :
			[
				{
					"g" : 0,
					"b" : 0,
					"g" : 0,
					"a" : 0,
					"d" : 0,
					"c" : 0,
					"fc" : 0,
					"ff" : 0,
					"fa" : 0,
					"ÖŽÇŠÜÄ  ----  öžçšüä" : "Да, это юникод"
				},

				true,
				null,
				false,
				"\"42\"",
				{ "on_object" : 1488 },
				"42 is 42"
			]
		}
	)";

	PanzerJson::Parser parser;

	PanzerJson::Parser::Result result= parser.Parse( json, std::strlen(json) );

	result.root.Serialize( std::cout );
}
