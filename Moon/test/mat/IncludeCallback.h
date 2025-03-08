#pragma once

#include "test/utils/CString.h"//<utils/CString.h>

#include <functional>

namespace TEST {

	struct IncludeResult {
		// The include name of the root file, as if it were being included.
		// I.e., 'foobar.h' in the case of #include "foobar.h"
		const utils::CString includeName;

		// The following fields should be filled out by the IncludeCallback when processing an include,
		// or when calling resolveIncludes for the root file.

		// The full contents of the include file. This may contain additional, recursive include
		// directives.
		utils::CString text;

		// The line number for the first line of text (first line is 0).
		size_t lineNumberOffset = 0;

		// The name of the include file. This gets passed as "includerName" for any includes inside of
		// source. This field isn't used by the include system; it's up to the callback to give meaning
		// to this value and interpret it accordingly.  In the case of DirIncluder, this is an empty
		// string to represent the root include file, and a canonical path for subsequent included
		// files.
		utils::CString name;
	};

	/**
	 * A callback invoked by the include system when an #include "file.h" directive is found.
	 *
	 * For example, if a file main.h includes file.h on line 10, then IncludeCallback would be called
	 * with the following:
	 *     includeCallback("main.h", {.includeName = "file.h" })
	 * It's then up to the IncludeCallback to fill out the .text, .name, and (optionally)
	 * lineNumberOffset fields.
	 *
	 * @param includedBy is the value that was given to IncludeResult.name for this source file, or
	 *        the empty string for the root source file.
	 * @param result is the IncludeResult that the callback should fill out.
	 * @return true, if the include was resolved successfully, false otherwise.
	 *
	 * For an example of implementing this callback, see tools/matc/src/matc/DirIncluder.h.
	 */
	using IncludeCallback = std::function<bool(
		const utils::CString& includedBy,
		IncludeResult& result)>;

} // namespace filamat

