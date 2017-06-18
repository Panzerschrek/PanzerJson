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
};

} // namespace PanzerJson
