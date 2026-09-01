/*
* Copyright (C) 2026 The Android Open Source Project
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
#ifndef TNT_FILAMENT_BACKEND_VULKANASYNCBACKEND_H
#define TNT_FILAMENT_BACKEND_VULKANASYNCBACKEND_H

#include "VulkanCommands.h"
#include "VulkanHandles.h"

#include "utils/TaskHandler.h"

namespace filament::backend {

class VulkanAsyncBackend {
public:
    explicit VulkanAsyncBackend(const VulkanPlatform* platform, const VulkanContext& context, fvkmemory::ResourceManager* resourceManager, bool asyncAvailable);

    void createIndexBuffer(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage, CallbackHandler* handler,
        CallbackHandler::Callback callback, void* user, utils::ImmutableCString&& tag);
    void updateIndexBuffer(AsyncCallId jobId, resource_ptr<VulkanIndexBuffer> ib,
        BufferDescriptor&& p, uint32_t byteOffset, CallbackHandler* handler,
        CallbackHandler::Callback const callback, void* user);

    void terminate() noexcept;

    void runUntilComplete();
private:
    std::unique_ptr<VulkanCommands> mCommands = nullptr;
    std::unique_ptr<VulkanSemaphoreManager> mSemaphoreManager = nullptr;
    std::unique_ptr<fvkutils::TaskHandler> mTaskHandler = nullptr;
    void startTaskHandler();
    void grabSyncHandles();
};

}

#endif //TNT_FILAMENT_BACKEND_VULKANASYNCBACKEND_H
