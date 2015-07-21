#pragma once

#include "expression.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	struct inclusion : sequence {
		string_literal name;
		inclusion_style style;

		inclusion( buffer_view<const token> seq, string_literal name, inclusion_style style ) : sequence( seq ), name( name ), style( style ) {

		}
	};

	struct text_line : sequence {

		text_line( buffer_view<const token> seq ) : sequence( seq ) {

		}

	};

	struct substitution_line : sequence {
		std::vector<symbol> symbols;

		substitution_line( buffer_view<const token> seq, std::vector<symbol> symbols ) : sequence( seq ), symbols( std::move( symbols ) ) {

		}

	};

	struct substitution_argument : symbol {
		substitution_argument( buffer_view<const token> seq ) : symbol( seq ) {

		}
	};

	typedef variant<substitution_argument, substitution_line> substitution_text;

	struct substitution : sequence {
		std::vector<substitution_text> text;

		substitution( buffer_view<const token> seq, std::vector<variant<substitution_argument, substitution_line>> text ) : sequence( seq ), text( std::move( text ) ) {

		}
	};

	struct undefinition : symbol {
		undefinition( buffer_view<const token> seq ) : symbol( seq ) {

		}
	};

	struct definition : sequence {
		symbol name;
		substitution_line substitution;

		template <typename... Tn>
		definition( buffer_view<const token> seq, symbol name, Tn&&... argn ) : sequence( seq ), name( name ), substitution( std::forward<Tn>( argn )... ) {

		}
	};

	struct function : sequence {
		symbol name;
		std::vector<symbol> parameters;
		substitution routine;
		bool variadic_argument;

		bool is_variadic_arguments() const {
			struct visitor {
				int last = 0;
				int dot_dot_dot = 0;
				void operator() ( const substitution_argument& arg ) {
					++last;
					if ( arg.tokens.empty() )
						return;
					if ( arg.tokens.back().id == token_id::dot_dot_dot )
						dot_dot_dot = last;
					++last;
				}

				void operator() ( const substitution_line& line ) {
					++last;
				}

				bool is_variadic() const {
					return dot_dot_dot == last;
				}
			};
			visitor v{};
			for ( auto& sub : routine.text ) {
				sub.visit( v );
			}
			return v.is_variadic();
		}

		function( buffer_view<const token> seq, symbol name,
			std::vector<symbol> params, substitution routine )
			: sequence( seq ), name( name ),
			parameters( std::move( params ) ),
			routine( std::move( routine ) ),
			variadic_argument( is_variadic_arguments() ) {

		}
	};

	struct block;

	struct if_elseif_else;
	struct pragma_construct;
	struct error_construct;

	typedef variant<
		//statement,
		// symbols
		symbol,
		text_line,
		undefinition,
		definition,
		function,
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

	struct block : sequence {
		std::vector<statement> statements;
	};

	struct if_elseif_else : sequence {
		std::vector<conditional> conditions;
		std::vector<block> success_blocks;
	};

	struct error_construct : sequence {
		string_literal text;
		error_construct( buffer_view<const token> seq, string_literal text ) : sequence( seq ), text( text ) {

		}
	};

	struct pragma_construct : sequence {
		
	};

}}}
