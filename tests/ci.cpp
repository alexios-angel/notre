#include <notre.hpp>

#if NOTRE_CNTTP_COMPILER_CHECK

#define TEST_MATCH(pattern, subject, result) static_assert(notre::match<pattern, notre::case_sensitive>(subject) == result)

#define TEST_CI_MATCH(pattern, subject, result) static_assert(notre::match<pattern, notre::case_insensitive>(subject) == result)

#else

#define UNIQUE_ID_HELPER(A,B) A ## B 
#define UNIQUE_ID(LINE) UNIQUE_ID_HELPER(_ptn, LINE)

#define TEST_MATCH(pattern, subject, result) static constexpr auto UNIQUE_ID(__LINE__) = ctll::fixed_string(pattern); static_assert(notre::regular_expression<typename notre::regex_builder<UNIQUE_ID(__LINE__)>::type, notre::match_method, notre::case_sensitive>{}(subject) == result)

#define TEST_CI_MATCH(pattern, subject, result) static constexpr auto UNIQUE_ID(__LINE__) = ctll::fixed_string(pattern); static_assert(notre::regular_expression<typename notre::regex_builder<UNIQUE_ID(__LINE__)>::type, notre::match_method, notre::case_insensitive>{}(subject) == result)

#endif

TEST_MATCH("aloha","aloha", true);
TEST_MATCH("aloha","ALOHA", false);

TEST_MATCH("[a-z]+","aloha", true);
TEST_MATCH("[a-z]+","ALOHA", false);

TEST_CI_MATCH("aloha", "aloha", true);
TEST_CI_MATCH("aloha", "ALOHA", true);

TEST_CI_MATCH("[a-z]+", "aloha", true);
TEST_CI_MATCH("[a-z]+", "ALOHA", true);

TEST_CI_MATCH("[a-z]+", "9", false);
TEST_CI_MATCH("[a-z]+", " ", false);

TEST_CI_MATCH("[a-Z]+", "9", false);
TEST_CI_MATCH("[a-Z]+", " ", false);

TEST_CI_MATCH("[a-Z]+", "z", false);
TEST_CI_MATCH("[a-Z]+", "z", false);

