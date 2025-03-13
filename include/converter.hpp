#pragma once
#include "margs_types.hpp"
#include "arg_value.hpp"

namespace margs {
	// arithmetic and default
	template<class T>
	struct converter {
		static arg_value encode(const T& rhs) {
			if constexpr (std::is_arithmetic_v<T>) {
				if constexpr (std::is_same_v<T, char> && std::is_unsigned_v<char>) {
					return arg_value(std::to_string((unsigned int)rhs));
				}
				else {
					return arg_value(std::to_string(rhs));
				}
			}
			// default
			else {
				return arg_value();
			}
		}

		static bool decode(const arg_value& lhs, T& rhs) {
			// arithmetic
			if constexpr (std::is_arithmetic_v<T>) {
				if (!lhs.isScalar()) return false;

				const std::string& scalar = lhs.as<std::string>();

				// integral
				if constexpr (std::is_integral_v<T>) {
					// signed
					if constexpr (std::is_signed_v<T>) {
						return ston(scalar, rhs);
					}
					// unsigned
					else {
						return stoun(scalar, rhs);
					}
				}
				// floating-point
				else {
					return stofp(scalar, rhs);
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
	struct converter<bool> {
		static arg_value encode(const bool& rhs) {
			return arg_value(rhs ? "true" : "false");
		}

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

	// null
	template<>
	struct converter<nullptr_t> {
		static arg_value encode(const nullptr_t& rhs) {
			return arg_value();
		}

		static bool decode(const arg_value& lhs, nullptr_t& rhs) {
			if (lhs.isNull()) {
				rhs = nullptr;
				return true;
			}
			
			if (lhs.isScalar()) {
				if (lhs.as<std::string>() == "null") {
					rhs = nullptr;
					return true;
				}
			}

			return false;
		}
	};

	//// uintptr_t
	//template<class T>
	//struct converter<T*> {
	//	static arg_value encode(const T*& value) {
	//		return arg_value(std::to_string(reinterpret_cast<uintptr_t>(value)));
	//	}

	//	static bool decode(const arg_value& arg, T*& value) {
	//		if (arg.isNull()) {
	//			value = nullptr;
	//			return true;
	//		}

	//		if (arg.isScalar()) {
	//			uintptr_t ptr;
	//			stoun(arg.as<std::string>(), ptr);
	//			value = reinterpret_cast<T*>(ptr);
	//			return true;
	//		}

	//		return false;
	//	}
	//};

	template<class T>
	struct converter<std::vector<T>> {
		static arg_value encode(const std::vector<T>& rhs) {
			arg_value& lhs = arg_value();

			for (const T& elem : rhs) {
				lhs.push_back(elem);
			}

			return lhs;
		}

		static bool decode(const arg_value& lhs, std::vector<T>& rhs) {
			if (!lhs.isSequence()) return false;

			rhs.clear();
			for (const arg_value& elem : lhs) {
				rhs.push_back(elem.as<T>());
			}

			return true;
		}
	};

	template<class T>
	struct converter<std::initializer_list<T>> {
		static arg_value encode(const std::initializer_list<T>& rhs) {
			arg_value& lhs = arg_value();

			for (const T& elem : rhs) {
				lhs.push_back(elem);
			}

			return lhs;
		}

		static bool decode(const arg_value& lhs, std::initializer_list<T>& rhs) {
			return false;
		}
	};
}