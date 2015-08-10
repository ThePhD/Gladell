#pragma once

#include "parse_tree.hpp"
#include "parse_result.hpp"
#include "symbol_table.hpp"
#include "construct.hpp"
#include "conditional_origin.hpp"
#include "precedence.hpp"
#include "../token.hpp"
#include "../parser_head.hpp"
#include "../parser_error.hpp"
#include "../../string.hpp"
#include "../../optional.hpp"
#include "../../range.hpp"
#include "../../stack.hpp"

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
				// TODO: proper error
				// expected token, but stream terminated
				throw parser_error();
			}
			expected_error( r.t, { std::forward<Tn>( argn )... } );
		}

		template <typename T0, typename T1, typename... Tn>
		void expected_error( const token& t, T0&& arg0, T1&& arg1, Tn&&... argn ) {
			expected_error( t, { std::forward<T0>( arg0 ), std::forward<T1>( arg1 ), std::forward<Tn>( argn )... } );
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

		whitespace parse_whitespace( read_head& r ) {
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
						throw parser_error();
					}
					advance( r );
					if ( !r.available || r.id != token_id::block_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						throw parser_error();
					}
					continue;
				case token_id::line_comment_begin:
					advance( r );
					if ( !r.available || r.id != token_id::comment_text ) {
						// TODO: proper parsing error
						// Expected comment text after block begin
						throw parser_error();
					}
					advance( r );
					if ( !r.available || r.id != token_id::line_comment_end ) {
						// TODO: proper parsing error
						// Expected comment end after block begin/text
						throw parser_error( );
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

		floating_literal parse_floating_literal( read_head& r ) {
			expected_error( r, token_id::float_literal );
			const read_head literalr = r;
			token_view literalseq( literalr.at, 1 );
			advance( r );
			return floating_literal( literalseq );
		}

		integral_literal parse_integral_literal( read_head& r ) {
			expected_error( r, token_id::integer_literal, token_id::integer_hex_literal, token_id::integer_octal_literal );
			const read_head literalr = r;
			token_view literalseq( literalr.at, 1 );
			advance( r );
			return integral_literal( literalseq );
		}

		string_literal parse_string_literal( read_head& r ) {
			expected_error( r, token_id::string_literal_begin );
			const read_head literalr = r;
			advance( r );
			expected_error( r, token_id::string_literal );
			const token& literaltoken = r.t;
			advance( r );
			expected_error( r, token_id::string_literal_end );
			token_view literalseq( literalr.at, 3 );
			advance( r );
			return string_literal( literalseq, literaltoken.lexeme );
		}

		variable parse_define_variable( const read_head& hashtokenreadhead, const read_head& idtokenreadhead, read_head& r ) {
			token_view idseq( idtokenreadhead.at, 1 );
			symbol id( idseq );
			auto definitionfind = symbols.definitions.find( id.name );
			if ( definitionfind != symbols.definitions.end() ) {
				definition& d = definitionfind->second;
				if ( !d.is<variable>() ) {
					// TODO: proper error
					// redeclaration of a preexisting type in this scope
					throw parser_error();
				}
				// TODO: proper error
				// Error/warning: redeclaring an already-defined symbol
				// TODO: warning ONLY if the actual defined symbol is different
				throw parser_error();
			}
			text_line substitutionline = parse_text_line( r );
			expected_error( r, token_id::preprocessor_statement_end );
			advance( r );
			token_view seq( hashtokenreadhead.at, r.at );
			return variable( seq, id, std::move( substitutionline ) );
		}

		function parse_define_function( const read_head& hashtokenreadhead, const read_head& idtokenreadhead, read_head& r ) {
			symbol id( token_view( idtokenreadhead.at, 1 ) );
			parse_whitespace( r );
			expected_error( r, token_id::open_parenthesis );
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
					advance( r );
					break;
				case token_id::comma:
					advance( r );
					continue;
				case token_id::dot_dot_dot:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						throw parser_error();
					}
					parameters.emplace_back( tseq );
					variablearguments = parameters.back();
					foundargument = true;
					advance( r );
					continue;
				case token_id::identifier:
					if ( variablearguments ) {
						// TODO: proper error
						// Argument expression already has a variable argument statement '...'
						// And must end on that statement
						throw parser_error();
					}
					parameters.emplace_back( tseq );
					foundargument = true;
					advance( r );
					continue;
				default:
					// TODO: proper error
					// Unexpected token, expected ')' or '...' or Identifier
					throw parser_error();
				}
				break;
			}
			substitution routine = parse_substitution( parameters, r );
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

			expected_error( r, token_id::preprocessor_statement_end );
			advance( r );

			token_view seq( hashtokenreadhead.at, r.at );
			function f( seq, id, std::move( parameters ), std::move( routine ) );
			auto definitionfind = symbols.definitions.find( f.name.name );
			if ( definitionfind != symbols.definitions.end() ) {
				definition& d = definitionfind->second;
				if ( !d.is<function>() ) {
					// TODO: proper error
					// redeclaration of a preexisting type in this scope
					throw parser_error();
				}
				// TODO: warning/error only if the definition is different from the first
				// throw parser_error();
			}
			return f;
		}

		definition parse_define( const read_head& hashtokenreadhead, read_head& r ) {
			expected_error( r, token_id::preprocessor_define );
			advance( r );
			expected_error( r, token_id::preprocessor_statement_begin );
			advance( r );
			parse_whitespace( r );
			expected_error( r, token_id::identifier );
			const read_head id = r;
			advance( r );
			parse_whitespace( r );
			switch ( r.id ) {
			case token_id::open_parenthesis: 
				return parse_define_function( hashtokenreadhead, id, r );
			default:
				break;
			}
			return parse_define_variable( hashtokenreadhead, id, r );
		}

		undefinition parse_undef( const read_head& hashtokenreadhead, read_head& r ) {
			expected_error( r, token_id::preprocessor_un_def );
			advance( r );
			parse_whitespace( r );
			expected_error( r, token_id::identifier );
			const token& idtoken = r.t;
			token_view idseq( &idtoken, 1 );
			symbol id( idseq );
			advance( r );
			token_view seq( hashtokenreadhead.at, r.at );
			undefinition u( seq, id );
			parse_whitespace( r );
			expected_error( r, token_id::preprocessor_statement_end );

			std::size_t count = symbols.definitions.erase( id.name );
			if ( count == 0 ) {
				// TODO: warning that 'undef'ing a symbol that doesn't exist?
			}
			else if ( count > 1 ) {
				// Should never hit this condition ever
				// TODO: proper error
				// multiply defined symbols being removed
				throw parser_error();
			}
			return u;
		}

		bool is_macro_end( token_id id ) const {
			switch ( id ) {
			case token_id::preprocessor_statement_end:
			case token_id::newlines:
				return true;
			}
			return false;
		}

		bool is_macro_end( read_head& r ) const {
			if ( !r.available ) {
				return true;
			}
			return is_macro_end( r.id );
		}

		expression_chain parse_expression_chain( read_head& r ) {
			expression_chain expr( token_view( r.at, r.at ) );
			token_view& seq = expr.tokens;
			const read_head beginr = r;
			for ( ; ; ) {
				const read_head ebeginr = r;
				stack<std::reference_wrapper<const token>> operations;
				// @ ## # 
				// * / + - ^ % | &  
				// || &&
				// != == < <= > >= 
				stack<std::reference_wrapper<const token>> terms;
				// Symbols, keywords
				
				for ( ;; ) {
					if ( is_macro_end( r ) ) {

					}
					switch ( r.id ) {
					case token_id::whitespace:
						break;
					case token_id::identifier:
						terms.push_back( r.t );
						break;
					// binary
					case token_id::add:
					//case token_id::add_assignment:
					case token_id::subtract:
					//case token_id::subtract_assignment:
					case token_id::multiply:
					//case token_id::multiply_assignment:
					case token_id::divide:
					//case token_id::divide_assignment:
					case token_id::modulus:
					//case token_id::modulus_assignment:
					case token_id::equal_to:
					case token_id::not_equal_to:
					case token_id::greater_than:
					case token_id::greater_than_or_equal_to:
					case token_id::less_than:
					case token_id::less_than_or_equal_to:
					case token_id::boolean_and:
					//case token_id::boolean_and_assignment:
					case token_id::boolean_or:
					//case token_id::boolean_or_assignment:
					case token_id::boolean_xor:
					//case token_id::boolean_xor_assignment:
					case token_id::expression_and:
					case token_id::expression_or:
					case token_id::token_pasting:
					{
						const operator_precedence& precedence = precedence_of( r.id ).get();
						operations.push_back( r.t );
						break;
					}
					// Unary
					case token_id::boolean_complement:
					case token_id::charizing:
					case token_id::stringizing:
					{
						const operator_precedence& precedence = precedence_of( r.id ).get();
						operations.push_back( r.t );
						break;
					}
					// "Function" calls
					case token_id::preprocessor_defined:
					{

						break;
					}
					default:
						// TODO: proper error
						// unexpected token in expression, expected {}
						throw parser_error();
					}
					advance( r );
				}
				token_view seq( ebeginr.at, r.at );
			}
			return expr;
		}

		conditional parse_conditional( conditional_origin origin, read_head& r ) {
			expression_chain e = parse_expression_chain( r );
			return conditional( origin, std::move( e ) );
		}

		conditional_block parse_conditional_branch( conditional_origin origin, read_head& r ) {
			expected_error( r, token_id::preprocessor_statement_begin );
			advance( r );
			conditional condition = parse_conditional( origin, r );
			block block = parse_block( r );
			return conditional_block( std::move( condition ), std::move( block ) );
		}

		if_elseif_else parse_if_elseif_else( const read_head& hashtokenreadhead, conditional_origin origin, read_head& r ) {
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
				throw parser_error();
			}
			advance( r );
			branches.success_blocks.push_back( parse_conditional_branch( origin, r ) );
			
			for ( bool endiftrigger = false; !endiftrigger; ) {
				parse_whitespace( r );
				switch ( r.id ) {
				case token_id::preprocessor_end_if:
					endiftrigger = true;
					continue;
				case token_id::preprocessor_if:
				case token_id::preprocessor_if_n_def:
				case token_id::preprocessor_if_def:
					// TODO: proper error
					// Cannot have if/ifndef/ifdef blocks chained when if/ifndef/ifdef has already been used
					throw parser_error();
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
				advance( r );
				branches.success_blocks.push_back( parse_conditional_branch( origin, r ) );
			}
			seq = token_view( hashtokenreadhead.at, r.at );
			return branches;
		}

		if_elseif_else parse_if( const read_head& hashtokenreadhead, read_head& r ) {
			return parse_if_elseif_else( hashtokenreadhead, conditional_origin::if_, r );
		}

		if_elseif_else parse_ifdef( const read_head& hashtokenreadhead, read_head& r ) {
			return parse_if_elseif_else( hashtokenreadhead, conditional_origin::if_def, r );
		}

		if_elseif_else parse_ifndef( const read_head& hashtokenreadhead, read_head& r ) {
			return parse_if_elseif_else( hashtokenreadhead, conditional_origin::if_n_def, r );
		}

		force_line parse_line_directive( const read_head& hashtokenreadhead, read_head& r ) {
			expected_error( r, token_id::preprocessor_statement_begin );
			advance( r );
			
			// TODO: implement line directive reading
			throw parser_error();

			expected_error( r, token_id::preprocessor_statement_end );
			advance( r );
		}

		inclusion parse_include( const read_head& hashtokenreadhead, read_head& r ) {
			const token& includetoken = r.t;
			advance( r );
			
			expected_error( r, token_id::preprocessor_statement_begin );
			advance( r );
			parse_whitespace( r );
			
			string_literal includeliteral = parse_string_literal( r );
			inclusion_style style = includetoken.value.get<inclusion_style>( );
			
			expected_error( r, token_id::preprocessor_statement_end );
			advance( r );

			token_view seq( hashtokenreadhead.at, r.at );

			return inclusion( seq, includeliteral, style );
		}

		pragma_construct parse_pragma( const read_head& hashtokenreadhead, read_head& r ) {
			advance( r );
			
			expected_error( r, token_id::preprocessor_statement_begin );
			advance( r );
			
			for ( ; r.available; advance( r ) ) {
				const token& t = r.t;
				if ( t.id == token_id::preprocessor_statement_end ) {
					break;
				}
			}
			
			expected_error( r, token_id::preprocessor_statement_end );
			advance( r );
			
			token_view seq( hashtokenreadhead.at, r.at );
			return pragma_construct( seq );
		}

		substitution parse_substitution( buffer_view<const symbol> parameters, read_head& r ) {
			// TODO: can this... ever really fail?
			auto beginat = r.at;
			auto lineat = r.at;
			std::vector<substitution_text> substitutiontext;
			auto commitline = [&]() {
				if ( lineat == r.at )
					return;
				token_view seq( lineat, r.at );
				substitutiontext.emplace_back( in_place_of<text_line>(), seq );
			};
			for ( ; r.available; ) {
				auto symbolfind = [ lexeme = r.t.get( ).lexeme ]( const symbol& s ) {
					return s.name == lexeme;
				};
				switch ( r.id ) {
				case token_id::preprocessor_variadic_arguments:
				case token_id::identifier:
					if ( !parameters.empty()
						&& std::find_if( parameters.begin(), parameters.end(), symbolfind ) != parameters.end() ) {
						commitline();
						substitutiontext.emplace_back( in_place_of<substitution_argument>(), token_view( r.at, 1 ) );
					}
					advance( r );
					lineat = r.at;
					continue;
				case token_id::whitespace:
					advance( r );
					continue;
				case token_id::preprocessor_statement_end:
				case token_id::stream_end:
				case token_id::newlines:
					break;
				default:
					continue;
				}
				break;
			}
			token_view seq( beginat, r.at );
			return substitution( seq, std::move( substitutiontext ) );
		}

		text_line parse_text_line( read_head& r ) {
			auto beginat = r.at;
			for ( ; r.available; advance( r ) ) {
				switch ( r.id ) {
				case token_id::preprocessor_block_end:
				case token_id::preprocessor_statement_end:
				case token_id::newlines:
					break;
				default:
					continue;
				}
				break;
			}
			token_view seq( beginat, r.at );
			return text_line( seq );
		}

		statement parse_preprocessor( read_head& r ) {
			const read_head hashtokenreadhead = r;
			advance( r );
			parse_whitespace( r );
			switch ( r.id ) {
			case token_id::preprocessor_un_def:
				return parse_undef( hashtokenreadhead, r );
			case token_id::preprocessor_define:
				{
					definition d = parse_define( hashtokenreadhead, r );
					switch ( d.class_index() ) {
					case definition::index<function>::value:
						return std::move( d.get<function>() );
					case definition::index<variable>::value:
					default:
						return std::move( d.get<variable>() );
					}
				}
				break;
			case token_id::preprocessor_if:
				return tree.make_if_elseif_else( parse_if( hashtokenreadhead, r ) );
			case token_id::preprocessor_if_def:
				return tree.make_if_elseif_else( parse_ifdef( hashtokenreadhead, r ) );
			case token_id::preprocessor_if_n_def:
				return tree.make_if_elseif_else( parse_ifndef( hashtokenreadhead, r ) );
			case token_id::preprocessor_else_if:
			case token_id::preprocessor_else_if_def:
			case token_id::preprocessor_else_if_n_def:
			case token_id::preprocessor_else:
			case token_id::preprocessor_end_if:
				// TODO: proper error
				// cannot else_if/else/endif without starting if/ifdef/ifndef
				throw parser_error();
				break;
			case token_id::preprocessor_include:
				return parse_include( hashtokenreadhead, r );
			case token_id::preprocessor_line:
				return parse_line_directive( hashtokenreadhead, r );
			case token_id::preprocessor_pragma:
				return parse_pragma( hashtokenreadhead, r );
			default:
				// Placeholder
				throw parser_error();
				break;
			}
		}

		statement parse_statement( read_head& r ) {
			parse_whitespace( r );
			switch ( r.id ) {
			case token_id::preprocessor_hash:
				if ( r.prevlinewhitespace ) {
					// Then it's valid and we parse other statements
					// TODO: bake this information into meaning of preprocessor_hash,
					// and distinguish it from the regular `hash` token_id
					return parse_preprocessor( r );
				}
				break;
			default:
				break;
			}
			return parse_text_line( r );
		}

		block parse_block( read_head& r ) {
			block resultblock;
			parse_block( r, resultblock );
			return resultblock;
		}

		void parse_block( read_head& r, block& resultblock ) {
			expected_error( r, token_id::preprocessor_block_begin );
			const token& blockbegintoken = r.t;
			intz blockid = blockbegintoken.value.get<intz>();
			advance( r );
			for ( ;; ) {
				parse_whitespace( r );
				switch ( r.id ) {
				case token_id::preprocessor_block_end:
				{
					const token& blockendtoken = r.t;
					intz endblockid = blockbegintoken.value.get<intz>();
					if ( blockid != endblockid ) {
						// TODO: proper error
						// mismatched block beginning at X, check to see if blocks are properly closed
						throw parser_error();
					}
					break;
				}
				case token_id::preprocessor_block_begin:
					advance( r );
				default:
				{
					statement statement = parse_statement( r );
					resultblock.statements.push_back( std::move( statement ) );
					continue;
				}
				}
				break;
			}
		}

		void parse_stream( read_head& r, block& targetblock ) {
			token_view& rootsequence = targetblock.tokens;
			auto beginat = r.at;
			expected_error( r, token_id::stream_begin );
			advance( r );
			rootsequence = token_view( beginat, 1 );
			
			parse_block( r, targetblock );
			
			expected_error( r, token_id::stream_end );
			advance( r );
			rootsequence = token_view( source.data(), &r.t.get() );
		}

	public:

		void operator () () {
			update( consumed );
			parse_stream( consumed, tree );
		}
	};

}}}
