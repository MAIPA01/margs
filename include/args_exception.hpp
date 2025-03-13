#pragma once
#include "margs_types.hpp"

namespace margs {
	class args_exception : public std::runtime_error {
	public:
		args_exception(const std::string& message) : std::runtime_error(message.c_str()) {}
	};
}