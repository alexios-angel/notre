#include <ctll/fixed_string.hpp>
#include <ostream>

void empty_symbol() { }

static constexpr auto Pattern = ctll::fixed_string{ LR"(^\s*(\d+)\s+:(\S):$(\S+?)$(\S+?)$(\S+))" };

static_assert(Pattern.size() == 38);

// ordinary string is taken as array of bytes
#ifdef NOTRE_STRING_IS_UTF8
static_assert(ctll::fixed_string("ěšč").size() == 3);
static_assert(ctll::fixed_string("😍").size() == 1);
static_assert(ctll::fixed_string("😍")[0] == L'😍');
#else
static_assert(ctll::fixed_string("ěšč").size() == 6); // it's just a bunch of bytes
static_assert(ctll::fixed_string("😍").size() == 4); // it's just a bunch of bytes
#endif

#if __cpp_char8_t
// u8"" is utf-8 encoded
static_assert(ctll::fixed_string(u8"ěšč").size() == 3);
static_assert(ctll::fixed_string(u8"😍").size() == 1);
static_assert(ctll::fixed_string(u8"😍")[0] == U'😍');
#endif

// u"" is utf-16
static_assert(ctll::fixed_string(u"ěšč").size() == 3);
static_assert(ctll::fixed_string(u"😍").size() == 1);
static_assert(ctll::fixed_string(u"😍").is_same_as(ctll::fixed_string(U"😍")));

// U"" is utf-32
static_assert(ctll::fixed_string(U"ěšč").size() == 3);
static_assert(ctll::fixed_string(U"😍").size() == 1);

// everything is converted into utf-32

// general purpose string API

static constexpr auto hello = ctll::fixed_string{"hello"};
static constexpr auto world = ctll::fixed_string{"world"};

// default construction gives an empty, correct string
static_assert(ctll::fixed_string<8>{}.size() == 0);
static_assert(ctll::fixed_string<8>{}.empty());
static_assert(ctll::fixed_string<8>{}.correct());

// observers
static_assert(!hello.empty());
static_assert(hello.length() == 5);
static_assert(decltype(hello)::max_size() == 5);
static_assert(decltype(hello)::capacity() == 5);
static_assert(hello.front() == U'h');
static_assert(hello.back() == U'o');
static_assert(hello.data() == hello.begin());
static_assert(hello.cend() - hello.cbegin() == 5);
static_assert(hello.view().size() == 5);

// comparison across different capacities and against literals
static_assert(hello == ctll::fixed_string{"hello"});
static_assert(hello != world);
static_assert(hello == "hello");
static_assert("hello" == hello);
static_assert(hello != "hellooo");
static_assert(hello == u"hello"); // decoded content is compared
static_assert(hello == U"hello");
static_assert(hello.compare(hello) == 0);
static_assert(hello < world);
static_assert(hello <= world);
static_assert(world > hello);
static_assert(world >= hello);
static_assert(ctll::fixed_string{"ab"} < ctll::fixed_string{"abc"});

// search
static_assert(hello.find(U'l') == 2);
static_assert(hello.find(U'l', 3) == 3);
static_assert(hello.find(U'z') == decltype(hello)::npos);
static_assert(hello.find(ctll::fixed_string{"llo"}) == 2);
static_assert(hello.find(ctll::fixed_string{"lol"}) == decltype(hello)::npos);
static_assert(hello.contains(U'e'));
static_assert(hello.contains(ctll::fixed_string{"ell"}));
static_assert(!hello.contains(ctll::fixed_string{"world"}));
static_assert(hello.starts_with(U'h'));
static_assert(hello.starts_with(ctll::fixed_string{"he"}));
static_assert(!hello.starts_with(ctll::fixed_string{"eh"}));
static_assert(hello.ends_with(U'o'));
static_assert(hello.ends_with(ctll::fixed_string{"llo"}));
static_assert(!hello.ends_with(ctll::fixed_string{"ol"}));

// concatenation
static_assert(hello + world == "helloworld");
static_assert(hello + ", " + world == "hello, world");
static_assert("<" + hello + ">" == "<hello>");
static_assert((hello + world).size() == 10);

// substring
static_assert(hello.substr<0>() == hello);
static_assert(hello.substr<1>() == "ello");
static_assert(hello.substr<1, 3>() == "ell");
static_assert(hello.substr<2, 100>() == "llo"); // count is clamped
static_assert(hello.substr<5>().empty());

// utf-32 content survives the general purpose operations
static_assert((ctll::fixed_string{u"😍"} + ctll::fixed_string{"!"}).size() == 2);
static_assert(ctll::fixed_string{u"a😍b"}.substr<1, 1>() == ctll::fixed_string{U"😍"});
static_assert(ctll::fixed_string{u"a😍b"}.find(U'😍') == 1);

// iostream interoperability (content is written as utf-8)
inline std::ostream & test_ostream_output(std::ostream & str) {
	return str << hello;
}

