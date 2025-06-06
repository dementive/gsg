#pragma once

#include "core/variant/variant.h"

namespace CSV {
// Parses a csv file into an Array of Arrays, each sub array is the values on each line.
// If the types of the values are ints or floats they will be automatically converted to the correct type.
Vector<Vector<Variant>> parse_file(const String &p_file_name);
} // namespace CSV
