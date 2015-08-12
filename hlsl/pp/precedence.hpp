#pragma once

#include "../operator_precedence.hpp"
#include "../../numeric.hpp"
#include "../../optional.hpp"
#include "../../enums.hpp"
#include <unordered_map>

namespace gld { namespace hlsl { namespace pp {

	inline const operator_precedence& precedence_of( operation op ) {
		static const operator_precedence precedences[] = {
			// Increment decrement operators
			{ operation::post_increment, 16384, associativity::left },
			{ operation::post_decrement, 16384, associativity::left },
			{ operation::pre_increment, 8192, associativity::right },
			{ operation::pre_decrement, 8192, associativity::right },
			// Comparison operators
			{ operation::less_than, 512, associativity::left },
			{ operation::greater_than, 512, associativity::left },
			{ operation::equal_to, 256, associativity::left },
			{ operation::not_equal_to, 256, associativity::left },
			{ operation::less_than_or_equal_to, 512, associativity::left },
			{ operation::greater_than_or_equal_to, 512, associativity::left },
			// Bitwise operators...
			{ operation::boolean_complement, 8192, associativity::left },
			{ operation::boolean_or, 32, associativity::left },
			{ operation::boolean_and, 128, associativity::left },
			{ operation::boolean_xor, 64, associativity::left },
			{ operation::left_shift, 1024, associativity::left },
			{ operation::right_shift, 1024, associativity::left },
			// ~= not in the language?
			{ operation::boolean_complement_assignment, 0, associativity::right },
			{ operation::boolean_or_assignment, 0, associativity::right },
			{ operation::boolean_and_assignment, 0, associativity::right },
			{ operation::boolean_xor_assignment, 0, associativity::right },
			{ operation::left_shift_assignment, 0, associativity::right },
			{ operation::right_shift_assignment, 0, associativity::right },
			// Expression operators
			{ operation::expression_negation, 8192, associativity::right },
			{ operation::expression_or, 16, associativity::left },
			{ operation::expression_and, 8, associativity::left },
			{ operation::assignment, 4, associativity::left },
			{ operation::ternary_expression, 4, associativity::left },
			// Math operators
			{ operation::add, 2048, associativity::left },
			{ operation::subtract, 2048, associativity::left },
			{ operation::multiply, 4096, associativity::left },
			{ operation::divide, 4096, associativity::left },
			{ operation::modulus, 4096, associativity::left },
			{ operation::add_assignment, 0, associativity::right },
			{ operation::subtract_assignment, 0, associativity::right },
			{ operation::multiply_assignment, 0, associativity::right },
			{ operation::divide_assignment, 0, associativity::right },
			{ operation::modulus_assignment, 0, associativity::right },
			// access operators
			{ operation::scope_access, 65536, associativity::left },
			{ operation::struct_access, 32768, associativity::left },
			{ operation::array_access, 32768, associativity::left },
			{ operation::function_access, 32768, associativity::left }
		};
		
		return precedences[ enums::to_underlying(op) ];
	}

}}}
