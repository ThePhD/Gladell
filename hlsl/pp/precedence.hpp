#pragma once

#include "../operator_precedence.hpp"
#include "../../numeric.hpp"
#include "../../optional.hpp"
#include <unordered_map>

namespace gld { namespace hlsl { namespace pp {

	inline optional<const operator_precedence&> precedence_of( token_id id ) {
		const std::unordered_map<token_id, operator_precedence> operator_predences( {
			{ token_id::add, { token_id::add, 1, associativity::left } },
			//{ token_id::add_assignment, { token_id::add_assignment, 2, associativity::left } },
			{ token_id::subtract, { token_id::subtract, 1, associativity::left } },
			//{ token_id::subtract_assignment, { token_id::subtract_assignment, 2, associativity::left } },

			{ token_id::multiply, { token_id::multiply, 256, associativity::left } },
			//{ token_id::multiply_assignment, { token_id::multiply_assignment, 257, associativity::left } },
			{ token_id::divide, { token_id::divide, 256, associativity::left } },
			//{ token_id::divide_assignment, { token_id::divide_assignment, 257, associativity::left } },
			{ token_id::modulus, { token_id::modulus, 256, associativity::left } },
			//{ token_id::modulus_assignment, { token_id::modulus_assignment, 257, associativity::left } },

			{ token_id::boolean_and, { token_id::boolean_and, 512, associativity::left } },
			//{ token_id::boolean_and_assignment, { token_id::boolean_and_assignment, 513, associativity::left } },
			{ token_id::boolean_or, { token_id::boolean_or, 512, associativity::left } },
			//{ token_id::boolean_or_assignment, { token_id::boolean_or_assignment, 513, associativity::left } },
			{ token_id::boolean_xor, { token_id::boolean_xor, 512, associativity::left } },
			//{ token_id::boolean_xor_assignment, { token_id::boolean_xor_assignment, 513, associativity::left } },
			{ token_id::left_shift, { token_id::left_shift, 512, associativity::left } },
			//{ token_id::left_shift_assignment, { token_id::left_shift_assignment, 513, associativity::left } },
			{ token_id::right_shift, { token_id::right_shift, 512, associativity::left } },
			//{ token_id::right_shift_assignment, { token_id::right_shift_assignment, 513, associativity::left } },
			{ token_id::boolean_complement, { token_id::boolean_complement, 514, associativity::right } },
			//{ token_id::boolean_complement_assignment,{ token_id::boolean_complement_assignment, 514, associativity::right } },

			{ token_id::equal_to, { token_id::equal_to, 512, associativity::left } },
			{ token_id::not_equal_to, { token_id::not_equal_to, 513, associativity::left } },
			{ token_id::greater_than, { token_id::greater_than, 512, associativity::left } },
			{ token_id::greater_than_or_equal_to, { token_id::greater_than_or_equal_to, 513, associativity::left } },
			{ token_id::less_than, { token_id::less_than, 512, associativity::left } },
			{ token_id::less_than_or_equal_to, { token_id::less_than_or_equal_to, 513, associativity::left } },
			
			{ token_id::expression_and, { token_id::expression_and, 1024, associativity::left } },
			{ token_id::expression_or, { token_id::expression_or, 1024, associativity::left } },
			{ token_id::expression_negation, { token_id::expression_negation, 1024, associativity::right } },

			{ token_id::charizing, { token_id::charizing, 2048, associativity::right } },
			{ token_id::token_pasting, { token_id::token_pasting, 2049, associativity::right } },
			{ token_id::stringizing, { token_id::stringizing, 2050, associativity::right } }
		} );

		auto opfind = operator_predences.find( id );
		if ( opfind == operator_predences.end() ) {
			return none;
		}
		return opfind->second;
	}

}}}
