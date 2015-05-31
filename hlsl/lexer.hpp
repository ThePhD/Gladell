#pragma once

#include "token.hpp"
#include "../string.hpp"
#include "../unicode.hpp"
#include "../int.hpp"
#include "../character_type.hpp"
#include <unordered_map>

namespace gld { namespace hlsl {

	class lexer {
	private:
		std::unordered_map<string_view, token_id> keywords;

		enum class state_routine {
			normal,
			preprocessor,
			block_comment,
			line_comment,
		};

		struct character_state {
			character_type character_type;
			bool line_terminator;
			occurrence consumed_where;
		};

	public:
		lexer() {
			keywords.insert({ 
				{ "auto", token_id::reserved_auto },
				{ "catch", token_id::reserved_catch },
				{ "char", token_id::reserved_char },
				{ "class", token_id::reserved_class },
				{ "const_cast", token_id::reserved_const_cast },
				{ "default", token_id::reserved_default },
				{ "delete", token_id::reserved_delete },
				{ "dynamic_cast", token_id::reserved_dynamic_cast },
				{ "enum", token_id::reserved_enum },
				{ "explicit", token_id::reserved_explicit },
				{ "friend", token_id::reserved_friend },
				{ "goto", token_id::reserved_goto },
				{ "long", token_id::reserved_long },
				{ "mutable", token_id::reserved_mutable },
				{ "namespace", token_id::reserved_namespace },
				{ "new", token_id::reserved_new },
				{ "operator", token_id::reserved_operator },
				{ "private", token_id::reserved_private },
				{ "public", token_id::reserved_public },
				{ "protected", token_id::reserved_protected },
				{ "reinterpret_case", token_id::reserved_reinterpret_cast },
				{ "short", token_id::reserved_short },
				{ "signed", token_id::reserved_signed },
				{ "sizeof", token_id::reserved_sizeof },
				{ "static_cast", token_id::reserved_static_cast },
				{ "template", token_id::reserved_template },
				{ "typename", token_id::reserved_typename },
				{ "this", token_id::reserved_this },
				{ "try", token_id::reserved_try },
				{ "union", token_id::reserved_union },
				{ "unsigned", token_id::reserved_unsigned },
				{ "virtual", token_id::reserved_virtual },

				{ "if", token_id::flow_control_if },
				{ "else", token_id::flow_control_else },
				{ "for", token_id::flow_control_for },
				{ "do", token_id::flow_control_do },
				{ "while", token_id::flow_control_while },
				{ "case", token_id::flow_control_case },
				{ "break", token_id::flow_control_break },
				{ "continue", token_id::flow_control_continue },
				{ "return", token_id::flow_control_return },

				{ "unroll", token_id::unroll_attribute },
				{ "loop", token_id::loop_attribute },
				{ "fastop", token_id::fastop_attribute },
				{ "allow_uav_condition", token_id::allow_uav_condition_attribute },
				{ "flatten", token_id::flatten_attribute },
				{ "branch", token_id::branch_attribute },
				{ "forcecase", token_id::forcecase_attribute },
				{ "call", token_id::call_attribute },

				{ "typedef", token_id::keyword_typedef },
				{ "struct", token_id::keyword_struct },
				{ "interface", token_id::keyword_interface },
				{ "cbuffer", token_id::keyword_cbuffer },
				{ "tbuffer", token_id::keyword_tbuffer },
				{ "register", token_id::keyword_register },
				{ "discard", token_id::keyword_discard },
				{ "inline", token_id::keyword_inline },
				{ "static", token_id::keyword_static },
				{ "extern", token_id::keyword_extern },
				{ "volatile", token_id::keyword_volatile },
				{ "in", token_id::in },
				{ "inout", token_id::inout },
				{ "out", token_id::out },
				{ "uniform", token_id::uniform },
				{ "unorm", token_id::unorm },
				{ "snorm", token_id::snorm },
				{ "const", token_id::keyword_const },
				{ "packoffset", token_id::keyword_packoffset },
				{ "row_major", token_id::keyword_row_major },
				{ "column_major", token_id::keyword_column_major },
				
				{ "point", token_id::primitive_point },
				{ "line", token_id::primitive_line },
				{ "lineadj", token_id::primitive_lineadj },
				{ "triangle", token_id::primitive_triangle },
				{ "triangleadj", token_id::primitive_triangleadj },
				
				{ "linear", token_id::linear },
				{ "noperspective", token_id::noperspective },
				{ "nointerpolation", token_id::nointerpolation },
				{ "sample", token_id::sample },
				{ "precise", token_id::precise },
				{ "shared", token_id::shared },
				{ "groupshared", token_id::groupshared },
				{ "clipplanes", token_id::clipplanes },
				{ "maxvertexcount", token_id::maxvertexcount },
				
				{ "pass", token_id::keyword_pass },
				{ "technique", token_id::keyword_technique },
				{ "technique10", token_id::keyword_technique_sm4 },
				{ "technique11", token_id::keyword_technique_sm5 },
				{ "compile", token_id::keyword_compile },
				{ "VertexShader", token_id::keyword_vertex_shader },
				{ "HullShader", token_id::keyword_hull_shader },
				{ "DomainShader", token_id::keyword_domain_shader },
				{ "GeometryShader", token_id::keyword_geometry_shader },
				{ "PixelShader", token_id::keyword_pixel_shader },
				{ "ComputeShader", token_id::keyword_compute_shader },
				
				{ "Compile", token_id::keyword_compile_sm4 },
				{ "SetVertexShader", token_id::keyword_vertex_shader_sm4 },
				{ "SetHullShader", token_id::keyword_hull_shader_sm4 },
				{ "SetDomainShader", token_id::keyword_domain_shader_sm4 },
				{ "SetGeometryShader", token_id::keyword_geometry_shader_sm4 },
				{ "SetPixelShader", token_id::keyword_pixel_shader_sm4 },
				{ "SetComputeShader", token_id::keyword_compute_shader_sm4 },

				{ "vs_2_0", token_id::profile_vs_20 },
				{ "vs_2_a", token_id::profile_vs_2a },
				{ "vs_2_b", token_id::profile_vs_2b },
				{ "vs_2_s_w", token_id::profile_vs_2sw },
				{ "vs_4_0_level_9_0", token_id::profile_vs_40_level_90 },
				{ "vs_4_0_level_9_1", token_id::profile_vs_40_level_91 },
				{ "vs_4_0_level_9_3", token_id::profile_vs_40_level_93 },
				{ "vs_3_0", token_id::profile_vs_30 },
				{ "vs_4_0", token_id::profile_vs_40 },
				{ "vs_4_1", token_id::profile_vs_41 },
				{ "vs_5_0", token_id::profile_vs_50 },
				{ "vs_5_1", token_id::profile_vs_51 },

				{ "gs_4_0", token_id::profile_gs_40 },
				{ "gs_4_1", token_id::profile_gs_41 },
				{ "gs_5_0", token_id::profile_gs_50 },
				{ "gs_5_1", token_id::profile_gs_51 },

				{ "hs_5_0", token_id::profile_hs_50 },
				{ "hs_5_1", token_id::profile_hs_51 },

				{ "ds_5_0", token_id::profile_ds_50 },
				{ "ds_5_1", token_id::profile_ds_51 },

				{ "ps_2_0", token_id::profile_ps_20 },
				{ "ps_2_a", token_id::profile_ps_2a },
				{ "ps_2_b", token_id::profile_ps_2b },
				{ "ps_2_s_w", token_id::profile_ps_2sw },
				{ "ps_4_0_level_9_0", token_id::profile_ps_40_level_90 },
				{ "ps_4_0_level_9_1", token_id::profile_ps_40_level_91 },
				{ "ps_4_0_level_9_3", token_id::profile_ps_40_level_93 },
				{ "ps_3_0", token_id::profile_ps_30 },
				{ "ps_4_0", token_id::profile_ps_40 },
				{ "ps_4_1", token_id::profile_ps_41 },
				{ "ps_5_0", token_id::profile_ps_50 },
				{ "ps_5_1", token_id::profile_ps_51 },

				{ "cs_4_0", token_id::profile_cs_40 },
				{ "cs_4_1", token_id::profile_cs_41 },
				{ "cs_5_0", token_id::profile_cs_50 },
				{ "cs_5_1", token_id::profile_cs_51 },

				{ "lib_4_0_level_9_0", token_id::profile_lib_40_level_90 },
				{ "lib_4_0_level_9_1", token_id::profile_lib_40_level_91 },
				{ "lib_4_0_level_9_3", token_id::profile_lib_40_level_93 },
				{ "lib_4_0", token_id::profile_lib_40 },
				{ "lib_4_1", token_id::profile_lib_41 },
				{ "lib_5_0", token_id::profile_lib_50 },
				{ "lib_5_1", token_id::profile_lib_51 },

				{ "texture", token_id::type_texture },
				{ "Texture1D", token_id::type_texture_1d },
				{ "Texture1DArray", token_id::type_texture_1d_array },
				{ "texture", token_id::type_texture_2d },
				{ "Texture2D", token_id::type_texture_2d },
				{ "Texture2DArray", token_id::type_texture_2d_array },
				{ "Texture3D", token_id::type_texture_3d },
				{ "TextureCube", token_id::type_texture_cube },
				
				{ "sampler", token_id::keyword_sampler },
				{ "sampler1D", token_id::type_sampler_1d },
				{ "sampler2D", token_id::type_sampler_2d },
				{ "sampler3D", token_id::type_sampler_3d },
				{ "samplerCUBE", token_id::type_sampler_cube },
				{ "sampler_state", token_id::type_sampler_state },
				{ "SamplerState", token_id::type_sampler_state_sm4 },
				{ "SamplerComparisonState", token_id::type_sampler_comparison_state },
				{ "ComparisonFunc", token_id::type_sampler_comparison_state_comparison_func },

				{ "vector", token_id::type_vector },
				{ "matrix", token_id::type_matrix },
				{ "buffer", token_id::type_buffer },

				{ "bool", token_id::type_bool },
				{ "int", token_id::type_integer },
				{ "uint", token_id::type_unsigned_integer },
				{ "dword", token_id::type_dword },
				{ "half", token_id::type_half },
				{ "float", token_id::type_float },
				{ "double", token_id::type_double },

				{ "/*", token_id::block_comment_begin }, // TODO: Move to basic per-character processor?
				{ "*/", token_id::block_comment_end },
				{ "//", token_id::line_comment },

				{ "#line", token_id::preprocessor_line },

			});
		}

		std::vector<token> operator() ( string_view source ) {
			std::vector<token> tokens;
			tokens.emplace_back(token_id::stream_begin, occurrence(), null);
			(*this)(source, tokens);
			tokens.emplace_back(token_id::stream_end, occurrence(), null);
			return tokens;
		}

		template <typename Container>
		void operator() (string_view source, Container& tokens) {
			namespace Unicode = Furrovine::Unicode;
			// Get the beginning and end
			auto begin = source.cbegin();
			auto end = source.cend();
			auto it = source.cbegin();
			if (begin == end) {
				return;
			}
			code_point peek = *it;
			++it;

			// Set up initial lexer state
			state_routine s = state_routine::normal;
			bool lasthardlinebreak = false;
			bool hardlinebreak = false;
			occurrence consumed_where = { 0, 0, 0 };
			intz lastoffset = 0;
			const intz& offset = consumed_where.offset;

			// Begin the process of walking the input
			for (; it != end; ++it) {
				code_point character = peek;
				peek = *it;
				lasthardlinebreak = hardlinebreak;
				hardlinebreak = Unicode::is_line_terminator(character);
				if (hardlinebreak) {
					++consumed_where.line;
					consumed_where.column = 0;
				}
				else {
					++consumed_where.column;
				}

				// Three potential states: 
				// regular, preprocessing, and comments
				switch (s) {
				case state_routine::normal:
					if (character == '/') {
						s = peek == '*'
							? state_routine::block_comment
							: (character == '/' ? state_routine::line_comment : state_routine::normal);

					}
					if (character == '#') {
						s = state_routine::preprocessor;
						break;
					}
					break;
				case state_routine::block_comment:
					if (peek == '/' && character == '*') {
						s = state_routine::normal;
					}
					break;
				case state_routine::line_comment:
					if (hardlinebreak) {
						s = state_routine::normal;
					}
					break;
				default:
					// Shit is wonky; std::terminate or something
					break;
				}
			}
		}
	};

}}
