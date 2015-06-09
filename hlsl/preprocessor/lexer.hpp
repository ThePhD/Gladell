#pragma once

#include "../lexer_head.hpp"
#include "../token.hpp"
#include "../lexer_error.hpp"
#include "../../optional.hpp"
#include "../../string.hpp"
#include "../../unicode.hpp"
#include "../../lexical_numeric_format.hpp"
#include "../../lexical_character_format.hpp"
#include <Furrovine++/scoped_destructor.hpp>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace gld { namespace hlsl { namespace preprocessor {

	class lexer {
	private:
		typedef string_view view_type;
		typedef decltype(std::declval<view_type>().cbegin()) iterator;
		typedef lexer_head<iterator> read_head;
		string_view source;
		
		iterator begin;
		iterator end;
		
		read_head consumed;
		read_head peeked;
		
		std::map<string_view, token_id> pragma;
		std::unordered_map<string_view, token_id> keywords;
		std::unordered_set<code_point> symbolcharacters;
		std::vector<token> tokens;

		bool inmacro, escaped;
		intz escapecount;

	public:
		lexer(string_view source) 
		: source(source), begin(source.cbegin()), end(source.cend()), 
			consumed( source.cbegin() ),
			peeked( source.cbegin() ),
			inmacro( false ),
			escaped( false ),
			escapecount( 0 ) {
			
			symbolcharacters.insert( {
				{ '#' },
				{ '@' },
				{ '>' },
				{ '<' },
				{ '=' },
				{ '&' },
				{ '|' },
				{ '^' },
				{ '(' },
				{ ')' },
				{ '[' },
				{ ']' },
				{ ',' },
				{ '+' },
				{ '*' },
				{ '%' },
				{ '/' },
				{ '-' },
				{ '\\' },
				{ '"' },
				{ '\'' },
				{ ';' },
				{ ',' },
				{ '.' },
				{ '!' },
				{ '}' },
				{ '{' }
			} );

			keywords.insert( {
				{ "define", token_id::preprocessor_define },
				{ "undef", token_id::preprocessor_undef },
				{ "if", token_id::preprocessor_if },
				{ "elif", token_id::preprocessor_elif },
				{ "ifdef", token_id::preprocessor_ifdef },
				{ "ifndef", token_id::preprocessor_ifndef },
				{ "else", token_id::preprocessor_else },
				{ "endif", token_id::preprocessor_endif },
				{ "include", token_id::preprocessor_include },
				{ "line", token_id::preprocessor_line },
				{ "pragma", token_id::preprocessor_pragma },
				{ "error", token_id::preprocessor_error },
				{ "warning", token_id::preprocessor_warning },
				{ "packoffset", token_id::keyword_packoffset },
				{ "row_major", token_id::keyword_row_major },
				{ "column_major", token_id::keyword_column_major },
			});

			pragma.insert({
				{ "once", token_id::preprocessor_pragma_once },
				{ "def", token_id::preprocessor_pragma_register_define },
				{ "pack_matrix", token_id::preprocessor_pragma_pack_matrix },
				{ "warning", token_id::preprocessor_pragma_warning },
				{ "message", token_id::preprocessor_pragma_message },
			});
		}

		bool is_symbol( code_point u ) const {
			return symbolcharacters.find( u ) != symbolcharacters.end();
		}

		std::vector<token> operator()() {
			tokens.emplace_back( token_id::stream_begin, consumed.where );
			lex();
			tokens.emplace_back( token_id::stream_end, consumed.where );
			return std::move( tokens );
		}

		void update( read_head& r ) const {
			r.after_available = r.available = r.at != end;
			if ( !r.available ) {
				r.c = static_cast<code_point>(-1);
				r.after_c = static_cast<code_point>(-1);
				r.previous_line_whitespace = r.line_whitespace;
				r.line_whitespace = false;
				r.white_space = false;
				r.line_terminator = false;
				r.compound_line_terminator = false;
				return;
			}
			r.after_at = r.at;
			++r.after_at;
			r.after_available = r.after_at != end;
			if ( r.after_available ) {
				r.after_c = *r.after_at;
			}
			r.where.offset = std::distance( begin.base(), r.at.base() );
			r.where.offset_after = std::distance( begin.base(), r.after_at.base() );
			r.c = *r.at;
			r.line_terminator = Unicode::is_line_terminator( r.c );
			r.white_space = r.line_terminator || Unicode::is_white_space( r.c );
			r.previous_line_whitespace = r.line_whitespace;
			if ( !r.white_space ) {
				r.line_whitespace = false;
				r.compound_line_terminator = false;
			}
			else if ( r.line_terminator ) {
				r.compound_line_terminator = r.c == '\r' &&
					r.after_available && r.after_c == '\n';
				r.line_whitespace = true;
			}
		}

		bool advance( read_head& r ) const {
			if ( !r.available ) {
				return false;
			}
			if ( r.line_terminator ) {
				if ( !r.compound_line_terminator ) {
					++r.where.line;
					++r.where.processed_line;
					r.where.column = 0;
				}
			}
			++r.at;
			++r.where.column;
			update( r );
			return true;
		}

		template <typename Predicate>
		void advance_if( read_head& r, Predicate&& predicate ) {
			while ( predicate( r ) ) {
				advance( r );
			}
		}

		template <typename Fx>
		void sync( read_head& from, read_head& to, Fx&& incr, intz jumps ) {
			to = from;
			while ( jumps-- > 0 ) {
				incr();
			}
		}

		bool consume( ) {
			return advance( consumed );
		}

		bool peek(  ) {
			return advance( peeked );
		}

		void sync_consumed( read_head& from, intz jumps = 0 ) {
			sync( from, consumed, [this]() { consume(); }, jumps );
		}

		void sync_peeked( read_head& from, intz jumps = 0 ) {
			sync( from, peeked, [this]() { peek(); }, jumps );
		}

		bool consume_newlines( bool addtokens = true ) {
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			bool foundnewline = false;
			for ( ; consumed.available && consumed.line_terminator; consume() ) {
				if ( consumed.compound_line_terminator ) {
					continue;
				}
				foundnewline = true;
			}
			if ( addtokens && foundnewline )
				tokens.emplace_back( token_id::newlines, beginwhere, source.subview( beginat, consumed.at ) );
			return foundnewline;
		}

		bool consume_whitespace() {
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			bool foundwhitespace = false;
			while ( consumed.available && consumed.white_space ) {
				if ( consumed.line_terminator ) {
					foundwhitespace |= consume_newlines();
					beginat = consumed.at;
				}
				else {
					consume();
				}
			}
			if ( beginat != consumed.at ) {
				tokens.emplace_back( token_id::whitespace, beginwhere, source.subview( beginat, consumed.at ) );
				foundwhitespace |= true;
			}
			return foundwhitespace;
		}

		bool consume_whitespace_notnewline() {
			auto beginat = consumed.at;
			while ( consumed.available && consumed.white_space 
				&& !consumed.line_terminator ) {
				consume();
			}
			if ( beginat != consumed.at ) {
				tokens.emplace_back( token_id::whitespace, consumed.where, source.subview( beginat, consumed.at ) );
				return true;
			}
			return false;
		}

		string_view read_non_whitespace( read_head& target ) {
			auto beginat = target.at;
			advance_if( target, []( read_head& r ) {
				return r.available && !Unicode::is_white_space( r.c );
			} );
			return source.subview( beginat, target.at );
		}

		string_view peek_non_whitespace() {
			return read_non_whitespace( peeked );
		}

		string_view consume_non_whitespace() {
			return read_non_whitespace( consumed );
		}

		string_view read_identifier( read_head& target )  {
			auto beginat = target.at;
			if ( !target.available || !Unicode::ucd::is_id_start( target.c ) ) {
				return source.subview( beginat, target.at );
			}
			advance_if( target, []( read_head& r ) {
				return r.available && Unicode::ucd::is_id_continue( r.c );
			} );
			return source.subview( beginat, target.at );
		}

		void consume_comment( token_id commentstyle, occurrence startwhere, string_view start ) {
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			switch ( commentstyle ) {
			case token_id::line_comment_begin:
				tokens.emplace_back( token_id::line_comment_begin, startwhere, start );
				while ( !Unicode::is_line_terminator( consumed.c ) ) {
					consume();
				}
				tokens.emplace_back( token_id::comment_text, beginwhere, source.subview( beginat, consumed.at ) );
				tokens.emplace_back( token_id::line_comment_end, consumed.where, source.subview( consumed.at, consumed.at ) );
				break;
			case token_id::block_comment_begin:
				tokens.emplace_back( token_id::block_comment_begin, startwhere, start );
				sync_peeked( consumed, 1 );
				while ( !(peeked.c == '/' && consumed.c == '*') ) {
					if ( !peeked.available || !consumed.available ) {
						// if we reach the end of input, this is a lexing error?
						// e.g., no end of comment was found
						// can you start a comment in one file, #include that file elsewhere,
						// and then end the comment after the #include ? Must look into...
						// No, you can't, because the #include for a block comment would
						// be considered a comment, not a preprocessing directive
						// TODO: actually write a proper error here...
						throw lexer_error();
					}
					if ( Unicode::is_line_terminator( consumed.c ) ) {
						consume_newlines( false );
						// Re-align peek and consumed heads
						sync_peeked( consumed, 1 );
					}
					consume();
					peek();
				}
				tokens.emplace_back( token_id::comment_text, beginwhere, source.subview( beginat, consumed.at ) );
				tokens.emplace_back( token_id::block_comment_end, consumed.where, source.subview( consumed.at, peeked.after_at ) );
				break;
			default:
				// TODO: proper error
				// Bad comment token id
				throw lexer_error();
				break;
			}
		}

		bool consume_comment() {
			auto beginwhere = consumed.where;
			if ( consumed.c != '/' ) {
				return false;
			}
			sync_peeked( consumed, 1 );
			if ( peeked.c == '/' ) {
				auto start = source.subview( consumed.at, peeked.after_at );
				sync_consumed( peeked );
				consume_comment( token_id::line_comment_begin, beginwhere, start );
			}
			else if ( peeked.c == '*' ) {
				auto start = source.subview( consumed.at, peeked.after_at );
				sync_consumed( peeked );
				consume_comment( token_id::block_comment_begin, beginwhere, start );
			}
			else {
				return false;
			}
			return true;
		}

		void activate_macro() {
			tokens.emplace_back( token_id::preprocessor_statement_begin, consumed.where, view_type() );
			inmacro = true;
		}

		void deactivate_macro() {
			tokens.emplace_back( token_id::preprocessor_statement_end, consumed.where, view_type() );
			inmacro = false;
		}

		void consume_pragma() {
			consume_whitespace_notnewline();
			auto beginwhere = consumed.where;
			string_view keyword = consume_non_whitespace();
			auto pragmafind = pragma.find( keyword );
			token_id tokenid = token_id::preprocessor_pragma_custom;
			if ( pragmafind == pragma.end() ) {
				// TODO: are unknown pragmas supposed to be ignored?
				// I think they are. So technically, this should be a lex_warning,
				// but you can't throw warnings, so we need
				// warning and error report mechanics...
				// TODO: proper warning of
				// "pragma contains unknown directive"
				// throw lexer_error();
			}
			else {
				tokenid = pragmafind->second;
			}
			activate_macro();
			tokens.emplace_back( tokenid, beginwhere, keyword );
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
				// TODO: change out for better consumer for #include directives
				activate_macro();
				// If it's not a quotes-based string...
				if ( !consume_string( '"', '"' ) ) {
					// Then it's a bracket based one. Maybe.
					consume_string( '<', '>' );
				}
				break;
			case token_id::preprocessor_ifdef:
			case token_id::preprocessor_ifndef:
			case token_id::preprocessor_if:
				tokens.emplace_back( token_id::preprocessor_block_begin, consumed.where, view_type() );
				activate_macro();
				break;
			case token_id::preprocessor_endif:
				tokens.emplace_back( token_id::preprocessor_block_end, consumed.where, view_type() );
				activate_macro();
				break;
			case token_id::preprocessor_error:
			case token_id::preprocessor_warning:
			case token_id::preprocessor_define:
			case token_id::preprocessor_elif:
			case token_id::preprocessor_else:
			case token_id::preprocessor_undef:
				activate_macro();
			default:
				break;
			}
		}

		void consume_preprocessor() {
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			if ( consumed.c != '#' )
				return;
			consume();
			tokens.emplace_back( token_id::hash, beginwhere, source.subview( beginat, consumed.at ) );
			if ( !consumed.available ) {
				// TODO: throw lex error or
				// let parser catch Unexpected Stream End?
				return;
			}
			consume_whitespace_notnewline();
			sync_peeked( consumed );
			beginwhere = consumed.where;
			string_view keyword = peek_non_whitespace();
			if ( keyword.empty() ) {
				// TODO: throw lex error or
				// let parser catch Unexpected Stream End?
				return;
			}
			sync_consumed( peeked );
			auto keywordsfind = keywords.find( keyword );
			if ( keywordsfind == keywords.end() ) {
				// TODO: proper lex error
				// Bad keyword for preprocessor (only pragmas can handle unknown preprocessors, right?)
				throw lexer_error();
			}
			tokens.emplace_back( keywordsfind->second, beginwhere, keyword );
			consume_macro( keywordsfind->second );
		}

		void consume_numeric() {
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			bool floating = false;
			bool suffix = false;
			bool invalid = false;
			bool negative = false;
			bool negativeexponent = false;
			bool exponentiation = false;
			lexical_numeric_format format = lexical_numeric_format::decimal;

			if ( consumed.available && consumed.c == '-' ) {
				negative = true;
				consume();
			}
			if ( consumed.available && consumed.c == '0' ) {
				consume();
				if ( consumed.c == 'x' ) {
					format = lexical_numeric_format::hex;
					consume();
				}
				else if ( consumed.c == 'b' ) {
					format = lexical_numeric_format::binary;
					consume();
				}
				else {
					format = lexical_numeric_format::octal;
				}

			}
			switch ( format ) {
			case lexical_numeric_format::hex:
				for ( ; consumed.available && !consumed.white_space; consume() ) {
					switch ( consumed.c ) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case 'a':
					case 'A':
					case 'b':
					case 'B':
					case 'c':
					case 'C':
					case 'e':
					case 'E':
					case 'd':
					case 'D':
					case 'f':
					case 'F':
						continue;
					default:
						invalid = true;
						break;
					}
					break;
				}
				break;
			case lexical_numeric_format::binary:
				for ( ; consumed.available && !consumed.white_space; consume() ) {
					switch ( consumed.c ) {
					case '0':
					case '1':
						continue;
					default:
						invalid = true;
						break;
					}
					break;
				}
				break;
			case lexical_numeric_format::octal:
				for ( ; consumed.available && !consumed.white_space; consume() ) {
					switch ( consumed.c ) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						continue;
					default:
						invalid = true;
						break;
					}
					break;
				}
				break;
			case lexical_numeric_format::decimal:
				for ( ; consumed.available && !consumed.white_space; consume() ) {
					switch ( consumed.c ) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						continue;
					case '.':
					case 'e':
					case 'E':
						break;
					default:
						invalid = true;
						continue;
					}
					break;
				}
				if ( consumed.c == '.' ) {
					consume();
					for ( ; consumed.available && !consumed.white_space; consume() ) {
						switch ( consumed.c ) {
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							continue;
						case 'e':
						case 'E':
							break;
						default:
							invalid = true;
							continue;
						}
						break;
					}
				}
				if ( consumed.c == 'e' || consumed.c == 'E' ) {
					consume();
					exponentiation = true;
					if ( consumed.available && consumed.c == '-' ) {
						negativeexponent = true;
						consume();
					}
					for ( ; consumed.available && !consumed.white_space; consume() ) {
						switch ( consumed.c ) {
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							continue;
						default:
							invalid = true;
							continue;
						}
						break;
					}
				}
				break;
			}
			// TODO: properly serialize a value
			// into the token for use with constant expression folding and the like
			// TODO: generic storage mechanism inside "token" ? unique_ptr the token
			// or do we create a Small Buffer (the size of a pointer) and any struct that's
			// larger than that pointer is stored as a pointer, while rest of by-value...?
			auto numeric = source.subview( beginat, consumed.at );
			tokens.emplace_back( floating ? token_id::float_literal : token_id::integer_literal, beginwhere, numeric );
		}

		void consume_char() {
			if ( consumed.c != '\'' ) {
				return;
			}
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			lexical_character_format format = lexical_character_format::normal;
			bool checkoctal = false;
			bool invalid = false;
			consume();
			tokens.emplace_back( token_id::character_literal_begin, beginwhere, source.subview( beginat, consumed.at ) );
			beginat = consumed.at;
			beginwhere = consumed.where;

			for ( bool charescaped = false; consumed.available && ( charescaped || consumed.c != '\'' ); consume() ) {
				if ( charescaped ) {
					if ( consumed.c == 'x' ) {
						format = lexical_character_format::hex;
					}
					else {
						checkoctal = true;
					}
				}
				else if ( consumed.c == '\\' ) {
					charescaped = true;
					continue;
				}
				charescaped = false;
			}

			if ( !consumed.available )
				return;

			auto characterlexeme = source.subview( beginat, consumed.at );
			auto characterlexemewhere = beginwhere;
			
			if ( checkoctal ) {
				auto target = characterlexeme.begin();
				// TODO:
				// Perhaps a more-clear expression might be
				// in order...
				format = characterlexeme.size() == 4 
					&& Unicode::is_numeric( *++target )
					&& Unicode::is_numeric( *++target ) 
					&& Unicode::is_numeric( *++target ) ?
					lexical_character_format::octal 
					: format;
			}
			else if ( format == lexical_character_format::hex ) {
				auto characterlexemeend = characterlexeme.end();
				intz tickcount = 0;
				for ( auto it = ++characterlexeme.begin(); it != characterlexemeend; ++it ) {
					if ( tickcount == 2 ) {
						invalid = true;
						break;
					}
					code_point cp = *it;
					switch ( cp ) {
					case '0': case '1': case '2':
					case '3': case '4': case '5':
					case '6': case '7': case '8':
					case '9': case 'A': case 'B':
					case 'C': case 'D': case 'E':
					case 'F': case 'a': case 'b':
					case 'c': case 'd': case 'e':
					case 'f':
						continue;
					case '\'':
						++tickcount;
						if ( tickcount < 2 ) {
							continue;
						}
						invalid = true;
						break;
					default:
						// Bad character format
						invalid = true;
						break;
					}
					break;
				}
			}

			beginat = consumed.at;
			beginwhere = consumed.where;
			Furrovine::make_destructor(
				[&]() {
					consume();
					tokens.emplace_back( token_id::character_literal_end, beginwhere, source.subview( beginat, consumed.at ) );
				}
			);

			if ( invalid ) {
				tokens.emplace_back( token_id::character_literal, characterlexemewhere, characterlexeme );
				return;
			}
			
			// TODO: need to handle character literals
			// and serialize / save that information with the token
			code_point value = 0;
			intz shift = 0;
			switch ( format ) {
			case lexical_character_format::octal: {
				const char* start = characterlexeme.data();
				const char* desiredend = characterlexeme.data_end() - 1;
				char* end = nullptr;
				++start;
				value = std::strtoul( start, &end, 8 );
				if ( end != desiredend ) {
					invalid = true;
				}
				break; }
			case lexical_character_format::hex: {
				const char* start = characterlexeme.data();
				const char* desiredend = characterlexeme.data_end() - 1;
				char* end = nullptr;
				++++start;
				value = std::strtoul( start, &end, 16 );
				if ( end != desiredend ) {
					invalid = true;
				}
				break; }
			case lexical_character_format::normal:
			default: {
				// TODO: properly handle multi-character literals?
				// TODO: unicode character literals?
				auto it = ++characterlexeme.begin();
				auto characterlexemeend = --characterlexeme.end();
				for ( ; it != characterlexemeend; ++it, shift += 8 ) {
					value = value |= (uint8)(*++characterlexeme.begin() << shift);
				}
				break; }
			}
			tokens.emplace_back( token_id::character_literal, beginwhere, characterlexeme, value );
		}

		bool consume_raw_string() {
			// TODO: implement by allowing consume_string to take
			// string-based delimeters,
			// and then determing the first delimeter by reading everything
			// from the R"[delimeter(][character|unescaped-character][)delimeter-from-before]"
			/*string*/code_point startdelimeter = '"';
			/*string*/code_point enddelimeter = '"';
			return consume_string( startdelimeter, enddelimeter, false );
		}

		// TODO: handle raw string literals (even if HLSL / GLSL don't have them, useful to handle
		// strict superset of the language and warn after AST to target format?)
		// TODO: take a full string_view as the delimeters, to allow for R"( begins...
		bool consume_string( code_point startdelimeter = '"', code_point enddelimeter = '"', bool respectnewlines = true ) {
			if ( consumed.c != startdelimeter ) {
				return false;
			}
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			consume();
			tokens.emplace_back( token_id::string_literal_begin, beginwhere, source.subview( beginat, consumed.at ) );
			beginat = consumed.at;
			for ( bool stringescaped = false; stringescaped ? true : consumed.c != enddelimeter; consume() ) {
				if ( !consumed.available ) {
					// Broken string literal, how to indicate it's a bad value that's consumed all input?
					// tokens.emplace_back( token_id::string_literal, beginwhere, source.subview( beginat, consumed.at ) );
					// introduce string literal markers: 
					// we just check for if there's a matching string_literal_end for every string_literal_begin
					return false;
				}
				if ( respectnewlines && consumed.line_terminator ) {
					// Broken string literal:
					// did not end before newline
					return false;
				}
				if ( consumed.c == '\\' ) {
					stringescaped = true;
					continue;
				}
				stringescaped = false;
			}
			tokens.emplace_back( token_id::string_literal, beginwhere, source.subview( beginat, consumed.at ) );
			beginat = consumed.at;
			consume();
			tokens.emplace_back( token_id::string_literal_end, beginwhere, source.subview( beginat, consumed.at ) );
			return true;
		}

		void consume_identifier() {
			auto beginat = consumed.at;
			auto beginwhere = consumed.where;
			sync_peeked( consumed );
			string_view identifier = read_identifier( peeked );
			if ( !identifier.empty() ) {
				sync_consumed( peeked );
				auto keywordsfind = keywords.find( identifier );
				if ( keywordsfind != keywords.end() ) {
					tokens.emplace_back( keywordsfind->second, beginwhere, identifier );
					return;
				}
				tokens.emplace_back( token_id::identifier, beginwhere, identifier );
				return;
			}
			consume();
			tokens.emplace_back( token_id::identifier, beginwhere, source.subview( beginat, consumed.at ) );
		}

		void lex ( ) {
			update( consumed );
			if ( !consumed.available ) {
				return;
			}
			
			// Now we're ready to roll
			for ( ; consumed.available; ) {
				if ( inmacro ) {
					consume_whitespace_notnewline();
					if ( consumed.line_terminator ) {
						if ( consumed.compound_line_terminator ) {
							continue;
						}
						if ( escaped ) {
							auto escapedat = consumed.at;
							auto escapedwhere = consumed.where;
							consume();
							tokens.emplace_back( token_id::preprocessor_escaped_newline, escapedwhere, source.subview( escapedat, consumed.at ) );
							escaped = false;
							continue;
						}
						deactivate_macro();
						continue;
					}
				}
				else {
					consume_whitespace();
				}

				auto beginat = consumed.at;
				auto beginwhere = consumed.where;
				switch ( consumed.c ) {
				case '\\':
					consume();
					if ( inmacro ) {
						tokens.emplace_back( token_id::escape, beginwhere, source.subview( beginat, consumed.at ) );
						escaped = true;
					}
					else if ( escaped ) {
						tokens.emplace_back( token_id::back_slash, beginwhere, source.subview( beginat, consumed.at ) );
					}
					else {
						tokens.emplace_back( token_id::escape, beginwhere, source.subview( beginat, consumed.at ) );
					}
					continue;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '0':
					consume_numeric();
					break;
				case '>':
					consume();
					if ( consumed.c == '=' ) {
						consume();
						tokens.emplace_back( token_id::greater_than_or_equal_to, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					else if ( peeked.c == '>' ) {
						consume();
						tokens.emplace_back( token_id::right_shift, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::greater_than, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '<':
					consume();
					if ( peeked.c == '=' ) {
						consume();
						tokens.emplace_back( token_id::less_than_or_equal_to, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					else if ( peeked.c == '<' ) {
						consume();
						tokens.emplace_back( token_id::left_shift, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::less_than, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '=':
					consume();
					if ( consumed.available && consumed.c == '=' ) {
						consume();
						tokens.emplace_back( token_id::equal_to, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::assignment, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '&':
					consume();
					if ( consumed.available && consumed.c == '&' ) {
						consume();
						tokens.emplace_back( token_id::expression_and, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::boolean_and, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '|':
					consume();
					if ( consumed.available && consumed.c == '|' ) {
						consume();
						tokens.emplace_back( token_id::expression_or, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::boolean_or, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '^':
					consume();
					tokens.emplace_back( token_id::boolean_xor, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '!':
					consume();
					if ( consumed.available && consumed.c == '=' ) {
						consume();
						tokens.emplace_back( token_id::not_equal_to, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::boolean_not, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '(':
					consume();
					tokens.emplace_back( token_id::open_parenthesis, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case ')':
					consume();
					tokens.emplace_back( token_id::close_parenthesis, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '{':
					consume();
					tokens.emplace_back( token_id::open_curly_bracket, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '}':
					consume();
					tokens.emplace_back( token_id::close_curly_bracket, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '[':
					consume();
					tokens.emplace_back( token_id::open_square_bracket, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case ']':
					consume();
					tokens.emplace_back( token_id::close_square_bracket, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case ',':
					consume();
					tokens.emplace_back( token_id::comma, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case ';':
					consume();
					tokens.emplace_back( token_id::semi_colon, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case ':':
					consume();
					tokens.emplace_back( token_id::colon, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '.':
					consume();
					tokens.emplace_back( token_id::dot, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '%':
					consume();
					tokens.emplace_back( token_id::modulus, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '+':
					consume();
					if ( consumed.c == '+' ) {
						tokens.emplace_back( token_id::increment, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::plus, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '-':
					consume();
					if ( consumed.c == '-' ) {
						tokens.emplace_back( token_id::decrement, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					tokens.emplace_back( token_id::minus, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '*':
					consume();
					tokens.emplace_back( token_id::multiply, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '/':
					if ( consume_comment() ) {
						break;
					}
					consume();
					tokens.emplace_back( token_id::divide, beginwhere, source.subview( beginat, consumed.at ) );
					break;
				case '@':
					consume();
					if ( consumed.available && consumed.c == '#' ) {
						tokens.emplace_back( token_id::preprocessor_charizing, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					consume_identifier();
					break;
				case '#':
					if ( consumed.previous_line_whitespace ) {
						consume_preprocessor();
						break;
					}
					consume();
					if ( consumed.available && consumed.c == '#' ) {
						consume();
						tokens.emplace_back( token_id::preprocessor_token_pasting, beginwhere, source.subview( beginat, consumed.at ) );
						break;
					}
					else {
						tokens.emplace_back( token_id::hash, beginwhere, source.subview( beginat, consumed.at ) );
					}
					break;
				case '\'':
					consume_char();
					break;
				case '"':
					if ( tokens.size() > 0 && tokens.back().lexeme == 'R' ) {
						consume_raw_string();
					}
					else {
						consume_string( '"', '"' );
					}
					break;
				default:
					consume_identifier();
					break;
				}
				escaped = false;
			}
		}
	};

}}}
