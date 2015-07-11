#pragma once

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

		inclusion( buffer_view<const token> seq, string_literal name ) : sequence( seq ), name( name ) {

		}
	};

	struct substitution_argument : symbol {

	};

	struct substitution_text : sequence {
		std::vector<symbol> symbols;
	};

	struct definition;
	struct function;

	struct block;

	struct if_elseif_else;
	struct pragma_construct;
	struct error_construct;

	typedef variant<
		//statement,
		// symbols
		symbol,
		index_ref<definition>,
		index_ref<function>,
		// aggregates
		index_ref<block>,
		// flow control
		index_ref<if_elseif_else>,
		// keyword constructs
		inclusion,
		index_ref<pragma_construct>,
		index_ref<error_construct>,
		parser_error
	> statement;

	struct substitution : sequence {
		std::vector<variant<substitution_argument, substitution_text>> text;

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

	struct block : sequence {
		std::vector<statement> statements;
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
		std::vector<block> success_blocks;
	};

	struct error_construct : sequence {
		string_literal text;


	};

}}}
