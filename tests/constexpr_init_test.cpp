#include "../include/PanzerJson/value.hpp"

namespace PanzerJson
{

namespace SimpleValueTest
{

static constexpr ValueBase simple_value( ValueBase::Type::Null );
static constexpr NullValue simple_null_value;

static constexpr ObjectValueWithEntriesStorage<1u> simple_object_storage0
{
	ObjectValue(1u),
	{
		{ "foo", &simple_null_value }
	}
};

static constexpr ObjectValueWithEntriesStorage<2u> simple_object_storage1
{
	ObjectValue(2u),
	{
		{ "bar", &simple_null_value },
		{ "foo", &simple_object_storage0.value },
	}
};

static constexpr StringValueWithStorage<4u> strings[]
{
	{
		StringValue(), "Foo",
	},
	{
		StringValue(), "Bar",
	},
	{
		StringValue(), "Baz",
	},
};

static constexpr StringValueWithStorage< sizeof("u8Сложная utf-8 öžçšüÖ строка") > string_utf8
{
	StringValue(),
	"u8Сложная utf-8 öžçšüÖ строка"
};

static constexpr BoolValue bool_value_false(false);
static constexpr BoolValue bool_value_true(true);

static constexpr ArrayValueWithElementsStorage<3u> simple_array_storage0
{
	ArrayValue(3u),
	{ &strings[0].value, &strings[1].value, &string_utf8.value }
};

static constexpr ArrayValueWithElementsStorage<1u> simple_array_storage1
{
	ArrayValue(1u),
	{ &simple_object_storage0.value }
};

static constexpr ArrayValueWithElementsStorage<1u> simple_array_storage2
{
	ArrayValue(1u),
	{ &simple_null_value }
};

static constexpr NumberValue number_value( "42", 42, 42.0 );

}

} // namespace PanzerJson
