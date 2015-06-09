#pragma once

#include "occurrence.hpp"
#include "../string.hpp"

namespace gld { namespace hlsl { 

	template <typename Iterator>
	struct head {
		Iterator at;
		code_point c;
		occurrence consumedwhere;
		bool consumedavailable;
	};

}}
