#pragma once

#include "../int.hpp"

namespace gld { namespace hlsl {

	struct occurrence {
		intz offset;
		intz offset_after;
		intz true_line;
		intz line;
		intz column;
	};

}}
