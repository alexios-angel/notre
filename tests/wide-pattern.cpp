#include <notre.hpp>

static constexpr auto Pattern = ctll::fixed_string(L"[a-z]+");

int main()
{
	const wchar_t test[]{ L"abc" };
	const wchar_t* begin = test;
	const wchar_t* end = begin + 3;
	[[maybe_unused]] const auto results = notre::re<Pattern>().search(begin, end);
}