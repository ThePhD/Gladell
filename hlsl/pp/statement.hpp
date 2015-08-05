#pragma once

#include "expression.hpp"

namespace gld { namespace hlsl { namespace pp {

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

	struct substitution_argument : symbol {
		substitution_argument( buffer_view<const token> seq ) : symbol( seq ) {

		}
	};

	typedef variant<substitution_argument, text_line> substitution_text;

	struct substitution : sequence {
		std::vector<substitution_text> text;

		substitution( buffer_view<const token> seq, std::vector<substitution_text> text ) : sequence( seq ), text( std::move( text ) ) {

		}
	};

	struct undefinition : sequence {
		symbol name;
		undefinition( buffer_view<const token> seq, symbol name ) : sequence( seq ), name( std::move( name ) ) {

		}
	};

	struct variable : sequence {
		symbol name;
		text_line substitution;

		template <typename... Tn>
		variable( buffer_view<const token> seq, symbol name, Tn&&... argn ) : sequence( seq ), name( name ), substitution( std::forward<Tn>( argn )... ) {

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

				void operator() ( const text_line& line ) {
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

	struct force_line : sequence {
		integral_literal number;
		optional<string_literal> filename;

		force_line( buffer_view<const token> seq, integral_literal number, optional<string_literal> filename ) : sequence( seq ), number( std::move( number ) ), filename( std::move( filename ) ) {
			
		}

	};

	typedef variant<function, variable> definition;

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
		variable,
		function,
		force_line,
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

	struct conditional_block {
		conditional condition;
		block branch;

		conditional_block( conditional condition, block branch ) : condition( std::move( condition ) ), branch( std::move( branch ) ) {

		}
	};

	struct if_elseif_else : sequence {
		std::vector<conditional_block> success_blocks;
		bool no_more_conditions;

		if_elseif_else() : no_more_conditions( false ) {

		}
	};

	struct error_construct : sequence {
		string_literal text;
		error_construct( buffer_view<const token> seq, string_literal text ) : sequence( seq ), text( text ) {

		}
	};

	struct pragma_construct : sequence {
		
	};

}}}
