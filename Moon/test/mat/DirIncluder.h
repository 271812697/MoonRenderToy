#pragma once
#include "IncludeCallback.h"//<filamat/>

#include "Path.h"//<utils/>

namespace TEST {

	// Functor callback handler used to resolve includes relative to a root include directory.
	class DirIncluder {
	public:
		void setIncludeDirectory(Path dir) noexcept {
			assert(dir.isDirectory());
			mIncludeDirectory = dir;
		}

		bool operator()(const utils::CString& includedBy, IncludeResult& result);

	private:
		Path mIncludeDirectory;

	};

}