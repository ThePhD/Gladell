#pragma once

#include "../token.hpp"
#include "../../optional.hpp"
#include "../../string.hpp"
#include "../../range.hpp"
#include "../../stack.hpp"
#include <functional>
#include <vector>
#include <unordered_map>

namespace gld { namespace hlsl { namespace preprocessor {

	struct sequence {
		buffer_view<const token> tokens;

		template <typename... Tn>
		sequence( Tn&&... argn ) : tokens( std::forward<Tn>( argn )... ) {

		}
	};

	struct symbol : sequence {
		std::reference_wrapper<const token> name;

		symbol( buffer_view<const token> seq, const token& target ) : sequence( seq ), name( target ) {

		}

	};

	struct expression : sequence {

		template <typename... Tn>
		expression ( Tn&&... argn ) : sequence( std::forward<Tn>( argn )... ) {}

	};

	struct definition : symbol {
		variant<unit,
			std::reference_wrapper<expression>,
			bool,
			uint8, uint16, uint32, uint64,
			int8, int16, int32, int64,
			code_point,
			half, float, double> value;

		template <typename... Tn>
		definition( buffer_view<const token> seq, const token& target, Tn&&... argn ) : symbol( seq, target ), value( std::forward<Tn>( argn )... ) {

		}
	};

	struct function : definition {
		std::vector<std::reference_wrapper<token>> parameters;
		std::vector<std::reference_wrapper<token>> expression_substitutions;

		function( buffer_view<const token> seq, const token& target, expression& expr ) : definition( seq, target, std::ref( expr ) ) {

		}

		std::reference_wrapper<expression> substitution() const {
			return value.get<std::reference_wrapper<expression>>();
		}
	};

	struct binary_expression : expression {
		std::reference_wrapper<expression> left;
		std::reference_wrapper<expression> right;
	};

	struct unary_expression : expression {
		std::reference_wrapper<expression> left;
	};

	struct conditional : expression {
		static const conditional true_value;

		conditional( buffer_view<const token> seq ) : expression( seq ) {

		}

	};

	const conditional conditional::true_value( nullptr );

	struct inclusion : expression {
		const token& target;

		inclusion( buffer_view<const token> seq, const token& name ) : expression( seq ), target( name ) {

		}
	};

	struct scope : sequence {
		optional<const scope&> parent;
		std::reference_wrapper<const conditional> condition;
		std::vector<std::reference_wrapper<sequence>> sequences;

		scope( buffer_view<const token> seq ) : sequence( seq ), parent( none ), condition( conditional::true_value ) {

		}

		scope( buffer_view<const token> seq, const scope& parent, const conditional& cond ) : sequence( seq ), parent( parent ), condition( cond ) {

		}

	};

	struct top_level {
		std::vector<std::unique_ptr<expression>> expressions;
		std::unordered_map<string_view, definition> symbols;
		std::unordered_map<string_view, function> functions;
		std::vector<scope> scopes;
		stack<std::reference_wrapper<scope>> scopestack;

		top_level( buffer_view<const token> tokenview ) {
			scopes.emplace_back( tokenview );
			// Covers the degenerate case in memory where
			// every single token resolves to an expression
			// (not even sure that's possible...?)
			scopes.reserve( tokenview.size() );
			scopestack.emplace( scopes.back() );
		}
	};

}}}
