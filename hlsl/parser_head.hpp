#pragma once

#include "token.hpp"
#include "../string.hpp"

namespace gld { namespace hlsl { 

	template <typename Iterator>
	struct parser_head {
		Iterator at;
		std::reference_wrapper<const token> t;
		token_id id;
		bool available;
		bool prevlinewhitespace;
		bool linewhitespace;
		
		parser_head( Iterator a ) : at( a ), 
		t( Furrovine::null_ref<token>::value ),
		id( token_id::whitespace ),
		available(false),
		prevlinewhitespace(true),
		linewhitespace(true) {

		}

	};

}}
