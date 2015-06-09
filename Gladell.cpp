#include "hlsl/shaders/source.hpp"
#include "hlsl/lexer.hpp"
#include "hlsl/preprocessor/lexer.hpp"
#include <jsonpp/jsonpp.hpp>
#include <fstream>

inline json::value to_json( gld::intuz n ) {
	return static_cast<double>( n );
}

inline json::value to_json( gld::intz n ) {
	return static_cast<double>( n );
}

inline json::value to_json( const gld::string_view& s ) {
	return json::value( s.data(), s.data_end() );
}

inline json::value to_json( const gld::hlsl::occurrence& o ) {
	json::object v( {
		{ "line", o.line },
		{ "column", o.column },
		{ "offset", o.offset } 
	} );
	return v;
}

inline json::value to_json( const gld::hlsl::token& t ) {
	gld::string_view id = to_string( t.id );
	json::object v( { 
		{ "id", id },
		{ "lexeme", t.lexeme },
		{ "where", t.where }
	} );
	return v;
}

template <typename Range>
void json_print( Range&& tokens ) {
	std::string data = json::dump_string( tokens, json::format_options( 5, json::format_options::none ) );
	std::ofstream output( "tokens.json" );
	output << data << std::endl;
}

int main( int argc, char* argv[] ) {
	using string = Furrovine::string;
	using string_view = Furrovine::string_view;
	std::vector<string_view> arguments(argv, argv + argc);

	string_view src =
#if 0
		gld::hlsl::shaders::sm40_level_93::nymph_batch;
#else
		gld::hlsl::shaders::fluff::pre_processing;
#endif
	gld::hlsl::preprocessor::lexer lexpp(src);
	auto tokens = lexpp();
	json_print( tokens );
}
