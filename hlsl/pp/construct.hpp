#pragma once

#include "../token.hpp"

namespace gld { namespace hlsl { namespace pp {

	struct sequence {
		buffer_view<const token> tokens;

		template <typename... Tn>
		sequence( Tn&&... argn ) : tokens( std::forward<Tn>( argn )... ) {

		}
	};

}}}
