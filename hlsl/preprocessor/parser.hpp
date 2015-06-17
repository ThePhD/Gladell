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
		typedef buffer_view<token> view_type;
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
		parser( buffer_view<token> tokens ) : source( std::move( tokens ) ),
		begin( adl_cbegin( source ) ), end( adl_cend( source ) ),
		consumed( begin ),
		toplevel( source ), targetscope( toplevel.scopes.back() ) {
			
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
					if ( !consumed.available || consumed.t.get().id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					consume();
					if ( !consumed.available || consumed.t.get().id != token_id::block_comment_end ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					continue;
				case token_id::line_comment_begin:
					consume();
					if ( !consumed.available || consumed.t.get().id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					consume();
					if ( !consumed.available || consumed.t.get().id != token_id::line_comment_end ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
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

		bool consume_preprocessor() {
			if ( !consumed.prevlinewhitespace )
				return false;
			const token& t = consumed.t;
			if ( t.id != token_id::hash ) {
				return false;
			}
			consume();
			consume_whitespace();
			return true;
		}

		bool consume_line() {
			// TODO: can this... ever really fail?
			for ( ; consumed.available; consume() ) {
				switch ( consumed.t.get().id ) {
				case token_id::newlines:
					consume();
					return true;
				case token_id::stream_end:
					consume();
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
					// TODO: proper error
					// unexpected token, expected Sequence...
					throw parser_error();
				}
			}
		}
	};

}}}
