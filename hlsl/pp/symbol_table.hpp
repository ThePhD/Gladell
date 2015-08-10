#pragma once

#include "statement.hpp"
#include <unordered_map>

namespace gld { namespace hlsl { namespace pp {

	struct symbol_table {
		// TODO: united table with variant?
		// TODO: measure single-table with variant for items
		// versus double-table with specifics
		// immediate benefit of variant: adding more "Tables" is easier, backing allocator changes simpler
		std::unordered_map<string_view, std::reference_wrapper<definition>> definitions;

		optional<definition&> operator[]( string_view& name ) {
			auto definesfind = definitions.find( name );
			if ( definesfind != definitions.end() ) {
				return definesfind->second;
			}

			return none;
		}

		optional<const definition&> operator[]( string_view& name ) const {
			auto definesfind = definitions.find( name );
			if ( definesfind != definitions.end() ) {
				return definesfind->second;
			}

			return none;
		}
	};

}}}
