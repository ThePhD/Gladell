#pragma once

#include "../../variant.hpp"
#include "construct.hpp"

namespace gld { namespace hlsl { namespace preprocessor {



	struct symbol : sequence {
		std::reference_wrapper<const token> name;

		symbol( buffer_view<const token> seq, const token& target ) : sequence( seq ), name( target ) {

		}

	};

	struct definition : symbol {
		variant<unit,
			bool,
			uint8, uint16, uint32, uint64,
			int8, int16, int32, int64,
			code_point,
			half, float, double> value;

		template <typename... Tn>
		definition( buffer_view<const token> seq, const token& target, Tn&&... argn ) : symbol( seq, target ), value( std::forward<Tn>( argn )... ) {

		}
	};

	struct function : definition {
		std::vector<std::reference_wrapper<const token>> parameters;
		std::vector<std::reference_wrapper<const token>> substitutions;
		bool variadic_argument;

		function( buffer_view<const token> seq, const token& target, expression& expr,
			std::vector<std::reference_wrapper<const token>> params,
			std::vector<std::reference_wrapper<const token>> subs )
			: definition( seq, target, std::ref( expr ) ), parameters( std::move( params ) ), substitutions( std::move( subs ) ),
			variadic_argument( !parameters.empty() && parameters.back().get().id == token_id::dot_dot_dot ) {

		}
	};

	struct integral_constant {

	};

	struct floating_constant {

	};

	struct not_expression;
	struct negate_expression;
	struct and_expression;
	struct or_expression;
	struct less_expression;
	struct less_equal_expression;
	struct greater_expression;
	struct greater_equal_expression;
	struct equal_expression;

	typedef variant<
		// symbols
		symbol,
		definition,
		function,
		floating_constant,
		integral_constant,
		// tree-like expressions
		std::reference_wrapper<not_expression>,
		std::reference_wrapper<negate_expression>,
		std::reference_wrapper<and_expression>,
		std::reference_wrapper<or_expression>,
		std::reference_wrapper<less_expression>,
		std::reference_wrapper<less_equal_expression>,
		std::reference_wrapper<greater_expression>,
		std::reference_wrapper<greater_equal_expression>,
		std::reference_wrapper<equal_expression>,
	> expression;

	struct binary_expression {
		std::reference_wrapper<expression> left;
		std::reference_wrapper<expression> right;
	};

	struct unary_expression {
		std::reference_wrapper<expression> left;
	};

	struct not_expression : unary_expression {

	};

	struct negation_expression : unary_expression {

	};

}}}
