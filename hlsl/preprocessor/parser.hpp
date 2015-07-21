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

	template <typename T>
	struct parse_result {
		variant<parser_error, T> result;

		struct visitor {
			T& operator() ( T& a ) const {
				return a;
			}

			const T& operator() ( const T& a ) const {
				return a;
			}

			T& operator() ( parser_error& error ) const {
				throw error;
			}

			const T& operator() ( const parser_error& error ) const {
				throw error;
			}
		};

		template <typename T0, typename... Tn, typename = std::enable_if_t<!std::is_same<Furrovine::tmp::unqualified_t<T0>, parse_result>::value && !std::is_same<Furrovine::tmp::unqualified_t<T0>, parser_error>::value>>
		parse_result( T0&& arg0, Tn&&... argn ) : result( in_place_of<T>(), std::forward<T0>( arg0 ), std::forward<Tn>( argn )... ) {

		}

		parse_result( const parser_error& e ) : result( in_place_of<parser_error>(), e ) {

		}

		parse_result( parser_error&& e ) : result( in_place_of<parser_error>(), std::move( e ) ) {

		}

		parse_result( const parse_result& ) = default;
		parse_result& operator=( const parse_result& ) = default;
		parse_result( parse_result&& ) = default;
		parse_result& operator=( parse_result&& ) = default;

		bool valid() const {
			return result.is<T>();
		}

		explicit operator bool() const {
			return valid();
		}

		T& get ( ) {
			return result.visit( visitor( ) );
		}

		const T& get ( ) const {
			return result.visit( visitor( ) );
		}

		operator T& () {
			return get();
		}

		operator const T& () const {
			return get();
		}
	};

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
			r.id = r.t.get().id;
			r.prevlinewhitespace = r.linewhitespace;
			switch ( r.id ) {
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

		parse_result<sequence> parse_whitespace( read_head& r ) {
			auto beginat = r.at;
			for ( ; r.available; ) {
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
					if ( !r.available || r.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						return parser_error();
					}
					advance( r );
					if ( !r.available || r.id != token_id::block_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						return parser_error();
					}
					continue;
				case token_id::line_comment_begin:
					advance( r );
					if ( !r.available || r.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						return parser_error();
					}
					advance( r );
					if ( !r.available || r.id != token_id::line_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						return parser_error( );
					}
					continue;
				default:
					break;
				}
				break;
			}
			return sequence( beginat, r.at );
		}

		parse_result<expression> parse_expression( read_head& r ) {
			return parser_error();
		}

		parse_result<definition> parse_define_variable( read_head& r, const iterator& beginat, const token& idtoken ) {
			symbol id( token_view( &idtoken, &idtoken + 1 ) );
			auto symbolfind = symbols.defines.find( id.name );
			if ( symbolfind == symbols.defines.end() ) {
				auto maybesubstitutionline = parse_substitution_line( r );
				if ( !maybesubstitutionline ) {
					// TODO: proper error
					// expected newline-terminated macro expression
					return parser_error( );
				}
				substitution_line& substitutionline = maybesubstitutionline;
				return definition( token_view( beginat, r.at ), id, std::move( substitutionline ) );
			}
			// TODO: proper error
			// Error/warning: redeclaring an already-defined symbol
			// TODO: warning ONLY if the actual defined symbol is different
			return parser_error();
		}

		parse_result<function> parse_define_function( read_head& r, const iterator& beginat, const token& idtoken ) {
			symbol id( token_view( &idtoken, &idtoken + 1 ) );
			parse_whitespace( r );
			expected_error( r.t, token_id::open_parenthesis );
			advance( r );
			bool foundargument = false;
			std::vector<symbol> parameters;
			parameters.reserve( 16 );
			optional<const symbol&> variablearguments = none;
			for ( ; consumed.available; ) {
				parse_whitespace( r );
				const token& t = r.t;
				token_view tseq( &t, &t + 1 );
				switch ( t.id ) {
				case token_id::close_parenthesis:
					break;
				case token_id::dot_dot_dot:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						return parser_error();
					}
					parameters.emplace_back( tseq );
					variablearguments = parameters.back();
					foundargument = true;
					continue;
				case token_id::identifier:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						// And must end on that statement
						return parser_error();
					}
					parameters.emplace_back( tseq );
					foundargument = true;
					continue;
				default:
					// TODO: proper error
					// Unexpected token, expected ')' or '...' or Identifier
					return parser_error();
				}
				break;
			}
			auto mayberoutine = parse_substitution( r, parameters );
			if ( !mayberoutine ) {
				// TODO: proper error
				// could not read the expression for this macro function expression
				throw parser_error();
			}
			substitution& routine = mayberoutine.get();
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

			token_view seq( beginat, consumed.at );
			function f( seq, id, std::move( parameters ), std::move( routine ) );
			auto functionfind = symbols.functions.find( id.name );
			if ( functionfind != symbols.functions.end() ) {
				// TODO: warning/error only if the defintion is different from the first
				// throw parser_error();
			}
			symbols.functions.emplace_hint( functionfind, id.name, f );
			return f;
		}

		statement parse_define( read_head& r ) {
			expected_error( consumed, token_id::preprocessor_statement_begin );
			auto beginat = consumed.at;
			advance( r );
			parse_whitespace( r );
			expected_error( consumed, token_id::identifier );
			const token& id = consumed.t;
			advance( r );
			parse_whitespace( r );
			switch ( consumed.id ) {
			case token_id::open_parenthesis: {
				function f = parse_define_function( r, beginat, id );
				return f;
			}
			default:
				break;
			}
			definition d = parse_define_variable( r, beginat, id );
			return d;
		}

		parse_result<undefinition> parse_undef( read_head& r ) {
			const token& t = consumed.t;
			expected_error( t, token_id::identifier );
			std::size_t count = symbols.defines.erase( t.lexeme );
			if ( count == 0 ) {
				// TODO: warning that 'undef'ing a symbol that doesn't exist?
			}
			else if ( count > 1 ) {
				// Should never hit this condition ever
				// TODO: proper error
				// multiply defined symbols being removed
				return parser_error();
			}
			token_view seq( &t, &t + 1 );
			return undefinition( seq );
		}

		bool preprocessor_conditional_begin_scope( read_head& r ) {
			/*auto conditionread = r;
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
			*/
			return true;
		}

		bool preprocessor_conditional_end_scope( read_head& r ) {
			/*auto conditionread = consumed;
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
			*/
			return true;
		}

		bool preprocessor_if( read_head& r ) {
			return preprocessor_conditional_begin_scope( r );
		}

		bool preprocessor_ifdef( read_head& r ) {
			return preprocessor_conditional_begin_scope( r );
		}

		bool parse_ifndef( read_head& r ) {
			return preprocessor_conditional_begin_scope( r );
		}

		bool parse_elif( read_head& r ) {
			return preprocessor_conditional_end_scope( r )
				&& preprocessor_conditional_begin_scope( r );
		}

		bool parse_else( read_head& r ) {
			return preprocessor_conditional_end_scope( r )
				&& preprocessor_conditional_begin_scope( r );
		}

		bool parse_endif( read_head& r ) {
			return preprocessor_conditional_end_scope( r );
		}

		bool parse_line_directive( read_head& r ) {
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

		parse_result<inclusion> parse_include( read_head& r, const token& includetoken ) {
			auto conditionread = r;
			optional<const token&> include;
			token_view literalseq;
			for ( ; conditionread.available; advance( conditionread ) ) {
				const token& t = conditionread.t;
				switch ( t.id ) {
				case token_id::string_literal_begin:
					if ( include ) {
						// TODO: proper error
						// two string literals on the same include line: syntax error
						return parser_error();
					}
					advance( conditionread );
					expected_error( conditionread, token_id::string_literal );
					{
						const token& literaltoken = conditionread.t;
						include = literaltoken;
						advance( conditionread );
						expected_error( conditionread, token_id::string_literal_end );
						literalseq = token_view( &literaltoken - 1, &literaltoken + 1 );
					}
					continue;
				case token_id::preprocessor_statement_end:
					break;
				}
				break;
			}
			advance( conditionread );
			r = conditionread;

			token_view seq( r.at, conditionread.at );
			inclusion_style style = includetoken.value.get<inclusion_style>();
			return inclusion( literalseq, string_literal( literalseq, literalseq[ 1 ].lexeme ), style );
		}

		bool parse_pragma( read_head& r ) {
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

		parse_result<substitution> parse_substitution( read_head& r, buffer_view<const symbol> parameters ) {
			// TODO: can this... ever really fail?
			auto beginat = r.at;
			auto lineat = r.at;
			std::vector<substitution_text> substitutiontext;
			std::vector<symbol> substitutionsymbols;
			auto commitline = [&]() {
				if ( lineat == r.at )
					return;
				token_view seq( lineat, r.at );
				substitutiontext.emplace_back( in_place_of<substitution_line>(), seq, std::move( substitutionsymbols ) );
				lineat = r.at;
			};
			for ( ; r.available; advance( r ) ) {
				auto symbolfind = [ lexeme = r.t.get().lexeme ]( const symbol& s ) {
					return s.name == lexeme;
				};
				switch ( r.id ) {
				case token_id::newlines:
					break;
				case token_id::identifier:
					if ( std::find_if( parameters.begin(), parameters.end(), symbolfind ) != parameters.end() ) {
						commitline();
						substitutiontext.emplace_back( in_place_of<substitution_argument>(), token_view( r.at, std::next( r.at, 1 ) ) );
					}
					else {
						substitutionsymbols.emplace_back( token_view( r.at, std::next( r.at, 1 ) ) );
					}
					continue;
				case token_id::stream_end:
					break;
				default:
					continue;
				}
				break;
			}
			token_view seq( beginat, r.at );
			return substitution( seq, substitutiontext );
		}

		parse_result<substitution_argument> parse_substitution_argument( read_head& r ) {
			return parser_error();
		}

		parse_result<substitution_line> parse_substitution_line( read_head& r ) {
			// TODO: can this... ever really fail?
			auto beginat = r.at;
			std::vector<symbol> substitutionsymbols;
			for ( bool stop = false; !stop && r.available; advance( r ) ) {
				switch ( r.id ) {
				case token_id::newlines:
					advance( r );
					stop = true;
					continue;
				case token_id::identifier:
					substitutionsymbols.emplace_back( token_view( r.at, std::next( r.at, 1 ) ) );
					continue;
				case token_id::stream_end:
					continue;
				default:
					continue;
				}
				break;
			}
			return substitution_line( token_view( beginat, r.at ), substitutionsymbols );
		}

		parse_result<text_line> parse_text_line( read_head& r ) {
			auto beginat = r.at;
			for ( bool stop = false; !stop && r.available; advance( r ) ) {
				switch ( r.id ) {
				case token_id::newlines:
					advance( r );
					stop = true;
					continue;
				case token_id::stream_end:
					continue;
				default:
					continue;
				}
				break;
			}
			return text_line( token_view( beginat, r.at ) );
		}

		parse_result<statement> parse_preprocessor( read_head& r ) {
			//advance( r );
			parse_whitespace( r );
			const token& t = r.t;
			switch ( t.id ) {
			case token_id::preprocessor_undef:
				advance( r );
				parse_whitespace( r );
				{
					parse_result<undefinition> result = parse_undef( r );
				}
				break;
			case token_id::preprocessor_define:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_define( r );
				}
				break;
			case token_id::preprocessor_if:
				advance( r );
				parse_whitespace( r );
				{
					auto result = preprocessor_if( r );
				}
				break;
			case token_id::preprocessor_ifdef:
				advance( r );
				parse_whitespace( r );
				{
					auto result = preprocessor_ifdef( r );
				}
				break;
			case token_id::preprocessor_ifndef:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_ifndef( r );
				}
				break;
			case token_id::preprocessor_elif:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_elif( r );
				}
				break;
			case token_id::preprocessor_else:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_else( r );
				}
				break;
			case token_id::preprocessor_endif:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_endif( r );
				}
				break;
			case token_id::preprocessor_include:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_include( r, t );
				}
				break;
			case token_id::preprocessor_line:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_line_directive( r );
				}
				break;
			case token_id::preprocessor_pragma:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_pragma( r );
				}
				break;
			default:
				break;
			}
			// Placeholder
			return parser_error();
		}

		parse_result<statement> parse_statement( read_head& r ) {
			switch ( r.id ) {
			case token_id::preprocessor_hash:
				if ( r.prevlinewhitespace ) {
					// Then it's valid and we parse other statements
					// TODO: bake this information into meaning of preprocessor_hash,
					// and distinguish it from the regular `hash` token_id
					parse_preprocessor( r );
					// Placeholder
					return parser_error();
				}
				break;
			default:
				break;
			}
			parse_result<text_line> maybetextline = parse_text_line( r );
			return parse_result<statement>( in_place_of<text_line>(), maybetextline.get() );
		}

	public:

		void operator () () {
			update( consumed );
			buffer_view<const token> rootsequence( source.data(), source.data() );
			bool streambegin = false, streamend = false;
			for ( ; consumed.available && !streamend; ) {
				const token& t = consumed.t;
				switch ( consumed.id ) {
				case token_id::stream_begin:
					rootsequence = token_view( &t, &t );
					if ( streambegin ) {
						// Two stream-begin tokens... was there a nested sequence concatenated to this one?
					}
					streambegin = true;
					advance( consumed );
					break;
				case token_id::stream_end:
					advance( consumed );
					streamend = true;
					break;
				default:
					parse_statement( consumed );
					break;
				}
				rootsequence = token_view( source.data(), &consumed.t.get() );
			}
		}
	};

}}}
