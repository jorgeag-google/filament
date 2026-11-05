/*
* Copyright (C) 2021 The Android Open Source Project
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
#ifndef TNT_VULKANASYNCWORKER_H
#define TNT_VULKANASYNCWORKER_H

#include "JobQueue.h"
#include "backend/platforms/VulkanPlatform.h"
#include "vulkan/utils/TaskHandler.h"

namespace filament::backend {
class VulkanAsyncWorker {
public:
    explicit VulkanAsyncWorker(const VulkanPlatform* platform);

    void postUpdateJob(AsyncCallId jobId, JobQueue* queue);
    void terminate() noexcept;

    void runUntilComplete();
private:
    VkQueue mAsyncQueue = VK_NULL_HANDLE;
    std::unique_ptr<fvkutils::TaskHandler> mTaskHandler = nullptr;
    void startTaskHandler();

};
} // namespace filament::backend

#endif // TNT_VULKANASYNCWORKER_H
