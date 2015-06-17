#include "range.hpp"
#include "hlsl/shaders/source.hpp"
#include "hlsl/preprocessor/lex.hpp"
#include "hlsl/preprocessor/parse.hpp"
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

template <typename T>
inline json::value to_json( const gld::buffer_view<T>& b ) {
	return json::array( b.begin(), b.end() );
}

inline json::value to_json( const std::pair<gld::string_view, gld::buffer_view<gld::hlsl::token>>& p ) {
	json::object v( {
		{ "source", p.first },
		{ "tokens", p.second }
	} );
	return v;
}

void json_print( gld::string_view name, gld::string_view source, gld::buffer_view<gld::hlsl::token> tokens ) {
	std::string data = json::dump_string( std::make_pair( source, tokens ), json::format_options( 5, json::format_options::none ) );
	std::ofstream output( name.c_str() );
	output << data << std::endl;
}

void lex_print( gld::string_view name, gld::string_view source ) {
	auto tokens = gld::hlsl::preprocessor::lex( source );
	json_print( name, source, tokens );
	gld::hlsl::preprocessor::parse( tokens );
}
	
int main( int argc, char* argv[] ) {
	using string = Furrovine::string;
	using string_view = Furrovine::string_view;
	std::vector<string_view> arguments(argv, argv + argc);
	
	lex_print( "pp.fluff.pre_processing.json", gld::hlsl::shaders::fluff::pre_processing );
	//lex_print( "pp.nymphbatch.json", gld::hlsl::shaders::sm40_level_93::nymph_batch );
}