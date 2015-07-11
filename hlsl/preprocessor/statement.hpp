#pragma once

#include "../../variant.hpp"
#include "expression.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	template <typename T>
	inline bool is_variadic_arguments( T& parameter ) {
		if ( parameters.empty() )
			return false;
		sequence& seq = parameters.back();
		if ( seq.tokens.empty() )
			return false;
		return seq.tokens.front().id == token_id::dot_dot_dot;
	}

	struct inclusion : sequence {
		string_literal name;

		inclusion( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct definition;
	struct function;

	typedef variant<
		//statement,
		// symbols
		symbol,
		std::reference_wrapper<definition>,
		std::reference_wrapper<function>,
		// flow control
		std::reference_wrapper<if_elseif_else>,
		std::reference_wrapper<switch_case>,
		// aggregates
		std::reference_wrapper<block>
	> statement;

	struct block : sequence {
		std::vector<statement> statements;
	};

	struct substitution : sequence {
		std::vector<symbol> substitutions;

		substitution( buffer_view<const token> seq ) : sequence( seq ) {

		}
	};

	struct definition : sequence {
		symbol name;
		block substitution;

		template <typename... Tn>
		definition( buffer_view<const token> seq, symbol name, Tn&&... argn ) : sequence( seq ), name( name ), substitution( std::forward<Tn>( argn )... ) {

		}
	};

	struct function : sequence {
		symbol name;
		std::vector<symbol> parameters;
		std::vector<symbol> substitutions;
		bool variadic_argument;
		block routine;

		function( buffer_view<const token> seq, symbol name,
			std::vector<symbol> params, std::vector<symbol> subs, block routine )
			: sequence( seq ), name( name ), parameters( std::move( params ) ), substitutions( std::move( subs ) ),
			variadic_argument( is_variadic_arguments( parameters ) ), routine( std::move( routine ) ) {

		}
	};

	struct if_elseif_else : sequence {
		std::vector<conditional> conditions;
	};

	struct switch_case : if_elseif_else {

	};

}}}
