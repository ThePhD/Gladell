#pragma once

#include "parse_tree.hpp"
#include "parse_result.hpp"
#include "symbol_table.hpp"
#include "construct.hpp"
#include "conditional_origin.hpp"
#include "../token.hpp"
#include "../parser_head.hpp"
#include "../parser_error.hpp"
#include "../../string.hpp"
#include "../../range.hpp"

namespace gld { namespace hlsl { namespace pp {

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
		
		parse_tree& tree;
		symbol_table& symbols;
		
	public:
		parser( view_type tokens, parse_tree& tree, symbol_table& symbols ) : source( std::move( tokens ) ),
		begin( adl_cbegin( source ) ), end( adl_cend( source ) ),
		consumed( begin ),
		tree( tree ), symbols( symbols ) {
			
		}

	private:
		template <typename... Tn>
		void expected_error ( const read_head& r, Tn&&... argn ) {
			if ( !r.available ) {
				// unexpected end of stream, expected 'id'
				throw parser_error();
			}
			expected_error( r.t, { std::forward<Tn>( argn )... } );
		}

		template <typename T0, typename T1, typename... Tn>
		void expected_error( const token& t, T0&& arg0, T1&& arg1, Tn&&... argn ) {
			expected_error( t, { std::forward<T0>( arg0 ), std::forward<T1>( arg1 ), std::forward<Tn>( argn )... } );
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

		parse_result<whitespace> parse_whitespace( read_head& r ) {
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
			token_view seq( beginat, r.at );
			return whitespace( seq );
		}

		parse_result<integral_literal> parse_integral_literal( read_head& r ) {
			expected_error( r, token_id::integer_literal, token_id::integer_hex_literal, token_id::integer_octal_literal );
			const token& literaltoken = r.t;
			token_view literalseq( &literaltoken, 1 );
			advance( r );
			return integral_literal( literalseq );
		}

		parse_result<string_literal> parse_string_literal( read_head& r ) {
			expected_error( r, token_id::string_literal_begin );
			advance( r );
			expected_error( r, token_id::string_literal );
			const token& literaltoken = r.t;
			advance( r );
			expected_error( r, token_id::string_literal_end );
			token_view literalseq( &literaltoken - 1, 3 );
			advance( r );
			return string_literal( literalseq, literalseq[ 1 ].lexeme );
		}

		parse_result<variable> parse_define_variable( read_head& r, const iterator& beginat, const token& idtoken ) {
			symbol id( token_view( &idtoken, &idtoken + 1 ) );
			auto symbolfind = symbols.variables.find( id.name );
			if ( symbolfind == symbols.variables.end() ) {
				auto resultsubstitutionline = parse_text_line( r );
				if ( !resultsubstitutionline ) {
					// TODO: proper error
					// expected newline-terminated macro expression
					return resultsubstitutionline.exception();
				}
				text_line& substitutionline = resultsubstitutionline;
				return variable( token_view( beginat, r.at ), id, std::move( substitutionline ) );
			}
			// TODO: proper error
			// Error/warning: redeclaring an already-defined symbol
			// TODO: warning ONLY if the actual defined symbol is different
			return parser_error();
		}

		parse_result<function> parse_define_function( read_head& r, const iterator& beginat, const token& idtoken ) {
			symbol id( token_view( &idtoken, 1 ) );
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
			auto resultroutine = parse_substitution( r, parameters );
			if ( !resultroutine ) {
				// TODO: proper error
				// could not read the substituion for this macro function expression
				return parser_error();
			}
			substitution& routine = resultroutine.get();
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
			auto functionfind = symbols.functions.find( f.name.name );
			if ( functionfind != symbols.functions.end() ) {
				// TODO: warning/error only if the defintion is different from the first
				// throw parser_error();
			}
			return f;
		}

		parse_result<definition> parse_define( read_head& r ) {
			expected_error( r, token_id::preprocessor_statement_begin );
			auto beginat = r.at;
			advance( r );
			parse_whitespace( r );
			expected_error( r, token_id::identifier );
			const token& id = r.t;
			advance( r );
			parse_whitespace( r );
			switch ( r.id ) {
			case token_id::open_parenthesis: 
			{
				auto resultfunction = parse_define_function( r, beginat, id );
				if ( !resultfunction ) {
					return resultfunction.exception();
				}
				function& f = resultfunction;
				return std::move( f );
			}
			default:
				break;
			}
			auto resultvariable = parse_define_variable( r, beginat, id );
			if ( !resultvariable ) {
				return resultvariable.exception();
			}
			variable& v = resultvariable;
			return std::move( v );
		}

		parse_result<undefinition> parse_undef( const token& hashtoken, read_head& r ) {
			expected_error( r, token_id::preprocessor_statement_begin );
			auto beginat = r.at;
			advance( r );
			parse_whitespace( r );
			expected_error( r, token_id::identifier );
			const token& idtoken = r.t;
			token_view idseq( &idtoken, 1 );
			symbol id( idseq );
			advance( r );
			token_view seq( beginat, r.at );
			undefinition u( seq, id );
			parse_whitespace( r );
			expected_error( r, token_id::preprocessor_statement_end );

			std::size_t count = symbols.variables.erase( id.name );
			if ( count == 0 ) {
				// TODO: warning that 'undef'ing a symbol that doesn't exist?
			}
			else if ( count > 1 ) {
				// Should never hit this condition ever
				// TODO: proper error
				// multiply defined symbols being removed
				return parser_error();
			}
			return u;
		}

		parse_result<expression> parse_expression( read_head& r ) {
			// TODO: fill this in properly
			return parser_error();
		}

		parse_result<conditional> parse_conditional( read_head& r, conditional_origin origin ) {
			auto resultexpression = parse_expression( r );
			if ( !resultexpression ) {
				return resultexpression.exception();
			}
			return conditional( origin, resultexpression );
		}

		parse_result<conditional_block> parse_conditional_branch( read_head& r, conditional_origin origin ) {
			auto resultcondition = parse_conditional( r, origin );
			if ( !resultcondition ) {
				return resultcondition.exception();
			}
			conditional& condition = resultcondition;
			auto resultblock = parse_block( r );
			if ( !resultblock ) {
				return resultblock.exception();
			}
			block& block = resultblock;
			return conditional_block( std::move( condition ), std::move( block ) );
		}

		parse_result<if_elseif_else> parse_if_elseif_else( read_head& r, conditional_origin origin ) {
			if_elseif_else branches;
			token_view& seq = branches.tokens;
			switch ( origin ) {
			case conditional_origin::if_:
			case conditional_origin::if_def:
			case conditional_origin::if_n_def:
				break;
			default:
				// TODO: proper error
				// cannot have else(if) as first engagement of block
				return parser_error();
			}
			auto resultconditionalbranch = parse_conditional_branch( r, origin );
			if ( !resultconditionalbranch ) {
				return resultconditionalbranch.exception();
			}
			branches.success_blocks.push_back( std::move( resultconditionalbranch.get() ) );
			
			for ( bool endiftrigger = false; !endiftrigger; ) {
				parse_whitespace( r );
				switch ( r.id ) {
				case token_id::preprocessor_endif:
					endiftrigger = true;
					continue;
				case token_id::preprocessor_if:
				case token_id::preprocessor_if_n_def:
				case token_id::preprocessor_if_def:
					// TODO: proper error
					// Cannot have if/ifndef/ifdef blocks chained when if/ifndef/ifdef has already been used
					return parser_error();
				case token_id::preprocessor_else_if:
					origin = conditional_origin::else_if;
					break;
				case token_id::preprocessor_else_if_def:
					origin = conditional_origin::else_if_def;
					break;
				case token_id::preprocessor_else_if_n_def:
					origin = conditional_origin::else_if_n_def;
					break;
				case token_id::preprocessor_else:
					origin = conditional_origin::else_;
					break;
				}
				auto resultconditionalbranch = parse_conditional_branch( r, origin );
				if ( !resultconditionalbranch ) {
					return resultconditionalbranch.exception();
				}
				branches.success_blocks.push_back( std::move( resultconditionalbranch.get() ) );
			}
			return branches;
		}

		parse_result<if_elseif_else> preprocessor_if( read_head& r ) {
			return parse_if_elseif_else( r, conditional_origin::if_ );
		}

		parse_result<if_elseif_else> preprocessor_ifdef( read_head& r ) {
			return parse_if_elseif_else( r, conditional_origin::if_def );
		}

		parse_result<if_elseif_else> parse_ifndef( read_head& r ) {
			return parse_if_elseif_else( r, conditional_origin::if_n_def );
		}

		parse_result<force_line> parse_line_directive( read_head& r ) {
			// TODO: implement line directive reading
			return parser_error();
		}

		parse_result<inclusion> parse_include( read_head& r, const token& includetoken ) {
			auto beginat = r.at;
			expected_error( r, token_id::preprocessor_statement_begin );
			parse_whitespace( r );
			string_literal includeliteral = parse_string_literal( r );
			inclusion_style style = includetoken.value.get<inclusion_style>( );
			token_view seq( beginat, r.at );
			return inclusion( seq, includeliteral, style );
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
			auto commitline = [&]() {
				if ( lineat == r.at )
					return;
				token_view seq( lineat, r.at );
				substitutiontext.emplace_back( in_place_of<text_line>(), seq );
				lineat = r.at;
			};
			for ( ; r.available; advance( r ) ) {
				parse_result<whitespace> w = parse_whitespace( r );
				if ( !w ) {
					return w.exception( );
				}
				auto symbolfind = [ lexeme = r.t.get( ).lexeme ]( const symbol& s ) {
					return s.name == lexeme;
				};
				switch ( r.id ) {
				case token_id::identifier:
					if ( !parameters.empty()
						&& std::find_if( parameters.begin(), parameters.end(), symbolfind ) != parameters.end() ) {
						commitline();
						substitutiontext.emplace_back( in_place_of<substitution_argument>(), token_view( r.at, 1 ) );
					}
					continue;
				default:
					continue;
				}
				break;
			}
			token_view seq( beginat, r.at );
			return substitution( seq, std::move( substitutiontext ) );
		}

		parse_result<text_line> parse_text_line( read_head& r ) {
			auto beginat = r.at;
			for ( ; r.available; advance( r ) ) {
				switch ( r.id ) {
				case token_id::preprocessor_hash:
					break;
				case token_id::stream_begin:
					break;
				case token_id::stream_end:
					break;
				case token_id::newlines:
					break;
				case token_id::preprocessor_statement_begin:
					break;
				case token_id::preprocessor_statement_end:
					break;
				default:
					continue;
				}
				break;
			}
			return text_line( token_view( beginat, r.at ) );
		}

		parse_result<statement> parse_preprocessor( const token& hashtoken, read_head& r ) {
			expected_error( r, token_id::preprocessor_statement_begin );
			switch ( r.id ) {
			case token_id::preprocessor_un_def:
				advance( r );
				parse_whitespace( r );
				{
					parse_result<undefinition> result = parse_undef( hashtoken, r );
					if ( !result ) {
						return result.exception( );
					}
					return result.get();
				}
				break;
			case token_id::preprocessor_define:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_define( r );
					if ( !result ) {
						return result.exception();
					}
					definition& d = result.get();
					switch ( d.class_index() ) {
					case definition::index<function>::value:
						return d.get<function>();
					case definition::index<variable>::value:
					default:
						return d.get<variable>();
					}
				}
				break;
			case token_id::preprocessor_if:
			case token_id::preprocessor_if_def:
			case token_id::preprocessor_if_n_def:
				advance( r );
				parse_whitespace( r );
				{
					auto result = parse_ifndef( r );
				}
				break;
			case token_id::preprocessor_else_if:
			case token_id::preprocessor_else_if_def:
			case token_id::preprocessor_else_if_n_def:
			case token_id::preprocessor_else:
			case token_id::preprocessor_endif:
				// TODO: proper error
				// cannot else_if/else/endif without starting if/ifdef/ifndef
				return parser_error();
			case token_id::preprocessor_include:
				{
					const token& t = r.t;
					advance( r );
					parse_whitespace( r );
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
			parse_whitespace( r );
			switch ( r.id ) {
			case token_id::preprocessor_hash:
				if ( r.prevlinewhitespace ) {
					// Then it's valid and we parse other statements
					// TODO: bake this information into meaning of preprocessor_hash,
					// and distinguish it from the regular `hash` token_id
					const token& hashtoken = r.t;
					advance( r );
					parse_whitespace( r );
					auto result = parse_preprocessor( hashtoken, r );
					if ( !result ) {
						return result.exception();
					}
					return result.get();
				}
				break;
			default:
				break;
			}
			parse_result<text_line> resulttextline = parse_text_line( r );
			if ( !resulttextline ) {
				return resulttextline.exception( );
			}
			return resulttextline.get( );
		}

		parse_result<block> parse_block( read_head& r ) {
			expected_error( r, token_id::preprocessor_block_begin );
			block resultblock;
			for ( ; r.id == token_id::preprocessor_block_begin; ) {
				advance( r );
				parse_result<statement> resultstatement = parse_statement( consumed );
				if ( !resultstatement ) {
					// here we throw the error
					// TODO: we may need to attempt some recovery at this point?
					return resultstatement.exception();
				}
				statement& statement = resultstatement;
				resultblock.statements.push_back( std::move( statement ) );
				expected_error( r, token_id::preprocessor_block_end );
				advance( r );
			}
			return resultblock;
		}

	public:

		void operator () () {
			update( consumed );
			token_view& rootsequence = tree.tokens;
			rootsequence = token_view(source.data(), source.data());
			bool streambegin = false, streamend = false;
			for ( ; consumed.available && !streamend; ) {
				switch ( consumed.id ) {
				case token_id::stream_begin:
					rootsequence = token_view( consumed.at, 1 );
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
				case token_id::preprocessor_hash:
				{
					auto resultstatement = parse_statement( consumed );
					if ( !resultstatement ) {
						throw resultstatement.exception();
					}
					tree.statements.push_back( std::move( resultstatement.get() ) );
					break;
				}
				case token_id::preprocessor_block_begin:
				{
					auto resultblock = parse_block( consumed );
					if ( !resultblock ) {
						// Throw the error here
						throw resultblock.exception();
					}
					block& blockref = resultblock;
					index_ref<block> blockindex = tree.make_block( std::move( resultblock ) );
					tree.statements.push_back( blockindex );
					break;
				}
				default:
					break;
				}
				rootsequence = token_view( source.data(), &consumed.t.get() );
			}
		}
	};

}}}
