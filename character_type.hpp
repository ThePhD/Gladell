#pragma once

#include <Furrovine++/enums.hpp>

namespace gld {

	enum class character_type {
		none = 0x0,
		numeric = 0x1,
		symbol = 0x2,
		id = 0x4,
		whitespace = 0x8
	};

}

namespace Furrovine {

	template <>
	struct enums::is_flags<gld::character_type> : std::true_type {

	};

}
