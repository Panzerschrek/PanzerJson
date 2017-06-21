extern void RunParserTests();
extern void RunParserErrorsTests();
extern void RunValueTests();

int main()
{
	RunValueTests();
	RunParserTests();
	RunParserErrorsTests();
}
