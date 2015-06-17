#pragma once

#include "token.hpp"
#include "../string.hpp"

namespace gld { namespace hlsl { 

	template <typename Iterator>
	struct parser_head {
		Iterator at;
		std::reference_wrapper<const token> t;
		bool available;
		bool prevlinewhitespace;
		bool linewhitespace;
		
		parser_head( Iterator a ) : at( a ), 
		t( Furrovine::null_ref<token>::value ),
		available(false),
		prevlinewhitespace(true),
		linewhitespace(true) {

		}

	};

}}
