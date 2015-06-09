#pragma once

#include "lex.hpp"
#include "parser.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	inline void parse( std::vector<token> tokens ) {
		parser p( tokens );
		p();
		//return p();
	}

}}}
