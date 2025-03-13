/*
 * margs - Maipa's Args Analyzer
 *
 * Licensed under the BSD 3-Clause License with Attribution Requirement.
 * See the LICENSE file for details: https://github.com/MAIPA01/margs/blob/main/LICENSE
 *
 * Copyright (c) 2025, Patryk Antosik (MAIPA01)
 */

#pragma once
#include "margs_types.hpp"

namespace margs {
	class args_exception : public std::runtime_error {
	public:
		args_exception(const std::string& message) : std::runtime_error(message.c_str()) {}
	};
}