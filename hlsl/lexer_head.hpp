#pragma once

#include "occurrence.hpp"
#include "../string.hpp"

namespace gld { namespace hlsl { 

	template <typename Iterator>
	struct lexer_head {
		Iterator at;
		Iterator after_at;
		code_point c;
		code_point after_c;
		occurrence where;
		bool available;
		bool after_available;
		bool white_space;
		bool line_terminator;
		bool compound_line_terminator;
		bool line_whitespace;
		bool previous_line_whitespace;

		lexer_head( Iterator a ) : at( a ), after_at( a ), 
		c( static_cast<code_point>( -1 ) ), after_c( static_cast<code_point>(-1) ),
		where{},
		available( true ),after_available( true ), 
		white_space( true ), line_terminator( false ), compound_line_terminator( false ),
		previous_line_whitespace( false ), line_whitespace( false ) {

		}

	};

}}
