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

#include "TaskHandler.h"

#include "utils/Panic.h"
#include "utils/debug.h"

namespace filament::backend::fvkutils {

TaskHandler::TaskHandler()
    : mShouldStop(false),
      mThread(&TaskHandler::loop, this) {}

void TaskHandler::post(WorkloadFunc&& workload, OnCompleteFunc&& oncomplete) {
    assert_invariant(!mShouldStop);
    {
        utils::UniqueLock lock(mTaskQueueMutex);
        mTaskQueue.push(std::make_pair(std::move(workload), std::move(oncomplete)));
    }
    mHasTaskCondition.notify_one();
}

void TaskHandler::drain() {
    assert_invariant(!mShouldStop);

    utils::Mutex syncPointMutex;
    utils::Condition syncCondition;
    bool done = false;
    post([] {},
            [&syncPointMutex, &syncCondition, &done] {
                {
                    utils::UniqueLock lock(syncPointMutex);
                    done = true;
                    syncCondition.notify_one();
                }
            });

    utils::UniqueLock lock(syncPointMutex);
    syncCondition.wait(lock, [&done] { return done; });
}

void TaskHandler::shutdown() {
    {
        utils::UniqueLock lock(mTaskQueueMutex);
        mShouldStop = true;
    }
    mHasTaskCondition.notify_one();
    mThread.join();
    FILAMENT_CHECK_POSTCONDITION(mTaskQueue.empty())
            << "TaskHandler has tasks in the queue after shutdown";
}

void TaskHandler::loop() {
    while (true) {
        utils::UniqueLock lock(mTaskQueueMutex);
        mHasTaskCondition.wait(lock, [this] { return !mTaskQueue.empty() || mShouldStop; });
        if (mShouldStop) {
            break;
        }
        auto [workload, oncomplete] = mTaskQueue.front();
        mTaskQueue.pop();
        lock.unlock();
        workload();
        oncomplete();
    }

    // Clean-up: we still need to call oncomplete for clients to do clean-up.
    while (true) {
        utils::UniqueLock lock(mTaskQueueMutex);
        if (mTaskQueue.empty()) {
            break;
        }
        auto [workload, oncomplete] = mTaskQueue.front();
        mTaskQueue.pop();
        lock.unlock();
        oncomplete();
    }
}
} // namespace filament::backend::fvkutils
