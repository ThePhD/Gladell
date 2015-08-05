#pragma once

#include "string.hpp"

namespace gld {

	enum class inclusion_style {
		quote,
		angle_bracket
	};

	inline string_view to_string( inclusion_style i ) {
		switch ( i ) {
		case inclusion_style::angle_bracket:
			return "angle bracket";
		case inclusion_style::quote:
			return "quote";
		}
		return string_view();
	}

}
