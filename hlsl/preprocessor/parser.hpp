#pragma once

#include "construct.hpp"
#include "../token.hpp"
#include "../parser_head.hpp"
#include "../parser_error.hpp"
#include "../../string.hpp"
#include "../../range.hpp"

namespace gld { namespace hlsl { namespace preprocessor {

	class parser {
	private:
		typedef buffer_view<const token> token_view;
		typedef token_view view_type;
		typedef Furrovine::tmp::range_cbegin_type_t<view_type> begin_iterator;
		typedef Furrovine::tmp::range_cend_type_t<view_type> end_iterator;
		typedef begin_iterator iterator;
		typedef parser_head<iterator> read_head;

		view_type source;
		iterator begin;
		end_iterator end;

		read_head consumed;

		top_level toplevel;
		std::reference_wrapper<scope> targetscope;
		
	public:
		parser( view_type tokens ) : source( std::move( tokens ) ),
		begin( adl_cbegin( source ) ), end( adl_cend( source ) ),
		consumed( begin ),
		toplevel( source ), targetscope( toplevel.scopes.back() ) {
			
		}

		void expected_error ( const read_head& r, token_id id ) {
			if ( !r.available ) {
				// unexpected end of stream, expected 'id'
				throw parser_error();
			}
			expected_error( r.t, id );
		}

		void expected_error( const token& t, token_id id ) {
			if ( id == t.id ) {
				// TODO: proper error
				// Expected 'blah', received 'blah'
				throw parser_error();
			}
		}

		void expected_error( const token& t, std::initializer_list<token_id> ids ) {
			if ( std::find( ids.begin(), ids.end(), t.id ) == ids.end() ) {
				// TODO: proper error
				// Expected 'blah', received 'blah'
				throw parser_error();
			}
		}

		bool update( read_head& r ) {
			r.available = r.at == end;
			if ( !r.available ) {
				return false;
			}
			r.t = *r.at;
			r.prevlinewhitespace = r.linewhitespace;
			switch ( r.t.get().id ) {
			case token_id::newlines:
				r.linewhitespace = true;
				break;
			case token_id::comment_text:
			case token_id::block_comment_begin:
			case token_id::block_comment_end:
			case token_id::line_comment_begin:
			case token_id::line_comment_end:
			case token_id::whitespace:
			case token_id::preprocessor_escaped_newline:
				break;
			default:
				r.linewhitespace = false;
				break;
			}
			return true;
		}

		bool advance( read_head& r ) {
			if ( !r.available ) {
				return false;
			}
			++r.at;
			update( r );
			return true;
		}

		bool consume() { 
			return advance( consumed ); 
		}

		bool read_whitespace( read_head& r ) {
			bool found = false;
			for ( ; r.available; found = true ) {
				const token& t = r.t;
				switch ( t.id ) {
				case token_id::whitespace:
					advance( r );
					continue;
				case token_id::newlines:
					advance( r );
					continue;
				case token_id::block_comment_begin:
					advance( r );
					if ( !r.available || t.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					advance( r );
					if ( !r.available || t.id != token_id::block_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						throw parser_error();
					}
					continue;
				case token_id::line_comment_begin:
					advance( r );
					if ( !r.available || t.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					advance( r );
					if ( !r.available || t.id != token_id::line_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						throw parser_error();
					}
					continue;
				default:
					break;
				}
				break;
			}
			return found;
		}

		bool consume_whitespace() {
			bool found = false;
			for ( ; consumed.available; found = true ) {
				const token& t = consumed.t;
				switch ( t.id ) {
				case token_id::whitespace:
					consume();
					continue;
				case token_id::newlines:
					consume();
					continue;
				case token_id::block_comment_begin:
					consume();
					if ( !consumed.available || t.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					consume();
					if ( !consumed.available || t.id != token_id::block_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						throw parser_error();
					}
					continue;
				case token_id::line_comment_begin:
					consume();
					if ( !consumed.available || t.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					consume();
					if ( !consumed.available || t.id != token_id::line_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						throw parser_error();
					}
					continue;
				default:
					break;
				}
				break;
			}
			return found;
		}

		bool consume_preprocessor_define_variable() {
			const token& begintoken = consumed.t;
			auto beginlexeme = begintoken.lexeme;
			auto beginat = consumed.at;
			consume();
			auto symbolfind = toplevel.symbols.find( beginlexeme );
			if ( symbolfind == toplevel.symbols.end() ) {
				symbolfind = toplevel.symbols.emplace_hint( symbolfind, begintoken.lexeme, definition( token_view( beginat, consumed.at ), begintoken, unit{} ) );
			}
			else {
				// Error/warning: redeclaring an already-defined symbol
				// TODO: warning ONLY if the actual defined symbol is different
				throw parser_error();
			}
			return false;
		}

		bool consume_preprocessor_define_function() {
			consume_whitespace();
			expected_error( consumed.t, token_id::open_parenthesis );
			consume();
			
			bool foundargument = false;
			optional<const token&> variablearguments = none;
			for ( ; consumed.available; ) {
				consume_whitespace();
				const token& t = consumed.t;
				switch ( t.id ) {
				case token_id::close_parenthesis:
					break;
				case token_id::dot_dot_dot:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						throw parser_error();
					}
					variablearguments = t;
					foundargument = true;
					break;
				case token_id::identifier:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						// And must end on that statement
						throw parser_error();
					}
					foundargument = true;
					break;
				default:
					// TODO: proper error
					// Unexpected token, expected ')' or '...' or Identifier
					throw parser_error();
				}
				break;
			}



			return true;
		}

		bool consume_preprocessor_define() {
			expected_error( consumed, token_id::identifier );
			auto peeked = consumed;
			consume_whitespace();
			advance( peeked );
			
			
			return true;
		}

		bool consume_preprocessor_undef() {
			const token& t = consumed.t;
			expected_error( t, token_id::identifier );
			std::size_t count = toplevel.symbols.erase( t.lexeme );
			if ( count == 0 ) {
				// TODO: warning that 'undef'ing a symbol that doesn't exist?
			}
			else if ( count > 1 ) {
				// Should never hit this condition ever
				// TODO: proper error
				// multiply defined symbols being removed
				throw parser_error();
			}
			return true;
		}

		bool consume_preprocessor_conditional_begin_scope( scope& enclosing ) {
			auto conditionread = consumed;
			for ( ; conditionread.available; advance( conditionread ) ) {
				const token& t = conditionread.t;
				if ( t.id == token_id::preprocessor_statement_end ) {
					break;
				}
			}
			expected_error( conditionread.t, token_id::preprocessor_statement_end );
			advance( conditionread );
			
			toplevel.scopes.emplace_back( token_view( conditionread.at, conditionread.at ) );
			advance( conditionread );

			scope& fresh = toplevel.scopes.back();
			toplevel.scopestack.push( fresh );
			targetscope = toplevel.scopestack.top();
			
			toplevel.expressions.push_back( std::make_unique<conditional>( token_view( consumed.at, conditionread.at ) ) );
			
			expression* resultexpression = toplevel.expressions.back().get();
			conditional& condition = *static_cast<conditional*>( resultexpression );
			fresh.condition = condition;
			
			return true;
		}

		bool consume_preprocessor_conditional_end_scope( scope& enclosing ) {
			auto conditionread = consumed;
			for ( ; conditionread.available; advance( conditionread ) ) {
				const token& t = conditionread.t;
				if ( t.id == token_id::preprocessor_block_end ) {
					break;
				}
			}
			expected_error( conditionread.t, token_id::preprocessor_block_end );
			advance( conditionread );
			enclosing.tokens = token_view( enclosing.tokens.cbegin(), conditionread.at );

			toplevel.scopestack.pop();

			return true;
		}

		bool consume_preprocessor_if() {
			return consume_preprocessor_conditional_begin_scope( targetscope );
		}

		bool consume_preprocessor_ifdef() {
			return consume_preprocessor_conditional_begin_scope( targetscope );
		}

		bool consume_preprocessor_ifndef() {
			return consume_preprocessor_conditional_begin_scope( targetscope );
		}

		bool consume_preprocessor_elif() {
			return consume_preprocessor_conditional_end_scope( targetscope )
				&& consume_preprocessor_conditional_begin_scope( targetscope );
		}

		bool consume_preprocessor_else() {
			return consume_preprocessor_conditional_end_scope( targetscope )
				&& consume_preprocessor_conditional_begin_scope( targetscope );
		}

		bool consume_preprocessor_endif() {
			return consume_preprocessor_conditional_end_scope( targetscope );
		}

		bool consume_preprocessor_line() {
			auto conditionread = consumed;
			for ( ; conditionread.available; advance( conditionread ) ) {
				const token& t = conditionread.t;
				if ( t.id == token_id::preprocessor_statement_end ) {
					break;
				}
			}
			consumed = conditionread;
			return true;
		}

		bool consume_preprocessor_include() {
			auto conditionread = consumed;
			optional<const token&> include;
			for ( ; conditionread.available; advance( conditionread ) ) {
				const token& t = conditionread.t;
				switch ( t.id ) {
				case token_id::string_literal_begin:
					if ( include ) {
						// TODO: proper error
						// two string literals on the same include line: syntax error
						throw parser_error();
					}
					advance( conditionread );
					expected_error( conditionread, token_id::string_literal );
					include = t;
					advance( conditionread );
					expected_error( conditionread, token_id::string_literal_end );
					continue;
				case token_id::preprocessor_statement_end:
					break;
				}
				break;
			}
			advance( conditionread );
			toplevel.expressions.push_back( std::make_unique<inclusion>( token_view( consumed.at, conditionread.at ), include.get() ) );
			
			consumed = conditionread;
			return true;
		}

		bool consume_preprocessor_pragma() {
			auto conditionread = consumed;
			for ( ; conditionread.available; advance( conditionread ) ) {
				const token& t = conditionread.t;
				if ( t.id == token_id::preprocessor_statement_end ) {
					break;
				}
			}
			consumed = conditionread;
			
			return true;
		}

		bool consume_preprocessor() {
			const token& t = consumed.t;
			if ( t.id != token_id::preprocessor_hash ) {
				return false;
			}
			consume();
			consume_whitespace();
			switch ( t.id ) {
			case token_id::preprocessor_undef:
				return consume_preprocessor_undef();
			case token_id::preprocessor_define:
				return consume_preprocessor_define();
			case token_id::preprocessor_if:
				return consume_preprocessor_if();
			case token_id::preprocessor_ifdef:
				return consume_preprocessor_ifdef();
			case token_id::preprocessor_ifndef:
				return consume_preprocessor_ifndef();
			case token_id::preprocessor_elif:
				return consume_preprocessor_elif();
			case token_id::preprocessor_else:
				return consume_preprocessor_else();
			case token_id::preprocessor_endif:
				return consume_preprocessor_endif();
			case token_id::preprocessor_include:
				return consume_preprocessor_include();
			case token_id::preprocessor_line:
				return consume_preprocessor_line();
			case token_id::preprocessor_pragma:
				return consume_preprocessor_pragma();
			default:
				return false;
			}
			return true;
		}

		bool consume_line() {
			// TODO: can this... ever really fail?
			for ( ; consumed.available; consume() ) {
				switch ( consumed.t.get().id ) {
				case token_id::newlines:
					consume();
					return true;
				case token_id::identifier: {
					scope& s = targetscope;
					break; }
				case token_id::stream_end:
					return true;
				default:
					continue;
				}
			}
			return true;
		}

		bool consume_sequence() {
			consume_whitespace();
			return consume_preprocessor() || consume_line();
		}

		void operator () () {
			for ( ; consumed.available; ) {
				if ( !consume_sequence() ) {
					const token& t = consumed.t;
					if ( t.id == token_id::stream_end ) {
						consume();
						break;
					}
					// TODO: proper error
					// unexpected token, expected Sequence...
					throw parser_error();
				}
			}
		}
	};

}}}
