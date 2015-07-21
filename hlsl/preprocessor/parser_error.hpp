#pragma once

#include "../string.hpp"
#include <exception>

namespace gld { namespace hlsl { namespace preprocessor {

	struct parser_error : public std::exception {
		string message;
		occurrence where;

		parser_error( occurrence where = {}, string message = "parsing failed" ) : where( where ), message(message) {

		}

		virtual const char* what() const override {
			return message.c_str();
		}
	};

}}}
