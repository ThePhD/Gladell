#pragma once

#include "../../variant.hpp"
#include "construct.hpp"
#include "expression.hpp"
#include "statement.hpp"
#include <vector>

namespace gld { namespace hlsl { namespace preprocessor {

	struct parse_tree {
#define GLD_STORAGE( x ) std::vector<x> x##_storage
		// Expression storage
		GLD_STORAGE( conditional );
		GLD_STORAGE( ternary_expression );

		GLD_STORAGE( not_expression );
		GLD_STORAGE( less_expression );
		GLD_STORAGE( less_equals_expression );
		GLD_STORAGE( greater_expression );
		GLD_STORAGE( greater_equals_expression );
		GLD_STORAGE( equals_expression );

		GLD_STORAGE( negate_expression );
		GLD_STORAGE( and_expression );
		GLD_STORAGE( or_expression );
		GLD_STORAGE( xor_expression );

		GLD_STORAGE( add_subtract_expression );
		GLD_STORAGE( multiply_divide_expression );
		GLD_STORAGE( modulus_expression );

		GLD_STORAGE( stringizing_expression );
		GLD_STORAGE( charizing_expression );
		GLD_STORAGE( token_pasting_expression );

		// Statement Storage
		GLD_STORAGE( definition );
		GLD_STORAGE( function );

		GLD_STORAGE( block );

		GLD_STORAGE( if_elseif_else );
		GLD_STORAGE( pragma_construct );
		GLD_STORAGE( error_construct );
#undef GLD_STORAGE

	public:
		// Defer initialization to later...
		optional<statement> root;

		parse_tree( ) {

		}

	};

}}}
