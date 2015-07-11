#pragma once

#include "parse_tree.hpp"
#include "symbol_table.hpp"
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
		begin_iterator begin;
		end_iterator end;

		read_head consumed;
		
		block target;
		parse_tree& tree;
		symbol_table& symbols;
		
	public:
		parser( view_type tokens, parse_tree& tree, symbol_table& symbols ) : source( std::move( tokens ) ),
		begin( adl_cbegin( source ) ), end( adl_cend( source ) ),
		consumed( begin ),
		tree( tree ), symbols( symbols ) {
			
		}

	private:
		template <typename Range>
		std::vector<std::reference_wrapper<const token>> get_substitutions( const buffer_view<const token>& seq, Range& range ) {
			std::vector<std::reference_wrapper<const token>> subs;
			subs.reserve( 16 );
			auto itend = adl_cend( range );
			for ( auto it = adl_cbegin( range ); it != itend; ++it ) {
				const token& t = *it;
				auto tokenfind = std::find_if( seq.cbegin(), seq.cend(),
					[&t]( const token& l ) -> bool {
					return l.lexeme == t.lexeme;
				}
				);
				if ( tokenfind == seq.cend() ) {
					continue;
				}
				subs.emplace_back( *tokenfind );
			}
			return subs;
		}

		void expected_error ( const read_head& r, token_id id ) {
			if ( !r.available ) {
				// unexpected end of stream, expected 'id'
				throw parser_error();
			}
			expected_error( r.t, id );
		}

		void expected_error( const token& t, token_id id ) {
			if ( id != t.id ) {
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
			r.available = r.at != end;
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

		bool whitespace( read_head& r ) {
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

		bool preprocessor_define_variable( read_head& r, const iterator& beginat, const token& id ) {
			auto symbolfind = symbols.defines.find( id.lexeme );
			if ( symbolfind == symbols.defines.end() ) {
				auto maybemacroexpression = macro_statement( r );
				if ( !maybemacroexpression ) {
					// TODO: proper error
					// expected newline-terminated macro expression
					throw parser_error();
				}
				expression& macroexpression = *maybemacroexpression;
				symbolfind = symbols.defines.emplace_hint( symbolfind, id.lexeme, definition( token_view( beginat, consumed.at ), id, std::ref( macroexpression ) ) );
				return true;
			}
			else {
				// TODO: proper error
				// Error/warning: redeclaring an already-defined symbol
				// TODO: warning ONLY if the actual defined symbol is different
				throw parser_error();
			}
			return false;
		}

		bool preprocessor_define_function( read_head& r, const iterator& beginat, const token& id ) {
			consume_whitespace();
			expected_error( consumed.t, token_id::open_parenthesis );
			consume();
			
			bool foundargument = false;
			std::vector<std::reference_wrapper<const token>> arguments;
			arguments.reserve( 16 );
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
					arguments.emplace_back( t );
					foundargument = true;
					continue;
				case token_id::identifier:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						// And must end on that statement
						throw parser_error();
					}
					arguments.emplace_back( t );
					foundargument = true;
					continue;
				default:
					// TODO: proper error
					// Unexpected token, expected ')' or '...' or Identifier
					throw parser_error();
				}
				break;
			}
			auto maybemacroexpression = macro_statement();
			if ( !maybemacroexpression ) {
				// TODO: proper error
				// could not read the expression for this macro function expression
				throw parser_error();
			}
			expression& macroexpression = *maybemacroexpression;
			if ( !foundargument ) {
				// TODO: is it an error if there are parenthesis, but no arguments?
				// I can imagine sometimes you might want to have () be part of the macro's interface
				// to just ensure some text is being 'called' with no parameters, so that's actually
				// a technically-good constraint that can come from macros
				// throw parser_error();
			}
			else {
				// Create substitutions and find items
			}


			auto functionfind = toplevel.functions.find( id.lexeme );
			if ( functionfind != toplevel.functions.end() ) {
				// TODO: warning/error only if the defintion is different from the first
				// throw parser_error();
			}
			else {
				// TODO: do we want to preserve the memory here?
				// We probably won't have more than 20, and even if we
				// over-allocated 20 for macro params, I don't think we're
				// going to take such a bit memory hit that we need to shrink it...
				// arguments.shrink_to_fit();
				// substitutions.shrink_to_fit();
				std::vector<std::reference_wrapper<const token>> substitutions = get_substitutions( macroexpression.tokens, arguments );
				toplevel.functions.emplace_hint( functionfind, id.lexeme, function( token_view( beginat, consumed.at ), id, macroexpression, std::move( arguments ), std::move( substitutions ) ) );
			}
			return true;
		}

		bool preprocessor_define( read_head& r ) {
			expected_error( consumed, token_id::preprocessor_statement_begin );
			auto beginat = consumed.at;
			consume();
			consume_whitespace();
			expected_error( consumed, token_id::identifier );
			const token& id = consumed.t;
			consume();
			consume_whitespace();
			switch ( consumed.t.get().id ) {
			case token_id::open_parenthesis:
				return preprocessor_define_function( beginat, id );
			default:
				return consume_preprocessor_define_variable( beginat, id );
			}
			
			return true;
		}

		bool preprocessor_undef( read_head& r ) {
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

		bool preprocessor_conditional_begin_scope( read_head& r, scope& enclosing ) {
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

		bool preprocessor_conditional_end_scope( read_head& r, scope& enclosing ) {
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

		bool preprocessor_if( read_head& r ) {
			return preprocessor_conditional_begin_scope( r, targetscope );
		}

		bool preprocessor_ifdef( read_head& r ) {
			return preprocessor_conditional_begin_scope( r, targetscope );
		}

		bool preprocessor_ifndef( read_head& r ) {
			return preprocessor_conditional_begin_scope( r, targetscope );
		}

		bool preprocessor_elif( read_head& r ) {
			return preprocessor_conditional_end_scope( r, targetscope )
				&& preprocessor_conditional_begin_scope( r, targetscope );
		}

		bool preprocessor_else( read_head& r ) {
			return preprocessor_conditional_end_scope( r, targetscope )
				&& preprocessor_conditional_begin_scope( r, targetscope );
		}

		bool preprocessor_endif( read_head& r ) {
			return preprocessor_conditional_end_scope( r, targetscope );
		}

		bool preprocessor_line( read_head& r ) {
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

		bool preprocessor_include( read_head& r ) {
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

		bool consume_preprocessor_pragma( read_head& r ) {
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

		bool consume_preprocessor( read_head& r ) {
			if ( consumed.t.get().id != token_id::preprocessor_hash ) {
				return false;
			}
			consume();
			consume_whitespace();
			const token& t = consumed.t;
			switch ( t.id ) {
			case token_id::preprocessor_undef:
				consume();
				consume_whitespace();
				return preprocessor_undef();
			case token_id::preprocessor_define:
				consume();
				consume_whitespace();
				return preprocessor_define();
			case token_id::preprocessor_if:
				consume();
				consume_whitespace();
				return preprocessor_if();
			case token_id::preprocessor_ifdef:
				consume();
				consume_whitespace();
				return preprocessor_ifdef();
			case token_id::preprocessor_ifndef:
				consume();
				consume_whitespace();
				return preprocessor_ifndef();
			case token_id::preprocessor_elif:
				consume();
				consume_whitespace();
				return preprocessor_elif();
			case token_id::preprocessor_else:
				consume();
				consume_whitespace();
				return preprocessor_else();
			case token_id::preprocessor_endif:
				consume();
				consume_whitespace();
				return preprocessor_endif();
			case token_id::preprocessor_include:
				consume();
				consume_whitespace();
				return preprocessor_include();
			case token_id::preprocessor_line:
				consume();
				consume_whitespace();
				return preprocessor_line();
			case token_id::preprocessor_pragma:
				consume();
				consume_whitespace();
				return consume_preprocessor_pragma();
			default:
				return false;
			}
			return true;
		}

		optional<expression&> macro_statement( read_head& r ) {
			auto beginat = consumed.at;
			for ( ; consumed.available; consume() ) {
				switch ( consumed.t.get().id ) {
				case token_id::preprocessor_statement_end:
					consume();
					toplevel.expressions.emplace_back( std::make_unique<expression>( token_view( beginat, consumed.at ) ) );
					return *toplevel.expressions.back();
				default:
					continue;
				}
				break;
			}
			return none;
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

		bool sequence( read_head& r ) {
			if ( !r.linewhitespace && r.prevlinewhitespace ) {
				whitespace( r );
				const token& thash = consumed.t;
				if ( thash.id == token_id::preprocessor_hash ) {
					advance( r );
					whitespace( r );
					return consume_preprocessor();
				}
			}

		}

		bool consume() {
			return advance( consumed );
		}

		bool consume_whitespace() {
			return whitespace( consumed );
		}

		bool consume_sequence() {
			consume_whitespace();
			return consume_preprocessor() || consume_line();
		}

	public:

		void operator () () {
			update( consumed );
			for ( ; consumed.available; ) {
				const token& t = consumed.t;
				switch ( t.id ) {
				case token_id::stream_begin:
				case token_id::stream_end:
					consume();
					break;
				default:
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
		}
	};

}}}
