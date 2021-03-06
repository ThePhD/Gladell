#pragma once

#include "../../variant.hpp"
#include "construct.hpp"
#include "expression.hpp"
#include "statement.hpp"
#include <vector>

namespace gld { namespace hlsl { namespace pp {

	struct parse_tree : public block {
#define GLD_STORAGE( x ) \
		private: std::vector<x> x##_storage; \
		public: x& operator[]( index_ref<x> i ) { return x##_storage[i.get()]; } \
		public: const x& operator[]( index_ref<x> i ) const { return x##_storage[i.get()]; } \
		public: template <typename... Tn> index_ref<x> make_##x ( Tn&&... argn ) { index_ref<x> i = x##_storage.size(); x##_storage.emplace_back( std::forward<Tn>( argn )... ); return i; }
		
		// Expression storage
		GLD_STORAGE( ternary_expression );

		GLD_STORAGE( not_expression );
		GLD_STORAGE( less_expression );
		GLD_STORAGE( less_equals_expression );
		GLD_STORAGE( greater_expression );
		GLD_STORAGE( greater_equals_expression );
		GLD_STORAGE( equals_expression );

		GLD_STORAGE( negate_expression );
		GLD_STORAGE( and_expression );
		GLD_STORAGE( or_expression );
		GLD_STORAGE( xor_expression );

		GLD_STORAGE( add_subtract_expression );
		GLD_STORAGE( multiply_divide_expression );
		GLD_STORAGE( modulus_expression );

		GLD_STORAGE( stringizing_expression );
		GLD_STORAGE( charizing_expression );
		GLD_STORAGE( token_pasting_expression );

		// Statement Storage
		GLD_STORAGE( block );
		GLD_STORAGE( if_elseif_else );

#undef GLD_STORAGE

	public:
		
		parse_tree( ) {

		}

	};

}}}
