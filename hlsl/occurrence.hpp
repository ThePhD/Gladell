#pragma once

#include "../numeric.hpp"

namespace gld { namespace hlsl {

	struct occurrence {
		intz offset;
		intz offset_after;
		intz processed_line;
		intz line;
		intz column;
	};

}}
