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
#include "arg_value.hpp"

namespace margs {
	// arithmetic and default
	template<class T>
	struct decoder {
		static bool decode(const arg_value& lhs, T& rhs) {
			// arithmetic
			if constexpr (std::is_arithmetic_v<T>) {
				if (!lhs.isScalar()) return false;

				const std::string& scalar = lhs.as<std::string>();

				// integral
				if constexpr (std::is_integral_v<T>) {
					// signed
					if constexpr (std::is_signed_v<T>) {
						return mstd::strtonum(scalar, rhs);
					}
					// unsigned
					else {
						return mstd::strtounum(scalar, rhs);
					}
				}
				// floating-point
				else {
					return mstd::strtofp(scalar, rhs);
				}
			}
			// default
			else {
				return false;
			}
		}
	};

	// true, 1 | false, 0
	template<>
	struct decoder<bool> {
		static bool decode(const arg_value& lhs, bool& rhs) {
			if (!lhs.isScalar()) return false;

			const std::string& arg_value = lhs.as<std::string>();

			if (arg_value == "true" || arg_value == "1") {
				rhs = true;
			}
			else if (arg_value == "false" || arg_value == "0") {
				rhs = false;
			}
			else {
				return false;
			}

			return true;
		}
	};

	template<class T>
	struct decoder<std::vector<T>> {
		static bool decode(const arg_value& lhs, std::vector<T>& rhs) {
			if (!lhs.isSequence()) return false;

			rhs.clear();
			rhs.reserve(lhs.size());
			for (const arg_value& elem : lhs) {
				rhs.push_back(elem.as<T>());
			}

			return true;
		}
	};
}