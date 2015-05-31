#include "hlsl/shaders/source.hpp"
#include "hlsl/lexer.hpp"
#include "hlsl/preprocessor/lexer.hpp"

int main( int argc, char* argv[] ) {
	using string = Furrovine::string;
	using string_view = Furrovine::string_view;
	std::vector<string_view> arguments(argv, argv + argc);

	string_view src = gld::hlsl::shaders::sm40_level_93::nymph_batch;

	gld::hlsl::preprocessor::lexer lexpp(src);
	auto tokens = lexpp();
	gld::hlsl::lexer lex;
	lex(src);
}
