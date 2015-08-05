#pragma once

#include "lex.hpp"
#include "parser.hpp"

namespace gld { namespace hlsl { namespace pp {

	inline parse_tree parse( std::vector<token> tokens ) {
		symbol_table symbols;
		parse_tree tree;
		parser p( tokens, tree, symbols );
		p();
		return tree;
	}

}}}
