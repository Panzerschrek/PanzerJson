#pragma once
#include <vector>

#include "../PanzerJson/value.hpp"

namespace PanzerJson
{

class Parser final
{
public:
	typedef std::vector<unsigned char> DataStorage;

	struct Result
	{
		enum class Error
		{
			NoError,
			UnexpectedLexem,
			UnexpectedEndOfFile,
		};

		Error error;
		size_t error_pos;

		Value root;
		DataStorage storage;
	};

public:
	Parser();
	~Parser();

	Result Parse( const char* json_text, size_t json_text_length );

	void ResetCaches();

private:
	const ValueBase* Parse_r(); // Can set error flag.
	StringType ParseString(); // Can set error flag.
	void SkipWhitespaces(); // Can set error flag.

	void CorrectPointers_r( ValueBase& value ); // Also, sorts objects keys.
	StringType CorrectStringPointer( StringType str );

private:
	const char* start_;
	const char* end_;
	const char* cur_;
	Result result_;

	std::vector<unsigned char> number_digits_;

	// Stacks for temporary storing of array/object elements.
	// Lower-level object can use stack, when upper-level object uses it.
	// But, bweh parsing of lower-level was done, upper-level object can continue push values to this stack.
	std::vector<const ValueBase*> array_elements_stack_;
	std::vector<ObjectValue::ObjectEntry> object_entries_stack_;
};

} // namespace PanzerJson
