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

#include <backend/platforms/PlatformGlfwGL.h>
#include "../gl_headers.h"
#include <utils/Logger.h>
#include <utils/Panic.h>
namespace filament::backend {

using namespace backend;

Driver* PlatformGlfwGL::createDriver(void* sharedGLContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    int result = bluegl::bind();
    FILAMENT_CHECK_POSTCONDITION(!result) << "Unable to load OpenGL entry points.";
    return OpenGLPlatform::createDefaultDriver(this, sharedGLContext, driverConfig);
}

int PlatformGlfwGL::getOSVersion() const noexcept {
    return 0;
}

void PlatformGlfwGL::terminate() noexcept {
}

Platform::SwapChain* PlatformGlfwGL::createSwapChain(
        void* nativeWindow, uint64_t flags) noexcept {
    return (SwapChain*)nativeWindow;
}

Platform::SwapChain* PlatformGlfwGL::createSwapChain(
        uint32_t width, uint32_t height, uint64_t flags) noexcept {
    // TODO: implement headless SwapChain
    return nullptr;
}

void PlatformGlfwGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
}

bool PlatformGlfwGL::makeCurrent(ContextType type, SwapChain* drawSwapChain,
        SwapChain* readSwapChain) {
    return true;
}

void PlatformGlfwGL::commit(Platform::SwapChain* swapChain) noexcept {
}

} // namespace filament::backend
