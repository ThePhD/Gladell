#pragma once

#include "../string.hpp"
#include "../int.hpp"
#include "token_id.hpp"
#include "occurrence.hpp"

namespace gld { namespace hlsl {

	struct token {
		token_id id;
		occurrence where;
		string_view lexeme;
		
		token(token_id id, occurrence where, string_view lexeme)
			: id(id), lexeme(lexeme), where(where) {

		}
	};

}}
