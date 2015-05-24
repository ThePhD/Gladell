#pragma once

#include "../token.hpp"
#include <Furrovine++/Unicode/properties.hpp>
#include <map>
#include <unordered_map>

namespace gld { namespace hlsl { namespace preprocessor {

	class lexer {
	private:
		std::map<string, token_id> pragma;
		std::unordered_map<string_view, token_id> keywords;

		struct character_information {
			code_point character;
			occurrence where;
			bool is_line_terminator;
			bool is_whitespace;
			bool line_whitespace;
			bool id_start;
			bool id_continue;
			bool other_id_start;
			bool other_id_continue;
			bool xid_start;
			bool xid_continue;
			string_view chunk;

			void set_character(code_point c) {
				namespace Unicode = Furrovine::Unicode;
				character = c;
				is_line_terminator = Unicode::is_line_terminator(character);
				is_whitespace = Unicode::is_white_space(character);
				line_whitespace &= is_whitespace;
				id_start = Unicode::ucd::is_id_start(character);
				id_continue = Unicode::ucd::is_id_continue(character);
				other_id_start = Unicode::ucd::is_other_id_start(character);
				other_id_continue = Unicode::ucd::is_other_id_continue(character);
				xid_start = Unicode::ucd::is_xid_start(character);
				xid_continue = Unicode::ucd::is_xid_continue(character);
			}
		};

		struct chunk_information {
			character_type type;
		};

	public:
		lexer() {
			keywords.insert({
				{ "#", token_id::hash },
				{ "define", token_id::preprocessor_define },
				{ "undef", token_id::preprocessor_undef },
				{ "if", token_id::preprocessor_if },
				{ "elif", token_id::preprocessor_elif },
				{ "ifdef", token_id::preprocessor_ifdef },
				{ "ifndef", token_id::preprocessor_ifndef },
				{ "elif", token_id::preprocessor_else },
				{ "endif", token_id::preprocessor_endif },
				{ "include", token_id::preprocessor_include },
				{ "line", token_id::preprocessor_line },
				{ "pragma", token_id::preprocessor_pragma },
				{ "error", token_id::preprocessor_error },
			});

			pragma.insert({
				{ "def", token_id::preprocessor_pragma_register_define },
				{ "pack_matrix", token_id::preprocessor_pragma_pack_matrix },
				{ "warning", token_id::preprocessor_pragma_warning },
				{ "message", token_id::preprocessor_pragma_message },
			});
		}

		std::vector<token> operator()(string_view source) {
			std::vector<token> tokens;
			tokens.emplace_back(token_id::stream_begin, occurrence(), null);
			(*this)(source, tokens);
			tokens.emplace_back(token_id::stream_end, occurrence(), null);
			return tokens;
		}

		void operator() ( string_view source, std::vector<token>& tokens ) {
			typedef decltype(source.cbegin()) iterator;
			
			auto begin = source.cbegin();
			auto end = source.cend();
			if (begin == end) {
				return;
			}
			
			std::array<character_information, 2> look = {};
			look.fill({});
			std::array<iterator, 2> lookit = {
				begin,
				begin,
			};
			std::array<bool, 2> lookitvalid = { };
			lookitvalid.fill(true);

			bool break_view = false;

			// Our state setter for the loop: 
			// fills the character information target
			// with basic information
			auto set_state = [&]() {
				auto& targetit = lookit.back();
				auto targetitafter = targetit;
				bool& targetvalid = lookitvalid.back();
				character_information& target = look.back();
				targetvalid = targetit != end;
				target.set_character(targetvalid ? *targetit : 0);
				intz distance = targetvalid ? std::distance(begin.base(), targetit.base()) : target.where.offset;
				intz distanceafter = targetvalid ? std::distance(begin.base(), (++targetitafter).base()) : target.where.offset_after;
				intz moved = distanceafter - distance;
				target.where.column += moved;
				target.where.offset = distance;
				target.where.offset_after = distanceafter;
				if (target.is_line_terminator) {
					++target.where.line;
					++target.where.true_line;
					target.where.column = 0;
				}
			};
			// Our incrementation function for the loop
			auto increment = [&]( ) {
				for (intuz la = 0, last = look.size() - 1; la < last; ++la) {
					look[la] = look[la + 1];
					lookit[la] = lookit[la + 1];
					lookitvalid[la] = lookitvalid[la + 1];
				}
				auto& targetit = lookit.back();
				++targetit;
				set_state();
			};

			// Now, the actual setup:
			// We first set the character of our first "peek"
			set_state();
			// Then we set up the look ahead for 1, 2, 3, 4...
			// of the array
			for (intuz i = 1; i < look.size(); ++i) {
				increment();
			}

			// Now we're ready to roll
			for ( intz lastbreak = 0; lookitvalid[0]; increment()) {
				auto& targetit = lookit[0];
				bool& targetvalid = lookitvalid[0];
				character_information& target = look[0];
				character_information& peek = look[1];
				if (break_view) {
					lastbreak = target.where.offset;
					break_view = false;
				}
				target.chunk = source.substr(lastbreak, target.where.offset_after);
				peek.chunk = source.substr(lastbreak, peek.where.offset_after);

				if (target.is_line_terminator) {
					if (target.character == '\r' && peek.character == '\n') {
						continue;
					}
					tokens.push_back(token(token_id::newline, target.where, target.chunk));
					break_view = true;
				}
				else if (target.is_whitespace && !peek.is_whitespace) {
					tokens.push_back(token(token_id::whitespace, target.where, target.chunk));
					break_view = true;
				}
				else {
					auto keywordfind = keywords.find(target.chunk);
					if (keywordfind != keywords.end()) {
						tokens.push_back(token(keywordfind->second, target.where, target.chunk));
						break_view = true;
					}
					else if (peek.is_whitespace) {
						// TODO: separate identifiers from:
						// string literals, numeric literals
						tokens.push_back(token(token_id::identifier, target.where, target.chunk));
						break_view = true;
					}
				}
			}
		}
	};

}}}
