#pragma once

#include "../../variant.hpp"
#include "construct.hpp"
#include "expression.hpp"
#include "statement.hpp"
#include <vector>

namespace gld { namespace hlsl { namespace preprocessor {

	typedef variant<
		//statement,
		// symbols
		definition,
		function,
		statement,
		
		//plain/substitution,
		text_expression,
		substitution_expression,
		substitution_function_expression,

		//expression,
		//binary_expression,
		addsub_expression,
		muldiv_expression,
		xor_expression,
		bitwise_or_expression,
		bitwise_and_expression,
		or_expression,
		and_expression,

		
		//unary_expression,
		not_expression,
		negation_expression,
		charizing_expression,
		stringizing_expression,

	> node;

	struct parse_tree {
		typedef 
		std::vector<scope> scope_storage;
		std::vector<sequence> sequence_storage;
		std::vector<symbol> symbol_storage;
		std::vector<> _storage;
		std::vector<> _storage;
		std::vector<> _storage;
	};

}}}
