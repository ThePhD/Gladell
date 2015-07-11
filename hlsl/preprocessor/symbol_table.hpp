#pragma once

#include "statement.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	struct symbol_table {
		// TODO: united table with variant?
		// TODO: measure single-table with variant for items
		// versus double-table with specifics
		// immediate benefit of variant: adding more "Tables" is easier, backing allocator changes simpler
		std::unordered_map<string_view, definition> defines;
		std::unordered_map<string_view, function> functions;

		optional<variant<definition&, function&>> operator[]( string_view& name ) {
			auto definesfind = defines.find( name );
			if ( definesfind != defines.end() ) {
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
