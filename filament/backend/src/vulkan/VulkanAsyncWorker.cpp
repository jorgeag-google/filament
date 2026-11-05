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

#include "VulkanAsyncWorker.h"

namespace filament::backend {
VulkanAsyncWorker::VulkanAsyncWorker(const VulkanPlatform* platform) {
    mAsyncQueue = platform->getAsyncQueue();
}

void VulkanAsyncWorker::postUpdateJob(AsyncCallId jobId, JobQueue* queue) {

}

void VulkanAsyncWorker::startTaskHandler () {
    // We don't create a task handler (start a thread) unless an Async method is called.
    if (!mTaskHandler) {
        mTaskHandler = std::make_unique<fvkutils::TaskHandler>();
    }
}

void VulkanAsyncWorker::terminate() noexcept {
    if (mTaskHandler) {
        mTaskHandler->shutdown();
        mTaskHandler.reset();
    }
}

void VulkanAsyncWorker::runUntilComplete() {
    if (mTaskHandler) {
        mTaskHandler->drain();
    }
}

} // namespace filament::backend
