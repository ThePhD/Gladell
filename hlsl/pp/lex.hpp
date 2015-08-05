#pragma once

#include "lexer.hpp"

namespace gld { namespace hlsl { namespace pp {

	inline std::vector<token> lex( string origin, string_view source ) {
		lexer l( std::move( origin ), source );
		return l();
	}

}}}
