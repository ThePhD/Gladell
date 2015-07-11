#pragma once

#include "lex.hpp"
#include "parser.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	inline void parse( std::vector<token> tokens ) {
		symbol_table symbols;
		parse_tree tree;
		parser p( tokens, tree, symbols );
		p();
		//return p();
	}

}}}
