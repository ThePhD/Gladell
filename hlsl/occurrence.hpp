#pragma once

#include "../numeric.hpp"

namespace gld { namespace hlsl {

	struct occurrence {
		intz offset;
		intz offset_after;
		intz processed_line;
		intz line;
		intz column;

		occurrence() : offset( 0 ), offset_after( 0 ), processed_line( 1 ), line( 1 ), column( 1 ) {

		}
	};

}}
