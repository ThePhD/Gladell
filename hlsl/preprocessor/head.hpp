#pragma once

#include "../occurrence.hpp"
#include "../../string.hpp"

namespace gld { namespace hlsl { 

	template <typename Iterator>
	struct head {
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

		head( Iterator a ) : at( a ), after_at( a ),
		available( true ), white_space( true ), 
		line_terminator( false ), line_whitespace( false ) {

		}

	};

}}
