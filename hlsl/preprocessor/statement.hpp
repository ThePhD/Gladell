#pragma once

#include "../../variant.hpp"
#include "construct.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	struct conditional : sequence {
		conditional( buffer_view<const token> seq ) : sequence( seq ) {

		}

	};

	struct inclusion {
		sequence name;

		inclusion( buffer_view<const token> seq ) : name( seq ) {

		}
	};

	struct scope : sequence {
		optional<const scope&> parent;
		std::reference_wrapper<const conditional> condition;
		std::vector<std::reference_wrapper<sequence>> sequences;

		scope( buffer_view<const token> seq ) : sequence( seq ), parent( none ), condition( conditional::true_value ) {

		}

		scope( buffer_view<const token> seq, const scope& parent, const conditional& cond ) : sequence( seq ), parent( parent ), condition( cond ) {

		}

	};

	typedef variant<
		//statement,
		// symbols
		scope,
		//statement,
		//ternary,
	> statement;

}}}
