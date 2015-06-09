#pragma once

#include "../token.hpp"
#include "../parser_error.hpp"
#include "../../string.hpp"
#include "../../range.hpp"
#include "../../lexical_numeric_format.hpp"
#include "../../lexical_character_format.hpp"
#include <vector>

namespace gld { namespace hlsl { namespace preprocessor {

	struct root {

	};

	class parser {
	private:
		buffer_view<token> tokens;


	public:
		parser( buffer_view<token> tokenview ) : tokens( std::move( tokenview ) ) {

		}

		void operator () () {

		}
	};

}}}
