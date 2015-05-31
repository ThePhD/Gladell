#pragma once

#include "../token.hpp"
#include "../../optional.hpp"
#include "../../string.hpp"
#include "../../unicode.hpp"
#include <map>
#include <unordered_map>

namespace gld { namespace hlsl { namespace preprocessor {

	class lexer {
	private:
		typedef decltype(std::declval<string_view>().cbegin()) iterator;
		string_view source;
		
		iterator begin;
		iterator end;
		
		iterator consumed;
		code_point c;
		occurrence consumedwhere;
		bool consumedavailable;
		
		iterator peeked;
		code_point p;
		occurrence peekwhere;
		bool peekedavailable;

		
		std::map<string, token_id> pragma;
		std::unordered_map<string_view, token_id> keywords;
		std::vector<token> tokens;

	public:
		lexer(string_view source) 
		: source(source), begin(source.cbegin()), end(source.cbegin()), 
			consumed( source.cbegin() ), consumedavailable( true ),
			peeked( source.cbegin() ), peekedavailable( true ) {
			keywords.insert({
				{ "#", token_id::hash },
				{ "(", token_id::open_parenthesis },
				{ ")", token_id::close_parenthesis },
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

		std::vector<token> operator()() {
			tokens.emplace_back(token_id::stream_begin, consumedwhere, null);
			lex();
			tokens.emplace_back(token_id::stream_end, consumedwhere, null);
			return std::move(tokens);
		}

		bool consumed_empty() const {
			return !consumedavailable;
		}

		bool peeked_empty() const {
			return !peekedavailable;
		}

		bool consume() {
			if ( !consumedavailable ) {
				c = static_cast<code_point>(-1);
				return !consumed_empty();
			}

			++consumed;
			auto saved = consumed;
			++saved;
			consumedwhere.offset = std::distance( begin.base(), consumed.base() );
			consumedwhere.offset_after = std::distance( begin.base(), saved.base() );
			consumedwhere.column += consumedwhere.offset_after - consumedwhere.offset;
			consumedavailable = consumed != end;
			c = *consumed;
			return !consumed_empty();
		}

		bool peek() {
			if ( !peekedavailable ) {
				c = static_cast<code_point>(-1);
				return !peeked_empty();
			}

			++peeked;
			auto saved = peeked;
			++saved;
			peekwhere.offset = std::distance( begin.base(), peeked.base() );
			peekwhere.offset_after = std::distance( begin.base(), saved.base() );
			peekwhere.column += peekwhere.offset_after - peekwhere.offset;
			peekedavailable = peeked != end;
			c = *peeked;
			return !peeked_empty();
		}

		bool consume_newlines() {
			auto beginconsume = consumed;
			bool foundnewline = false;
			while ( !consumed_empty() && Unicode::is_line_terminator( c ) ) {
				bool iscarriagereturn = c == '\r';
				if ( consume() ) {
					if ( iscarriagereturn && c == '\n' ) {
						continue;
					}
				}
				++consumedwhere.line;
				tokens.emplace_back( token_id::newline, consumedwhere, source.subview( beginconsume, consumed ) );
				foundnewline = true;
			}
			return foundnewline;
		}

		bool consume_whitespace() {
			auto beginconsume = consumed;
			bool foundwhitespace = false;
			while ( !consumed_empty() && Unicode::is_white_space( c ) ) {
				if ( Unicode::is_line_terminator( c ) ) {
					foundwhitespace |= consume_newlines();
					beginconsume = consumed;
				}
				else {
					consume();
				}
			}
			if ( beginconsume != consumed ) {
				tokens.emplace_back( token_id::whitespace, consumedwhere, source.subview( beginconsume, consumed ) );
				foundwhitespace |= true;
			}
			return foundwhitespace;
		}

		bool consume_whitespace_notnewline() {
			auto beginconsume = consumed;
			while ( !consumed_empty() && Unicode::is_white_space( c ) 
				&& !Unicode::is_line_terminator( c ) ) {
				consume();
			}
			if ( beginconsume != consumed ) {
				tokens.emplace_back( token_id::whitespace, consumedwhere, source.subview( beginconsume, consumed ) );
				return true;
			}
			return false;
		}

		void consume_toplevel() {
			consume_preprocessor();
			
		}

		void consume_comment() {
			auto beginconsume = consumed;
			if ( c != '/' ) {
				return;
			}
			consume();
			if ( c == '/' ) {
				consume();
				tokens.emplace_back( token_id::line_comment, consumedwhere, source.subview( beginconsume, consumed ) );
				beginconsume = consumed;
				while ( !Unicode::is_line_terminator( c ) ) {
					consume();
				}
				tokens.emplace_back( token_id::comment_text, consumedwhere, source.subview( beginconsume, consumed ) );
			}
			else if ( c == '*' ) {
				consume();
				tokens.emplace_back( token_id::block_comment_begin, consumedwhere, source.subview( beginconsume, consumed ) );
				beginconsume = consumed;
				peeked = consumed;
				peek();
				auto beginwhere = consumedwhere;
				while ( !(p == '/' && c == '*') ) {
					if ( !consumed_empty()
						&& !peeked_empty() ) {
						// TODO: if we reach the end of input, this is a lexing error
						// e.g., no end of comment was found
						// TODO: can you start a comment in one file, #include that file elsewhere,
						// and then end the comment after the #include ? Must look into...
					}
					consume();
					peek();
				}
				// Move the peak read head up by one
				// so we can go from consumed, which is * to 1 past the /
				peek();
				tokens.emplace_back( token_id::comment_text, beginwhere, source.subview( beginconsume, consumed ) );
				tokens.emplace_back( token_id::block_comment_end, consumedwhere, source.subview( consumed, peeked ) );
			}
		}

		void consume_macro_line() {
			while ( !Unicode::is_line_terminator( c ) ) {
				consume_whitespace_notnewline();
				consume_identifier();
				consume_comment();
			}
			if ( c == '\\' ) {
				consume();
				if ( consume_newlines() ) {
					// This macro has a newline 
					// that is escaped goes directly to the next line
					consume_macro_line();
				}
			}
		}

		void consume_pragma() {

		}

		void consume_macro( token_id preprocessorid ) {
			switch ( preprocessorid ) {
			case token_id::preprocessor_pragma:
				consume_pragma();
				break;
			case token_id::preprocessor_include:
				// TODO: should the lexer stream be
				// modified to also preprocess included
				// file streams?
				// TODO: change out for better consumer
				consume_macro_line();
				break;
			case token_id::preprocessor_error:
			case token_id::preprocessor_warning:
			case token_id::preprocessor_define:
			case token_id::preprocessor_if:
			case token_id::preprocessor_elif:
			case token_id::preprocessor_else:
			case token_id::preprocessor_endif:
			case token_id::preprocessor_ifdef:
			case token_id::preprocessor_ifndef:
			case token_id::preprocessor_undef:
			default:
				// These macros can multiline
				consume_macro_line();
				break;
			}
		}

		string_view consume_identifier() {
			auto beginconsume = consumed;
			if ( !Unicode::ucd::is_id_start( c ) ) {
				return source.subview( beginconsume, consumed );
			}
			consume();
			while ( Unicode::ucd::is_id_continue( c ) ) {
				// Read the whole identifier text
				consume();
			}
			return source.subview( beginconsume, consumed );
		}

		void consume_preprocessor() {
			auto beginconsume = consumed;
			if ( c != '#' )
				return;
			consume();
			tokens.emplace_back( token_id::hash, consumedwhere, source.subview( beginconsume, consumed ) );
			if ( consumed_empty() )
				return;
			
			string_view identifier = consume_identifier();
			if ( identifier.empty() )
				return;
			auto keywordsfind = keywords.find( identifier );
			if ( keywordsfind == keywords.end() )
				return;
			tokens.emplace_back( keywordsfind->second, consumedwhere, identifier );
			consume_macro( keywordsfind->second );
		}

		void lex ( ) {
			if ( consumed_empty() ) {
				return;
			}
			c = *consumed;

			// Now we're ready to roll
			for (; !consumed_empty() ;) {
				consume_whitespace();
				consume_toplevel();				
			}
		}
	};

}}}
