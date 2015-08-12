#pragma once

namespace gld {

	enum class operation {
		// Increment decrement operators
		post_increment,
		post_decrement,
		pre_increment,
		pre_decrement,
		// Comparison operators
		less_than,
		greater_than,
		equal_to,
		not_equal_to,
		less_than_or_equal_to,
		greater_than_or_equal_to,
		// Bitwise operators...
		boolean_complement,
		boolean_or,
		boolean_and,
		boolean_xor,
		left_shift,
		right_shift,
		// ~= not in the language?
		boolean_complement_assignment,
		boolean_or_assignment,
		boolean_and_assignment,
		boolean_xor_assignment,
		left_shift_assignment,
		right_shift_assignment,
		// Expression operators
		expression_negation,
		expression_or,
		expression_and,
		assignment,
		ternary_expression,
		// Math operators
		add,
		subtract,
		multiply,
		divide,
		modulus,
		add_assignment,
		subtract_assignment,
		multiply_assignment,
		divide_assignment,
		modulus_assignment,
		// access operators
		scope_access,
		struct_access,
		array_access,
		function_access,
	};

}
