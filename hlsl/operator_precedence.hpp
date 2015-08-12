#pragma once

#include "token_id.hpp"
#include "../numeric.hpp"
#include "../operation.hpp"

namespace gld { namespace hlsl {

	enum class associativity {
		left,
		right
	};

	struct operator_precedence {
		operation op;
		intz precedence;
		associativity association;
	};

}}
