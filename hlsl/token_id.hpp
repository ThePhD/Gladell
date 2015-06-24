#pragma once

namespace gld { namespace hlsl {

	enum class token_id {
		// Markers
		stream_begin,
		stream_end,

		// Escaping
		escape,

		// Character Tokens
		open_brace,
		open_curly_bracket = open_brace,
		close_brace,
		close_curly_bracket = close_brace,
		open_bracket,
		open_square_bracket = open_bracket,
		close_bracket,
		close_square_bracket = close_bracket,
		open_parenthesis,
		close_parenthesis,
		plus,
		minus,
		times,
		forward_slash,
		back_slash,
		colon,
		percent,
		equals,
		tilde,
		carat,
		bar,
		ampersand,
		exclamation,
		question_mark,
		single_quotation,
		double_quotation,
		dot,
		comma,
		semi_colon,
		hash,
		whitespace,
		newlines,
		dot_dot_dot,

		// Preprocessor
		preprocessor_hash,
		preprocessor_block_begin,
		preprocessor_block_end,
		preprocessor_statement_begin,
		preprocessor_statement_end,
		preprocessor_escaped_newline,
		preprocessor_variadic_arguments,
		preprocessor_define,
		preprocessor_undef,
		preprocessor_if,
		preprocessor_elif,
		preprocessor_else,
		preprocessor_ifdef,
		preprocessor_ifndef,
		preprocessor_endif,
		preprocessor_include,
		preprocessor_line,
		preprocessor_pragma,
		preprocessor_error,
		preprocessor_warning,
		preprocessor_pragma_once,
		preprocessor_pragma_pack_matrix,
		preprocessor_pragma_register_define,
		preprocessor_pragma_warning,
		preprocessor_pragma_message,
		preprocessor_pragma_custom,
		preprocessor_stringizing,
		preprocessor_charizing,
		preprocessor_token_pasting,

		// Reserved
		reserved_auto,
		reserved_dynamic_cast,
		reserved_const_cast,
		reserved_reinterpret_cast,
		reserved_static_cast,
		reserved_template,
		reserved_typename,
		reserved_char,
		reserved_short,
		reserved_long,
		reserved_signed,
		reserved_unsigned,
		reserved_namespace,
		reserved_enum,
		reserved_union,
		reserved_class,
		reserved_friend,
		reserved_private,
		reserved_public,
		reserved_protected,
		reserved_try,
		reserved_catch,
		reserved_default,
		reserved_new,
		reserved_delete,
		reserved_operator,
		reserved_explicit,
		reserved_goto,
		reserved_mutable,
		reserved_sizeof,
		reserved_this,
		reserved_virtual,

		// Operators, combinations of character tokens can yield these instead
		// Increment decrement operators
		increment,
		decrement,
		// Comparison operators
		less_than,
		greater_than,
		equal_to,
		not_equal_to,
		less_than_or_equal_to,
		greater_than_or_equal_to,
		// Bitwise operators...
		boolean_not,
		boolean_or,
		boolean_and,
		boolean_xor,
		left_shift,
		right_shift,
		// ~= not in the language?
		boolean_not_assignment,
		boolean_or_assignment,
		boolean_and_assignment,
		boolean_xor_assignment,
		left_shift_assignment,
		right_shift_assignment,
		// Expression operators
		expression_or,
		expression_and,
		assignment,
		// Math operators
		add,
		subtract,
		multiply,
		divide,
		modulus,
		add_assignment,
		subtract_assignment,
		multiply_assignment,
		divide_assignment,
		modulus_assignment,
		// access operators
		static_access,
		struct_access,
		trailing_dots,

		// Literals
		float_literal,
		integer_literal,
		integer_octal_literal,
		integer_hex_literal,
		character_literal_begin,
		character_literal_end,
		character_literal,
		string_literal_begin,
		string_literal_end,
		string_literal,
		boolean_literal,
		boolean_literal_true,
		boolean_literal_false,

		// Comments
		block_comment_begin,
		block_comment_end,
		line_comment_begin,
		line_comment_end,
		comment_text,

		// Flow control
		flow_control_if,
		flow_control_else,
		flow_control_for,
		flow_control_do,
		flow_control_while,
		flow_control_switch,
		flow_control_case,
		flow_control_break,
		flow_control_continue,
		flow_control_return,

		// Flow control modifiers
		// For/while loop modifiers
		unroll_attribute,
		loop_attribute,
		fastop_attribute,
		allow_uav_condition_attribute,

		// If / switch statement modifiers
		flatten_attribute,
		branch_attribute,
		// switch statement modifiers only
		forcecase_attribute,
		call_attribute,

		// Keywords
		keyword_sampler,
		keyword_texture,
		keyword_typedef,
		keyword_struct,
		keyword_interface,
		keyword_cbuffer,
		keyword_tbuffer,
		keyword_register,
		keyword_discard,
		keyword_const,
		keyword_packoffset,
		keyword_row_major,
		keyword_column_major,
		keyword_pass,
		keyword_technique,
		keyword_technique_sm4,
		keyword_technique_sm5,
		keyword_compile,
		keyword_vertex_shader,
		keyword_hull_shader,
		keyword_domain_shader,
		keyword_geometry_shader,
		keyword_pixel_shader,
		keyword_compute_shader,
		keyword_compile_sm4,
		keyword_vertex_shader_sm4,
		keyword_hull_shader_sm4,
		keyword_domain_shader_sm4,
		keyword_geometry_shader_sm4,
		keyword_pixel_shader_sm4,
		keyword_compute_shader_sm4,
		keyword_inline,
		keyword_static,
		keyword_extern,
		keyword_volatile,

		// Input Modifiers
		uniform,
		in,
		out,
		inout,
		unorm,
		snorm,
		// Interpolation Modifiers
		linear,
		centroid,
		nointerpolation,
		noperspective,
		sample,
		// variable modifiers / storage classes
		precise,
		shared,
		groupshared,
		clipplanes,
		maxvertexcount,

		// Geometry shader primitives
		primitive_point,
		primitive_line,
		primitive_triangle,
		primitive_lineadj,
		primitive_triangleadj,

		// Identifier
		identifier,

		// Built-in types
		type_string,
		type_matrix,
		type_vector,
		type_half,
		type_half_vector,
		type_half_matrix,
		type_dword,
		type_dword_vector,
		type_dword_matrix,
		type_float,
		type_float_vector,
		type_float_matrix,
		type_double,
		type_double_vector,
		type_double_matrix,
		type_bool,
		type_bool_vector,
		type_bool_matrix,
		type_integer,
		type_integer_vector,
		type_integer_matrix,
		type_unsigned_integer,
		type_unsigned_integer_vector,
		type_unsigned_integer_matrix,
		type_min16_integer,
		type_min16_float,
		type_min10_float,
		type_min12_integer,
		type_min16_unsigned_integer,
		type_buffer,
		type_point_stream,
		type_line_stream,
		type_triangle_stream,
		type_sampler_1d,
		type_sampler_2d,
		type_sampler_3d,
		type_sampler_cube,
		type_sampler_comparison_state,
		type_sampler_state,
		type_sampler_state_sm4,
		type_sampler_state_address_u,
		type_sampler_state_address_v,
		type_sampler_state_address_w,
		type_sampler_state_border_color,
		type_sampler_state_filter,
		type_sampler_state_max_anisotropy,
		type_sampler_state_max_lod,
		type_sampler_state_min_lod,
		type_sampler_state_mip_load_bias,
		type_sampler_comparison_state_comparison_func,
		type_texture,
		type_texture_1d,
		type_texture_1d_array,
		type_texture_2d,
		type_texture_2d_array,
		type_texture_3d,
		type_texture_cube,

		profile_vs_20,
		profile_ps_20,

		profile_vs_2a,
		profile_ps_2a,

		profile_vs_2b,
		profile_ps_2b,

		profile_vs_2sw,
		profile_ps_2sw,

		profile_vs_40_level_90,
		profile_ps_40_level_90,
		profile_lib_40_level_90,

		profile_vs_40_level_91,
		profile_ps_40_level_91,
		profile_lib_40_level_91,

		profile_vs_40_level_93,
		profile_ps_40_level_93,
		profile_lib_40_level_93,

		profile_vs_30,
		profile_ps_30,

		profile_vs_40,
		profile_gs_40,
		profile_ps_40,
		profile_cs_40,
		profile_lib_40,

		profile_vs_41,
		profile_gs_41,
		profile_ps_41,
		profile_cs_41,
		profile_lib_41,

		profile_vs_50,
		profile_ds_50,
		profile_hs_50,
		profile_gs_50,
		profile_ps_50,
		profile_cs_50,
		profile_lib_50,

		profile_vs_51,
		profile_ds_51,
		profile_hs_51,
		profile_gs_51,
		profile_ps_51,
		profile_cs_51,
		profile_lib_51,
	};

	inline string_view to_string( token_id id ) {
		switch ( id ) {
			// Markers
		case token_id::stream_begin:
			return "stream_begin";
		case token_id::stream_end:
			return "stream_end";

			// Escaping
		case token_id::escape:
			return "escape";

			// Character Tokens
		case token_id::open_brace:
			return "open_brace";
		case token_id::close_brace:
			return "close_brace";
		case token_id::open_bracket:
			return "open_bracket";
		case token_id::close_bracket:
			return "close_bracket";
		case token_id::open_parenthesis:
			return "open_parenthesis";
		case token_id::close_parenthesis:
			return "close_parenthesis";
		case token_id::plus:
			return "plus";
		case token_id::minus:
			return "minus";
		case token_id::times:
			return "times";
		case token_id::forward_slash:
			return "forward_slash";
		case token_id::back_slash:
			return "back_slash";
		case token_id::colon:
			return "colon";
		case token_id::percent:
			return "percent";
		case token_id::equals:
			return "equals";
		case token_id::tilde:
			return "tilde";
		case token_id::carat:
			return "carat";
		case token_id::bar:
			return "bar";
		case token_id::ampersand:
			return "ampersand";
		case token_id::exclamation:
			return "exclamation";
		case token_id::question_mark:
			return "question_mark";
		case token_id::single_quotation:
			return "single_quotation";
		case token_id::double_quotation:
			return "double_quotation";
		case token_id::dot:
			return "dot";
		case token_id::comma:
			return "comma";
		case token_id::semi_colon:
			return "semi_colon";
		case token_id::hash:
			return "hash";
		case token_id::whitespace:
			return "whitespace";
		case token_id::newlines:
			return "newlines";

			// Preprocessor
		case token_id::preprocessor_block_begin:
			return "preprocessor_block_begin";
		case token_id::preprocessor_block_end:
			return "preprocessor_block_end";
		case token_id::preprocessor_statement_begin:
			return "preprocessor_statement_begin";
		case token_id::preprocessor_statement_end:
			return "preprocessor_statement_end";
		case token_id::preprocessor_escaped_newline:
			return "preprocessor_escaped_newline";
		case token_id::preprocessor_define:
			return "preprocessor_define";
		case token_id::preprocessor_undef:
			return "preprocessor_undef";
		case token_id::preprocessor_if:
			return "preprocessor_if";
		case token_id::preprocessor_elif:
			return "preprocessor_elif";
		case token_id::preprocessor_else:
			return "preprocessor_else";
		case token_id::preprocessor_ifdef:
			return "preprocessor_ifdef";
		case token_id::preprocessor_ifndef:
			return "preprocessor_ifndef";
		case token_id::preprocessor_endif:
			return "preprocessor_endif";
		case token_id::preprocessor_include:
			return "preprocessor_include";
		case token_id::preprocessor_line:
			return "preprocessor_line";
		case token_id::preprocessor_pragma:
			return "preprocessor_pragma";
		case token_id::preprocessor_error:
			return "preprocessor_error";
		case token_id::preprocessor_warning:
			return "preprocessor_warning";
		case token_id::preprocessor_pragma_once:
			return "preprocessor_pragma_once";
		case token_id::preprocessor_pragma_pack_matrix:
			return "preprocessor_pragma_pack_matrix";
		case token_id::preprocessor_pragma_register_define:
			return "preprocessor_pragma_register_define";
		case token_id::preprocessor_pragma_warning:
			return "preprocessor_pragma_warning";
		case token_id::preprocessor_pragma_message:
			return "preprocessor_pragma_message";
		case token_id::preprocessor_pragma_custom:
			return "preprocessor_pragma_custom";
		case token_id::preprocessor_stringizing:
			return "preprocessor_stringizing";
		case token_id::preprocessor_charizing:
			return "preprocessor_charizing";
		case token_id::preprocessor_token_pasting:
			return "preprocessor_token_pasting";

			// Reserved
		case token_id::reserved_auto:
			return "reserved_auto";
		case token_id::reserved_dynamic_cast:
			return "reserved_dynamic_cast";
		case token_id::reserved_const_cast:
			return "reserved_const_cast";
		case token_id::reserved_reinterpret_cast:
			return "reserved_reinterpret_cast";
		case token_id::reserved_static_cast:
			return "reserved_static_cast";
		case token_id::reserved_template:
			return "reserved_template";
		case token_id::reserved_typename:
			return "reserved_typename";
		case token_id::reserved_char:
			return "reserved_char";
		case token_id::reserved_short:
			return "reserved_short";
		case token_id::reserved_long:
			return "reserved_long";
		case token_id::reserved_signed:
			return "reserved_signed";
		case token_id::reserved_unsigned:
			return "reserved_unsigned";
		case token_id::reserved_namespace:
			return "reserved_namespace";
		case token_id::reserved_enum:
			return "reserved_enum";
		case token_id::reserved_union:
			return "reserved_union";
		case token_id::reserved_class:
			return "reserved_class";
		case token_id::reserved_friend:
			return "reserved_friend";
		case token_id::reserved_private:
			return "reserved_private";
		case token_id::reserved_public:
			return "reserved_public";
		case token_id::reserved_protected:
			return "reserved_protected";
		case token_id::reserved_try:
			return "reserved_try";
		case token_id::reserved_catch:
			return "reserved_catch";
		case token_id::reserved_default:
			return "reserved_default";
		case token_id::reserved_new:
			return "reserved_new";
		case token_id::reserved_delete:
			return "reserved_delete";
		case token_id::reserved_operator:
			return "reserved_operator";
		case token_id::reserved_explicit:
			return "reserved_explicit";
		case token_id::reserved_goto:
			return "reserved_goto";
		case token_id::reserved_mutable:
			return "reserved_mutable";
		case token_id::reserved_sizeof:
			return "reserved_sizeof";
		case token_id::reserved_this:
			return "reserved_this";
		case token_id::reserved_virtual:
			return "reserved_virtual";

			// Operators, combinations of character tokens can yield these instead
			// Increment decrement operators
		case token_id::increment:
			return "increment";
		case token_id::decrement:
			return "decrement";
			// Comparison operators
		case token_id::less_than:
			return "less_than";
		case token_id::greater_than:
			return "greater_than";
		case token_id::equal_to:
			return "equal_to";
		case token_id::not_equal_to:
			return "not_equal_to";
		case token_id::less_than_or_equal_to:
			return "less_than_or_equal_to";
		case token_id::greater_than_or_equal_to:
			return "greater_than_or_equal_to";
			// Bitwise operators...
		case token_id::boolean_not:
			return "boolean_not";
		case token_id::boolean_or:
			return "boolean_or";
		case token_id::boolean_and:
			return "boolean_and";
		case token_id::boolean_xor:
			return "boolean_xor";
		case token_id::left_shift:
			return "left_shift";
		case token_id::right_shift:
			return "right_shift";
			// ~= not in the language?
		case token_id::boolean_not_assignment:
			return "boolean_not_assignment";
		case token_id::boolean_or_assignment:
			return "boolean_or_assignment";
		case token_id::boolean_and_assignment:
			return "boolean_and_assignment";
		case token_id::boolean_xor_assignment:
			return "boolean_xor_assignment";
		case token_id::left_shift_assignment:
			return "left_shift_assignment";
		case token_id::right_shift_assignment:
			return "right_shift_assignment";
			// Expression operators
		case token_id::expression_or:
			return "expression_or";
		case token_id::expression_and:
			return "expression_and";
		case token_id::assignment:
			return "assignment";
			// Math operators
		case token_id::add:
			return "add";
		case token_id::subtract:
			return "subtract";
		case token_id::multiply:
			return "multiply";
		case token_id::divide:
			return "divide";
		case token_id::modulus:
			return "modulus";
		case token_id::add_assignment:
			return "add_assignment";
		case token_id::subtract_assignment:
			return "subtract_assignment";
		case token_id::multiply_assignment:
			return "multiply_assignment";
		case token_id::divide_assignment:
			return "divide_assignment";
		case token_id::modulus_assignment:
			return "modulus_assignment";
			// access operators
		case token_id::static_access:
			return "static_access";
		case token_id::struct_access:
			return "struct_access";
		case token_id::trailing_dots:
			return "trailing_dots";

			// Literals
		case token_id::float_literal:
			return "float_literal";
		case token_id::integer_literal:
			return "integer_literal";
		case token_id::integer_octal_literal:
			return "integer_octal_literal";
		case token_id::integer_hex_literal:
			return "integer_hex_literal";
		case token_id::character_literal_begin:
			return "character_literal_begin";
		case token_id::character_literal_end:
			return "character_literal_end";
		case token_id::character_literal:
			return "character_literal";
		case token_id::string_literal_begin:
			return "string_literal_begin";
		case token_id::string_literal_end:
			return "string_literal_end";
		case token_id::string_literal:
			return "string_literal";
		case token_id::boolean_literal:
			return "boolean_literal";
		case token_id::boolean_literal_true:
			return "boolean_literal_true";
		case token_id::boolean_literal_false:
			return "boolean_literal_false";

			// Comments
		case token_id::block_comment_begin:
			return "block_comment_begin";
		case token_id::block_comment_end:
			return "block_comment_end";
		case token_id::line_comment_begin:
			return "line_comment_begin";
		case token_id::line_comment_end:
			return "line_comment_end";
		case token_id::comment_text:
			return "comment_text";

			// Flow control
		case token_id::flow_control_if:
			return "flow_control_if";
		case token_id::flow_control_else:
			return "flow_control_else";
		case token_id::flow_control_for:
			return "flow_control_for";
		case token_id::flow_control_do:
			return "flow_control_do";
		case token_id::flow_control_while:
			return "flow_control_while";
		case token_id::flow_control_switch:
			return "flow_control_switch";
		case token_id::flow_control_case:
			return "flow_control_case";
		case token_id::flow_control_break:
			return "flow_control_break";
		case token_id::flow_control_continue:
			return "flow_control_continue";
		case token_id::flow_control_return:
			return "flow_control_return";

			// Flow control modifiers
			// For/while loop modifiers
		case token_id::unroll_attribute:
			return "unroll_attribute";
		case token_id::loop_attribute:
			return "loop_attribute";
		case token_id::fastop_attribute:
			return "fastop_attribute";
		case token_id::allow_uav_condition_attribute:
			return "allow_uav_condition_attribute";

			// If / switch statement modifiers
		case token_id::flatten_attribute:
			return "flatten_attribute";
		case token_id::branch_attribute:
			return "branch_attribute";
			// switch statement modifiers only
		case token_id::forcecase_attribute:
			return "forcecase_attribute";
		case token_id::call_attribute:
			return "call_attribute";

			// Keywords
		case token_id::keyword_sampler:
			return "keyword_sampler";
		case token_id::keyword_texture:
			return "keyword_texture";
		case token_id::keyword_typedef:
			return "keyword_typedef";
		case token_id::keyword_struct:
			return "keyword_struct";
		case token_id::keyword_interface:
			return "keyword_interface";
		case token_id::keyword_cbuffer:
			return "keyword_cbuffer";
		case token_id::keyword_tbuffer:
			return "keyword_tbuffer";
		case token_id::keyword_register:
			return "keyword_register";
		case token_id::keyword_discard:
			return "keyword_discard";
		case token_id::keyword_const:
			return "keyword_const";
		case token_id::keyword_packoffset:
			return "keyword_packoffset";
		case token_id::keyword_row_major:
			return "keyword_row_major";
		case token_id::keyword_column_major:
			return "keyword_column_major";
		case token_id::keyword_pass:
			return "keyword_pass";
		case token_id::keyword_technique:
			return "keyword_technique";
		case token_id::keyword_technique_sm4:
			return "keyword_technique_sm4";
		case token_id::keyword_technique_sm5:
			return "keyword_technique_sm5";
		case token_id::keyword_compile:
			return "keyword_compile";
		case token_id::keyword_vertex_shader:
			return "keyword_vertex_shader";
		case token_id::keyword_hull_shader:
			return "keyword_hull_shader";
		case token_id::keyword_domain_shader:
			return "keyword_domain_shader";
		case token_id::keyword_geometry_shader:
			return "keyword_geometry_shader";
		case token_id::keyword_pixel_shader:
			return "keyword_pixel_shader";
		case token_id::keyword_compute_shader:
			return "keyword_compute_shader";
		case token_id::keyword_compile_sm4:
			return "keyword_compile_sm4";
		case token_id::keyword_vertex_shader_sm4:
			return "keyword_vertex_shader_sm4";
		case token_id::keyword_hull_shader_sm4:
			return "keyword_hull_shader_sm4";
		case token_id::keyword_domain_shader_sm4:
			return "keyword_domain_shader_sm4";
		case token_id::keyword_geometry_shader_sm4:
			return "keyword_geometry_shader_sm4";
		case token_id::keyword_pixel_shader_sm4:
			return "keyword_pixel_shader_sm4";
		case token_id::keyword_compute_shader_sm4:
			return "keyword_compute_shader_sm4";
		case token_id::keyword_inline:
			return "keyword_inline";
		case token_id::keyword_static:
			return "keyword_static";
		case token_id::keyword_extern:
			return "keyword_extern";
		case token_id::keyword_volatile:
			return "keyword_volatile";

			// Input Modifiers
		case token_id::uniform:
			return "uniform";
		case token_id::in:
			return "in";
		case token_id::out:
			return "out";
		case token_id::inout:
			return "inout";
		case token_id::unorm:
			return "unorm";
		case token_id::snorm:
			return "snorm";
			// Interpolation Modifiers
		case token_id::linear:
			return "linear";
		case token_id::centroid:
			return "centroid";
		case token_id::nointerpolation:
			return "nointerpolation";
		case token_id::noperspective:
			return "noperspective";
		case token_id::sample:
			return "sample";
			// variable modifiers / storage classes
		case token_id::precise:
			return "precise";
		case token_id::shared:
			return "shared";
		case token_id::groupshared:
			return "groupshared";
		case token_id::clipplanes:
			return "clipplanes";
		case token_id::maxvertexcount:
			return "maxvertexcount";

			// Geometry shader primitives
		case token_id::primitive_point:
			return "primitive_point";
		case token_id::primitive_line:
			return "primitive_line";
		case token_id::primitive_triangle:
			return "primitive_triangle";
		case token_id::primitive_lineadj:
			return "primitive_lineadj";
		case token_id::primitive_triangleadj:
			return "primitive_triangleadj";

			// Identifier
		case token_id::identifier:
			return "identifier";

			// Built-in types
		case token_id::type_string:
			return "type_string";
		case token_id::type_matrix:
			return "type_matrix";
		case token_id::type_vector:
			return "type_vector";
		case token_id::type_half:
			return "type_half";
		case token_id::type_half_vector:
			return "type_half_vector";
		case token_id::type_half_matrix:
			return "type_half_matrix";
		case token_id::type_dword:
			return "type_dword";
		case token_id::type_dword_vector:
			return "type_dword_vector";
		case token_id::type_dword_matrix:
			return "type_dword_matrix";
		case token_id::type_float:
			return "type_float";
		case token_id::type_float_vector:
			return "type_float_vector";
		case token_id::type_float_matrix:
			return "type_float_matrix";
		case token_id::type_double:
			return "type_double";
		case token_id::type_double_vector:
			return "type_double_vector";
		case token_id::type_double_matrix:
			return "type_double_matrix";
		case token_id::type_bool:
			return "type_bool";
		case token_id::type_bool_vector:
			return "type_bool_vector";
		case token_id::type_bool_matrix:
			return "type_bool_matrix";
		case token_id::type_integer:
			return "type_integer";
		case token_id::type_integer_vector:
			return "type_integer_vector";
		case token_id::type_integer_matrix:
			return "type_integer_matrix";
		case token_id::type_unsigned_integer:
			return "type_unsigned_integer";
		case token_id::type_unsigned_integer_vector:
			return "type_unsigned_integer_vector";
		case token_id::type_unsigned_integer_matrix:
			return "type_unsigned_integer_matrix";
		case token_id::type_min16_integer:
			return "type_min16_integer";
		case token_id::type_min16_float:
			return "type_min16_float";
		case token_id::type_min10_float:
			return "type_min10_float";
		case token_id::type_min12_integer:
			return "type_min12_integer";
		case token_id::type_min16_unsigned_integer:
			return "type_min16_unsigned_integer";
		case token_id::type_buffer:
			return "type_buffer";
		case token_id::type_point_stream:
			return "type_point_stream";
		case token_id::type_line_stream:
			return "type_line_stream";
		case token_id::type_triangle_stream:
			return "type_triangle_stream";
		case token_id::type_sampler_1d:
			return "type_sampler_1d";
		case token_id::type_sampler_2d:
			return "type_sampler_2d";
		case token_id::type_sampler_3d:
			return "type_sampler_3d";
		case token_id::type_sampler_cube:
			return "type_sampler_cube";
		case token_id::type_sampler_comparison_state:
			return "type_sampler_comparison_state";
		case token_id::type_sampler_state:
			return "type_sampler_state";
		case token_id::type_sampler_state_sm4:
			return "type_sampler_state_sm4";
		case token_id::type_sampler_state_address_u:
			return "type_sampler_state_address_u";
		case token_id::type_sampler_state_address_v:
			return "type_sampler_state_address_v";
		case token_id::type_sampler_state_address_w:
			return "type_sampler_state_address_w";
		case token_id::type_sampler_state_border_color:
			return "type_sampler_state_border_color";
		case token_id::type_sampler_state_filter:
			return "type_sampler_state_filter";
		case token_id::type_sampler_state_max_anisotropy:
			return "type_sampler_state_max_anisotropy";
		case token_id::type_sampler_state_max_lod:
			return "type_sampler_state_max_lod";
		case token_id::type_sampler_state_min_lod:
			return "type_sampler_state_min_lod";
		case token_id::type_sampler_state_mip_load_bias:
			return "type_sampler_state_mip_load_bias";
		case token_id::type_sampler_comparison_state_comparison_func:
			return "type_sampler_comparison_state_comparison_func";
		case token_id::type_texture:
			return "type_texture";
		case token_id::type_texture_1d:
			return "type_texture_1d";
		case token_id::type_texture_1d_array:
			return "type_texture_1d_array";
		case token_id::type_texture_2d:
			return "type_texture_2d";
		case token_id::type_texture_2d_array:
			return "type_texture_2d_array";
		case token_id::type_texture_3d:
			return "type_texture_3d";
		case token_id::type_texture_cube:
			return "type_texture_cube";

		case token_id::profile_vs_20:
			return "profile_vs_20";
		case token_id::profile_ps_20:
			return "profile_ps_20";

		case token_id::profile_vs_2a:
			return "profile_vs_2a";
		case token_id::profile_ps_2a:
			return "profile_ps_2a";

		case token_id::profile_vs_2b:
			return "profile_vs_2b";
		case token_id::profile_ps_2b:
			return "profile_ps_2b";

		case token_id::profile_vs_2sw:
			return "profile_vs_2sw";
		case token_id::profile_ps_2sw:
			return "profile_ps_2sw";

		case token_id::profile_vs_40_level_90:
			return "profile_vs_40_level_90";
		case token_id::profile_ps_40_level_90:
			return "profile_ps_40_level_90";
		case token_id::profile_lib_40_level_90:
			return "profile_lib_40_level_90";

		case token_id::profile_vs_40_level_91:
			return "profile_vs_40_level_91";
		case token_id::profile_ps_40_level_91:
			return "profile_ps_40_level_91";
		case token_id::profile_lib_40_level_91:
			return "profile_lib_40_level_91";

		case token_id::profile_vs_40_level_93:
			return "profile_vs_40_level_93";
		case token_id::profile_ps_40_level_93:
			return "profile_ps_40_level_93";
		case token_id::profile_lib_40_level_93:
			return "profile_lib_40_level_93";

		case token_id::profile_vs_30:
			return "profile_vs_30";
		case token_id::profile_ps_30:
			return "profile_ps_30";

		case token_id::profile_vs_40:
			return "profile_vs_40";
		case token_id::profile_gs_40:
			return "profile_gs_40";
		case token_id::profile_ps_40:
			return "profile_ps_40";
		case token_id::profile_cs_40:
			return "profile_cs_40";
		case token_id::profile_lib_40:
			return "profile_lib_40";

		case token_id::profile_vs_41:
			return "profile_vs_41";
		case token_id::profile_gs_41:
			return "profile_gs_41";
		case token_id::profile_ps_41:
			return "profile_ps_41";
		case token_id::profile_cs_41:
			return "profile_cs_41";
		case token_id::profile_lib_41:
			return "profile_lib_41";

		case token_id::profile_vs_50:
			return "profile_vs_50";
		case token_id::profile_ds_50:
			return "profile_ds_50";
		case token_id::profile_hs_50:
			return "profile_hs_50";
		case token_id::profile_gs_50:
			return "profile_gs_50";
		case token_id::profile_ps_50:
			return "profile_ps_50";
		case token_id::profile_cs_50:
			return "profile_cs_50";
		case token_id::profile_lib_50:
			return "profile_lib_50";

		case token_id::profile_vs_51:
			return "profile_vs_51";
		case token_id::profile_ds_51:
			return "profile_ds_51";
		case token_id::profile_hs_51:
			return "profile_hs_51";
		case token_id::profile_gs_51:
			return "profile_gs_51";
		case token_id::profile_ps_51:
			return "profile_ps_51";
		case token_id::profile_cs_51:
			return "profile_cs_51";
		case token_id::profile_lib_51:
			return "profile_lib_51";
		default:
			throw - 1;
			return "INVALID_ENUM_VALUE";
		}
	}

}}
