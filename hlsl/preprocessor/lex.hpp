#pragma once

#include "lexer.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	inline std::vector<token> lex( string_view source ) {
		lexer l( source );
		return l();
	}

}}}
