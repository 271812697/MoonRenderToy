/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <private/backend/PlatformFactory.h>

#include <utils/Systrace.h>
#include <utils/debug.h>
#include "backend/platforms/PlatformWGL.h"



#include "noop/PlatformNoop.h"

namespace filament::backend {

	// Creates the platform-specific Platform object. The caller takes ownership and is
	// responsible for destroying it. Initialization of the backend API is deferred until
	// createDriver(). The passed-in backend hint is replaced with the resolved backend.
	Platform* PlatformFactory::create(Backend* backend) noexcept {
		SYSTRACE_CALL();
		assert_invariant(backend);

		assert_invariant(*backend == Backend::OPENGL);
		return new PlatformWGL();
	}

	// destroys a Platform created by create()
	void PlatformFactory::destroy(Platform** platform) noexcept {
		delete* platform;
		*platform = nullptr;
	}

} // namespace filament::backend
