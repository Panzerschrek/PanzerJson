#include <iostream>
#include "../include/PanzerJson/serializer.hpp"

extern void RunParserTests();
extern void RunParserErrorsTests();
extern void RunValueTests();

int main()
{
	PanzerJson::Serializer().Serialize( PanzerJson::Value(), std::cout );

	RunValueTests();
	RunParserTests();
	RunParserErrorsTests();
}
