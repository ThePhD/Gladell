#pragma once

#include <Furrovine++/ebco.hpp>

namespace gld {

	template <typename T, typename... Tags>
	struct tagged : public Furrovine::ebco<T> {
	private:
		typedef Furrovine::ebco<T> base;

	public:
		template <typename T0, typename... Tn, 
			typename = std::enable_if_t<!std::is_same<tagged, Furrovine::tmp::unqualified_t<T0>>::value>
		>
		tagged( T0&& arg0, Tn&&... argn ) : base( std::forward<T0>( arg0 ), std::forward<Tn>( argn )... ) { }
		tagged( ) : base( ) { };
		tagged( const tagged& item ) : base( item ) { };
		tagged( tagged&& item ) : base( std::move( item ) ) { };
		tagged& operator=( const tagged& item ) { base::operator=( item ) };
		tagged& operator=( tagged&& item ) { base::operator=( std::move( item ) ) };

		operator T& ( ) {
			return base::get( );
		}

		operator const T& ( ) const {
			return base::get( );
		}
	};

}
