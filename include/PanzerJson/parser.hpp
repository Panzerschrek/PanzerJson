#pragma once
#include <memory>
#include <vector>

#include "../PanzerJson/value.hpp"

namespace PanzerJson
{

class Parser final
{
public:
	typedef std::vector<unsigned char> DataStorage;

	// Parse result.
	// If JSON parsed successfully, ResultPtr returned with "Value" and associated internal storage.
	// "root", it copies and derived values must live no longer, than returned from parser "ResultPtr",
	// because all data pointed to this storage.

	// Result is noncopyable, so, one way to "copy" it - use shared pointers.
	struct Result final
	{
		enum class Error
		{
			NoError,
			UnexpectedLexem,
			ControlCharacterInsideString,
			UnexpectedEndOfFile,
			ExtraCharactersAfterJsonRoot,
			RootIsNotObjectOrArray,
		};

		Error error= Error::NoError;
		size_t error_pos= 0u;

		Value root;

		Result()= default;

	private:
		friend class Parser;
		Result& operator=( const Result& )= delete;
		Result( const Result& )= delete;

		DataStorage storage;
	};

	typedef std::unique_ptr<const Result> ResultPtr;
	typedef std::shared_ptr<const Result> ResultSharedPtr;

public:
	Parser();
	~Parser();

	// Returns pointer to result.
	// Never returns nullptr.
	ResultPtr Parse( const char* json_text_null_teriminated );
	ResultPtr Parse( const char* json_text, size_t json_text_length );

	// Enable json root to be not only array or object.
	void SetEnableNoncompositeJsonRoot( bool enable );
	bool GetEnableNoncompositeJsonRoot() const;

	void ResetCaches();

private:
	const ValueBase* Parse_r(); // Can set error flag.
	StringType ParseString(); // Can set error flag.
	void SkipWhitespaces(); // Can set error flag.
	void SkipWhitespacesAtEnd(); // Can set error flag.

	void CorrectPointers_r( ValueBase& value ); // Also, sorts objects keys.
	StringType CorrectStringPointer( StringType str );

private:
	const char* start_;
	const char* end_;
	const char* cur_;
	Result result_;

	bool enable_noncomposite_json_root_= true;

	std::vector<unsigned char> number_digits_;

	// Stacks for temporary storing of array/object elements.
	// Lower-level object can use stack, when upper-level object uses it.
	// But, bweh parsing of lower-level was done, upper-level object can continue push values to this stack.
	std::vector<const ValueBase*> array_elements_stack_;
	std::vector<ObjectValue::ObjectEntry> object_entries_stack_;
};

} // namespace PanzerJson
