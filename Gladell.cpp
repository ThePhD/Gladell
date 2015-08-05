#include "range.hpp"
#include "hlsl/shaders/source.hpp"
#include "hlsl/pp/lex.hpp"
#include "hlsl/pp/parse.hpp"
#include <jsonpp/jsonpp.hpp>
#include <fstream>

inline json::value to_json( gld::uintz n ) {
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

inline json::value to_json( const gld::hlsl::token_value& t ) {
	using namespace gld;
	using gld::hlsl::token_value;
	switch ( t.class_index() ) {
	case token_value::index<bool>::value:
		return json::value( t.unsafe_get<bool>() );
	case token_value::index<int8>::value:
		return json::value( t.unsafe_get<int8>() );
	case token_value::index<int16>::value:
		return json::value( t.unsafe_get<int16>() );
	case token_value::index<int32>::value:
		return json::value( t.unsafe_get<int32>() );
	case token_value::index<int64>::value:
		return json::value( static_cast<double>( t.unsafe_get<int64>() ) );
	case token_value::index<uint8>::value:
		return json::value( t.unsafe_get<uint8>() );
	case token_value::index<uint16>::value:
		return json::value( t.unsafe_get<uint16>() );
	case token_value::index<uint32>::value:
		return json::value( t.unsafe_get<uint32>() );
	case token_value::index<uint64>::value:
		return json::value( static_cast<double>( t.unsafe_get<uint64>() ) );
	case token_value::index<float>::value:
		return json::value( t.unsafe_get<float>() );
	case token_value::index<double>::value:
		return json::value( t.unsafe_get<double>() );
	case token_value::index<string>::value:
		return json::value( string_view( t.unsafe_get<string>() ) );
	case token_value::index<inclusion_style>::value:
		return json::value( to_string( t.unsafe_get<inclusion_style>() ) );
	case token_value::index<code_point>::value:
		return json::value( t.unsafe_get<code_point>() );
	case token_value::index<unit>::value:
	default:
		break;
	}
	return json::null();
}

inline json::value to_json( const gld::hlsl::token& t ) {
	gld::string_view id = to_string( t.id );
	if ( t.value.is<gld::unit>() ) {
		json::object v( {
			{ "id", id },
			{ "lexeme", t.lexeme },
			{ "where", t.where }
		} );
		return v;
	}
	else {
		json::object v( {
			{ "id", id },
			{ "lexeme", t.lexeme },
			{ "where", t.where },
			{ "value", t.value }
		} );
		return v;
	}
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

void lex_print( gld::string name, gld::string_view source ) {
	auto tokens = gld::hlsl::pp::lex( name, source );
	json_print( name + ".pp.lex.json", source, tokens );
	gld::hlsl::pp::parse_tree tree = gld::hlsl::pp::parse( tokens );

}

int main( int argc, char* argv[] ) {
	using namespace Furrovine::tmp;
	using string = Furrovine::string;
	using string_view = Furrovine::string_view;
	std::vector<string_view> arguments(argv, argv + argc);

	lex_print( "fluff", gld::hlsl::shaders::fluff::pre_processing );
	//lex_print( "nymphbatch.json", gld::hlsl::shaders::sm40_level_93::nymph_batch );
}