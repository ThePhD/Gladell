#pragma once

namespace gld { namespace hlsl { namespace pp {

	enum class conditional_origin {
		if_,
		if_def,
		if_n_def,
		else_if,
		else_if_def,
		else_if_n_def,
		else_,
	};

}}}
