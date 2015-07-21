#pragma once

#include "../string.hpp"
#include "../numeric.hpp"
#include "../unit.hpp"
#include "../variant.hpp"
#include "../inclusion_style.hpp"
#include "token_id.hpp"
#include "occurrence.hpp"

namespace gld { namespace hlsl {

	struct token {
		token_id id;
		occurrence where;
		string_view lexeme;
		variant<unit, 
			string,
			inclusion_style,
			bool,
			uint8, uint16, uint32, uint64,  
			int8, int16, int32, int64, 
			code_point,
			half, float, double> value;

		token( token_id id, occurrence where, string_view lexeme = {} )
		: token( id, where, lexeme, unit() ) {

		}
		
		template <typename T0, typename... Tn>
		token( token_id id, occurrence where, string_view lexeme, T0&& arg0, Tn&&... argn )
		: id( id ), lexeme( lexeme ), where( where ), value( std::forward<T0>( arg0 ), std::forward<Tn>( argn )... ) {

		}
	};

}}
