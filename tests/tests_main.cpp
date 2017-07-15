extern void RunParserTests();
extern void RunParserErrorsTests();
extern void RunValueTests();
extern void RunParsersEqualityTests();

int main()
{
	RunValueTests();
	RunParserTests();
	RunParserErrorsTests();
	RunParsersEqualityTests();
}
