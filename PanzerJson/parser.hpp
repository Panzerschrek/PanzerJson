#pragma once
#include <vector>

#include "value.hpp"

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
		Value root;
		DataStorage storage;
	};

public:
	Parser();
	~Parser();

	Result Parse( const char* json_text, size_t json_text_length );

private:
	const ValueBase* Parse_r();
	StringType ParseString();
	void SkipWhitespaces();

	void CorrectPointers_r( ValueBase& value );

private:
	const char* start_;
	const char* end_;
	const char* cur_;
	Result result_;
};

} // namespace PanzerJson
