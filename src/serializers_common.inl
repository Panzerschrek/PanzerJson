namespace PanzerJson
{

template<class Stream>
void SerializeString( Stream& stream, const StringType str )
{
	// Change this if string type chaged.

	// Produce escaped symbols, quotes.

	stream << '"';

	const char* s= str;
	while( *s != '\0' )
	{
		if( *s == '"' )
			stream << "\\\"";
		else if( *s == '\\' )
			stream << "\\\\";
		else if( *s == '/' )
			stream << "\\/";
		else if( *s == '\b' )
			stream << "\\b";
		else if( *s == '\f' )
			stream << "\\f";
		else if( *s == '\n' )
			stream << "\\n";
		else if( *s == '\r' )
			stream << "\\r";
		else if( *s == '\t' )
			stream << "\\t";
		else
			stream << *s;

		++s;
	}

	stream << '"';
}

} // namespace PanzerJson
