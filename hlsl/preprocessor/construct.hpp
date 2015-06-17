#pragma once

#include "../token.hpp"
#include "../../optional.hpp"
#include "../../string.hpp"
#include "../../range.hpp"
#include <functional>
#include <vector>
#include <unordered_map>

namespace gld { namespace hlsl { namespace preprocessor {

	struct sequence {
		buffer_view<token> tokens;
	};

	struct symbol : sequence {
		std::reference_wrapper<token> name;
	};

	struct expression : sequence {

	};

	struct definition : symbol {
		variant<unit,
			expression,
			bool,
			uint8, uint16, uint32, uint64,
			int8, int16, int32, int64,
			code_point,
			half, float, double> value;
	};

	struct function : symbol {
		std::vector<std::reference_wrapper<token>> parameters;
		expression substitution;
		std::vector<std::reference_wrapper<token>> expression_substitutions;
	};

	struct binary_expression : expression {
		std::reference_wrapper<expression> left;
		std::reference_wrapper<expression> right;
	};

	struct unary_expression : expression {
		std::reference_wrapper<expression> target;
	};

	struct conditional : expression {
		std::reference_wrapper<expression> condition;
	};

	struct inclusion : expression {
		string_view target;
	};

	struct scope : sequence {
		optional<conditional> condition;
		std::vector<std::reference_wrapper<sequence>> sequences;
	};

	struct top_level {
		std::unordered_map<string_view, definition> symbols;
		std::unordered_map<string_view, function> functions;
		std::vector<scope> scopes;

		top_level( buffer_view<token> tokenview ) : scopes( 1 ) {
			// Covers the degenerate case in memory where
			// every single token resolves to an expression
			// (not even sure that's possible...?)
			scopes.reserve( tokenview.size() );
		}
	};

}}}
