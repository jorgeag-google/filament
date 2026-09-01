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
#include "VulkanAsyncBackend.h"
namespace filament::backend {
VulkanAsyncBackend::VulkanAsyncBackend(const VulkanPlatform* platform, const VulkanContext& context, fvkmemory::ResourceManager* resourceManager, bool asyncAvailable) {
    if (asyncAvailable) {
        // Semaphore manager
        mSemaphoreManager = std::make_unique<VulkanSemaphoreManager>(platform->getDevice(), resourceManager);

        auto graphicsQueueFamilyIndex = platform->getGraphicsQueueFamilyIndex();
        auto protectedGraphicsQueueFamilyIndex = platform->getProtectedGraphicsQueueFamilyIndex();
        // A new queue only accesable by this object
        VkQueue queue;
        bluevk::vkGetDeviceQueue(platform->getDevice(), graphicsQueueFamilyIndex, 0, &queue);
        VkQueue protectedQueue;
        bluevk::vkGetDeviceQueue(platform->getDevice(), protectedGraphicsQueueFamilyIndex, 0, &protectedQueue);

        mCommands = std::make_unique<VulkanCommands>(
                        platform->getDevice(),
                        queue,
                        graphicsQueueFamilyIndex,
                        protectedQueue,
                        protectedGraphicsQueueFamilyIndex,
                        context,
                        mSemaphoreManager.get()
                        );
    }
}
void VulkanAsyncBackend::terminate() noexcept{
    if (mTaskHandler) {
        mTaskHandler->shutdown();
        mTaskHandler.reset();
    }
}

void VulkanAsyncBackend::runUntilComplete() {
    if (mTaskHandler) {
        mTaskHandler->drain();
    }
}

void VulkanAsyncBackend::createIndexBuffer(Handle<HwIndexBuffer> ibh, ElementType elementType,
    uint32_t indexCount, BufferUsage usage, CallbackHandler* handler,
    CallbackHandler::Callback callback, void* user, utils::ImmutableCString&& tag) {
    /* Maybe this is not even needed */
}

void VulkanAsyncBackend::updateIndexBuffer(AsyncCallId jobId, resource_ptr<VulkanIndexBuffer> ib,
    BufferDescriptor&& p, uint32_t byteOffset, CallbackHandler* handler,
    CallbackHandler::Callback const callback, void* user) {

    auto createIndexBufferFunc = [this, &ib, &p, byteOffset]() {
        VulkanCommandBuffer& commands = mCommands->get();
        commands.acquire(ib);
        ib->loadFromCpu(commands, p.buffer, byteOffset, p.size);
        // scheduleDestroy(std::move(p));
    };
    auto cleanIndexBufferFunc = [this/*, handler, callback, user*/] () {
        // scheduleCallback(handler, user, callback);
        grabSyncHandles();
    };
    startTaskHandler();
    mTaskHandler->post(createIndexBufferFunc, cleanIndexBufferFunc);
}

void VulkanAsyncBackend::startTaskHandler () {
    // We don't create a task handler (start a thread) unless an Async method is called.
    if (!mTaskHandler) {
        mTaskHandler = std::make_unique<fvkutils::TaskHandler>();
    }
}

void VulkanAsyncBackend::grabSyncHandles() {
    // keep track of the semaphores or any other sync primitive from the mCommands
    assert(mCommands);
}

}
