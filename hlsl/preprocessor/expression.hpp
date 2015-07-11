#pragma once

#include "../../variant.hpp"
#include "../../constant_type.hpp"
#include "construct.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	template <typename T>
	inline string_view create_name( T& tokens ) {
		if ( tokens.empty() )
			return {};
		auto first = tokens.first().lexeme.begin();
		auto last = tokens.back().lexeme.end();
		return string_view( first, last );
	}

	struct symbol : sequence {

		string_view name;

		symbol( buffer_view<const token> seq ) : sequence( seq ), name( create_name( tokens ) ) {
			
		}

	};

	struct integral_literal : sequence {
		constant_type type;
		integral_literal( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct floating_literal : sequence {
		constant_type type;
		floating_literal( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct boolean_literal : sequence {

		boolean_literal( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct string_literal : sequence {

		string_literal( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct char_literal : sequence {

		char_literal( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct defined : sequence {
		symbol target;

		defined( buffer_view<const token> seq, symbol target ) : sequence( seq ), target( target ) {

		}
	};

	struct conditional;
	struct ternary_expression;

	struct not_expression;
	struct less_expression;
	struct less_equals_expression;
	struct greater_expression;
	struct greater_equals_expression;
	struct equals_expression;

	struct negate_expression;
	struct and_expression;
	struct or_expression;
	struct xor_expression;

	struct add_subtract_expression;
	struct multiply_divide_expression;
	struct modulus_expression;
	
	struct stringizing_expression;
	struct charizing_expression;
	struct token_pasting_expression;

	typedef variant<
		symbol,
		floating_literal,
		integral_literal,
		string_literal,
		char_literal,
		boolean_literal,
		// keyword operators
		defined,
		/* tree-like expressions */
		// preprocessor operators/expression
		std::reference_wrapper<stringizing_expression>,
		std::reference_wrapper<charizing_expression>,
		std::reference_wrapper<token_pasting_expression>,
		// flow control
		std::reference_wrapper<ternary_expression>,
		// comparison operators
		std::reference_wrapper<not_expression>,
		std::reference_wrapper<less_expression>,
		std::reference_wrapper<less_equals_expression>,
		std::reference_wrapper<greater_expression>,
		std::reference_wrapper<greater_equals_expression>,
		std::reference_wrapper<equals_expression>,
		// math operators
		std::reference_wrapper<add_subtract_expression>,
		std::reference_wrapper<multiply_divide_expression>,
		std::reference_wrapper<modulus_expression>,
		// bit operators
		std::reference_wrapper<and_expression>,
		std::reference_wrapper<negate_expression>,
		std::reference_wrapper<or_expression>,
		std::reference_wrapper<xor_expression>
	> expression;

	struct unary_expression {
		expression operand;
	};

	struct binary_expression {
		expression left;
		expression right;
	};

	struct conditional {
		expression operand;

		bool can_evaluate_to_boolean() const {
			// TODO: token validation and the like
			return false;
		}
	};

	struct ternary_expression {
		conditional condition;
		expression true_branch;
		expression false_branch;
	};

	struct not_expression : unary_expression {};
	struct less_expression : binary_expression {};
	struct less_equals_expression : binary_expression {};
	struct greater_expression : binary_expression {};
	struct greater_equals_expression : binary_expression {};
	struct equals_expression : binary_expression {};
	struct not_equals_expression : binary_expression {};

	struct negate_expression : unary_expression {};
	struct bitwise_and_expression : binary_expression {};
	struct bitwise_or_expression : binary_expression {};
	struct bitwise_xor_expression : binary_expression {};
	struct and_expression : binary_expression {};
	struct or_expression : binary_expression {};
	struct xor_expression : binary_expression {};

	struct add_subtract_expression : binary_expression {};
	struct multiply_divide_expression : binary_expression {};
	struct modulus_expression : binary_expression {};

	struct stringizing_expression : unary_expression {};
	struct charizing_expression : unary_expression {};
	struct token_pasting_expression : unary_expression {};
	
}}}
