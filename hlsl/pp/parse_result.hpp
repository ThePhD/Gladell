#pragma once

#include "../../variant.hpp"
#include "parser_error.hpp"

namespace gld { namespace hlsl { namespace pp {

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

		parse_result() : result() {

		}

		template <typename T0, typename... Tn, typename = std::enable_if_t<
			!std::is_same<Furrovine::tmp::unqualified_t<T0>, parse_result>::value 
			&& !std::is_same<Furrovine::tmp::unqualified_t<T0>, parser_error>::value
		>>
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

		template <typename T>
		parse_result( const parse_result<T>& r ) : parse_result( r.valid( ) ? parse_result( r.get( ) ) : parse_result( r.exception() ) ) { 
		
		}
		
		template <typename T>
		parse_result& operator=( const parse_result<T>& r ) {
			if ( r.valid( ) ) {
				operator=( r.get( ) );
			}
			else {
				operator=( r.exception( ) );
			}
		}

		template <typename T>
		parse_result( parse_result<T>&& r ) : parse_result( r.valid( ) ? parse_result( std::move( r.get( ) ) ) : parse_result( std::move( r.exception() ) ) ) {
		
		}

		template <typename T>
		parse_result& operator=( parse_result<T>&& r ) {
			if ( r.valid( ) ) {
				operator=( std::move( r.get( ) ) );
			}
			else {
				operator=( std::move( r.exception( ) ) );
			}
		}

		bool valid() const {
			return result.is<T>();
		}

		explicit operator bool() const {
			return valid();
		}

		operator T& () {
			return get();
		}

		operator const T& () const {
			return get();
		}

		T& get() {
			return result.visit( visitor() );
		}

		const T& get() const {
			return result.visit( visitor() );
		}

		parser_error& exception() {
			return result.get<parser_error>();
		}

		const parser_error& exception() const {
			return result.get<parser_error>();
		}
	};

}}}
