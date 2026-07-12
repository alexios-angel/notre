#include "notre.hpp"

static constexpr inline auto pattern = ctll::fixed_string{"abcdefghijklmnopqrstuvwxy|abcdefghijklmnopqrstuvwxyz|abcdefghijklmnopqrstuvwxyz"};

static constexpr inline auto a = notre::re<pattern>();

template <typename T> void identify(T) { }

void fnc() {
	identify(a);
}


