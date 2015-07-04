#pragma once

#include "../token.hpp"
#include "../../optional.hpp"
#include "../../string.hpp"
#include "../../range.hpp"
#include "../../stack.hpp"
#include <functional>
#include <vector>
#include <unordered_map>

namespace gld { namespace hlsl { namespace preprocessor {

	struct sequence {
		buffer_view<const token> tokens;

		template <typename... Tn>
		sequence( Tn&&... argn ) : tokens( std::forward<Tn>( argn )... ) {

		}
	};

}}}
