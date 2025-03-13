/*
 * margs - Maipa's Args Analyzer
 *
 * Licensed under the BSD 3-Clause License with Attribution Requirement.
 * See the LICENSE file for details: https://github.com/MAIPA01/margs/blob/main/LICENSE
 *
 * Copyright (c) 2025, Patryk Antosik (MAIPA01)
 */

#pragma once
#pragma region VERSION
#define MARGS_VERSION_MAJOR 1
#define MARGS_VERSION_MINOR 0
#define MARGS_VERSION_PATCH 0

#define MARGS_VERSION_TO_STRING(major, minor, patch) #major #minor #patch
#define MARGS_VERSION_TO_INT(major, minor, patch) (major * 100 + minor * 10 + patch)

#define MARGS_VERSION_STRING MARGS_VERSION_TO_STRING(MARGS_VERSION_MAJOR, \
												MARGS_VERSION_MINOR, \
												MARGS_VERSION_PATCH)
#define MARGS_VERSION_INT MARGS_VERSION_TO_INT(MARGS_VERSION_MAJOR, \
											MARGS_VERSION_MINOR, \
											MARGS_VERSION_PATCH)
#define MARGS_VERSION MARGS_VERSION_STRING
#pragma endregion

#pragma region LAST_UPDATE
#define MARGS_LAST_UPDATE_DAY 13
#define MARGS_LAST_UPDATE_MONTH 3
#define MARGS_LAST_UPDATE_YEAR 2025

#define MARGS_LAST_UPDATE_DATE "13.03.2025"
#pragma endregion