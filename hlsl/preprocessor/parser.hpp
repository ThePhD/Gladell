#pragma once

#include "../token.hpp"
#include "../parser_error.hpp"
#include "../../string.hpp"
#include "../../range.hpp"
#include "../../lexical_numeric_format.hpp"
#include "../../lexical_character_format.hpp"
#include <vector>

namespace gld { namespace hlsl { namespace preprocessor {

	struct sequence {
		buffer_view<token> tokens;
	};

	struct symbol : sequence {
		std::reference_wrapper<token> name;
	};

	struct definition : symbol {
		std::vector<std::reference_wrapper<token>> parameters;
		variant<unit,
			symbol,
			string_view,
			bool,
			uint8, uint16, uint32, uint64,
			int8, int16, int32, int64,
			code_point,
			half, float, double> value;
	};

	struct expression : sequence {

	};

	struct conditional : sequence {
		std::vector<std::reference_wrapper<token>> expression;
	};

	struct inclusion : sequence {

	};
	
	struct scope {
		conditional condition;
		std::vector<definition> defines;
		std::vector<inclusion> includes;
		std::vector<definition> defines;
	};

	struct top_level {
		string data;
		std::vector<scope> scopes;
	};

	class parser {
	private:
		buffer_view<token> tokens;
		top_level toplevel;

	public:
		parser( buffer_view<token> tokenview ) : tokens( std::move( tokenview ) ) {

		}

		void operator () () {

		}
	};

}}}
