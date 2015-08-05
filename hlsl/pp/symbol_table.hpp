#pragma once

#include "statement.hpp"

namespace gld { namespace hlsl { namespace pp {

	struct symbol_table {
		// TODO: united table with variant?
		// TODO: measure single-table with variant for items
		// versus double-table with specifics
		// immediate benefit of variant: adding more "Tables" is easier, backing allocator changes simpler
		std::unordered_map<string_view, std::reference_wrapper<variable>> variables;
		std::unordered_map<string_view, std::reference_wrapper<function>> functions;

		optional<variant<variable&, function&>> operator[]( string_view& name ) {
			auto definesfind = variables.find( name );
			if ( definesfind != variables.end() ) {
				return definesfind->second;
			}
			auto functionsfind = functions.find( name );
			if ( functionsfind != functions.end() ) {
				return functionsfind->second;
			}

			return none;
		}
	};

}}}
