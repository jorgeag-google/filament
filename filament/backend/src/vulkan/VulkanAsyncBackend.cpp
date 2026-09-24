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
VulkanAsyncBackend::VulkanAsyncBackend(const VulkanPlatform* platform, const VulkanContext& context,
    ResourceManager* resourceManager, bool asyncAvailable) {
    if (asyncAvailable) {
        // keep track of our own Semaphore manager for the later Sync work
        mSemaphoreManager = std::make_unique<VulkanSemaphoreManager>(platform->getDevice(), resourceManager);
        // A new queue only accessible by this object (It will live inside commands)
        auto graphicsQueueFamilyIndex = platform->getGraphicsQueueFamilyIndex();
        VkQueue queue;
        bluevk::vkGetDeviceQueue(platform->getDevice(), graphicsQueueFamilyIndex, 0, &queue);

        mAsyncCommands = std::make_unique<VulkanCommands>(
                        platform->getDevice(),
                        queue,
                        graphicsQueueFamilyIndex,
                        platform->getProtectedGraphicsQueue(),
                        platform->getProtectedGraphicsQueueFamilyIndex(),
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

void VulkanAsyncBackend::postUpdateJob(std::function<void(VulkanCommandBuffer&)> job,
    AsyncCallId jobId, DriverBase::AsyncCompletion* completion, JobQueue* queue) {


    auto updateFunc = [this, job, jobId] {
        VulkanCommandBuffer& commands = mAsyncCommands->get();
        job(commands);
        FVK_LOGW << "Async Job " << jobId << " recorded";
    };

    auto onCompleteFunc = [this, jobId, queue, completion]()  {

        queue->push([this, jobId, completion]()  {
            if (!mAsyncCommands->flush(true) ) { FVK_LOGW << "Error on flush";}
            FVK_LOGW << "Async Job " << jobId << " flush executed";
            mAsyncCommands->wait();
            FVK_LOGW << "Async Job " << jobId << " waited to finish";
            completion->schedule(AsyncCallStatus::COMPLETED);
            FVK_LOGW << "Async Job " << jobId << " fired callback";
            delete completion;
        });
        FVK_LOGW << "Async Job " << jobId << " on complete executed";
    };

    startTaskHandler();
    mTaskHandler->post(updateFunc, onCompleteFunc);

}

void VulkanAsyncBackend::startTaskHandler () {
    // We don't create a task handler (start a thread) unless an Async method is called.
    if (!mTaskHandler) {
        mTaskHandler = std::make_unique<fvkutils::TaskHandler>();
    }
}

void VulkanAsyncBackend::grabSyncHandles() {
    // keep track of the semaphores or any other sync primitive from the mCommands
    assert(mAsyncCommands);
}

void VulkanAsyncBackend::gc() {
    assert(mAsyncCommands);
    if (mTaskHandler) {
        mTaskHandler->post([this]() {
            //FVK_LOGW << "VulkanAsyncBackend - Before gc";
            //mResourceManager->print();
            mAsyncCommands->gc(false);
            //FVK_LOGW << "VulkanAsyncBackend - After gc";
            //mResourceManager->print();
            }, [](){});
    }

}

}
