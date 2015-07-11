#pragma once

#include <Furrovine++/ebco.hpp>
#include <Furrovine++/identity_function.hpp>
#include <memory>

namespace Furrovine {

	template <typename T, typename Transform = identity_function, typename Alloc = std::allocator<T>>
	struct recursive_type : private ebco<Alloc>, private ebco<Transform> {
	private:
		typedef ebco<Transform> transform;
		typedef ebco<Alloc> alloc;
		T* p;
		
	public:

		template <typename Tn>
		recursive_type( Tn&&... argn ) : alloc(), p( alloc::get().allocate( 1 ) ) {
			alloc::get().construct( p, std::forward<Tn>( argn )... );
		}

		T& get() {
			return const_cast<T&>(const_cast<const recursive_type&>(*this).get());
		}

		const T& get() const {
			return *transform::get()( p );
		}

		~recursive_type() {
			alloc::get().destroy( p );
			alloc::get().deallocate( p, 1 );
		}
	};

}
