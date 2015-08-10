#pragma once

#include "token_id.hpp"
#include "../numeric.hpp"

namespace gld { namespace hlsl {

	enum class associativity {
		left,
		right
	};

	struct operator_precedence {
		token_id op;
		intz precedence;
		associativity association;
	};

}}
