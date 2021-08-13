#include"stdafx.h"
#include "TaskSystem.h"

namespace ButiTaskSystem {
class TaskQueue::ImplInstance {
    friend class TaskQueue;
    bool isEnd = false;
    LockedQueue<std::unique_ptr<ITask>> lq_task;
    std::vector<std::thread> vec_thread;
};

std::unique_ptr<TaskQueue> unq_taskQueue=nullptr;

void Initialize()
{
    unq_taskQueue = std::make_unique<TaskQueue>(16);
}
void Start(const int threadSize)
{
    GetTaskQueue()->Start();
}
void Dispose()
{
    GetTaskQueue()->Dispose();
}
std::unique_ptr<TaskQueue>& GetTaskQueue()
{
    if (!unq_taskQueue) {
        Initialize();
    }
    return unq_taskQueue;
}
}

ButiTaskSystem::TaskQueue::TaskQueue(const int taskSize)
{
    p_instance = new ImplInstance();
    p_instance ->lq_task.Resize(taskSize);
}

void ButiTaskSystem::TaskQueue::Start(const int threadSize)
{
    if (p_instance->vec_thread.size()) {
        throw ButiEngine::ButiException(L"タスクシステムを複数回起動しています");
        return;
    }
    p_instance->vec_thread.resize(threadSize);

    for (auto itr = p_instance->vec_thread.begin(), end = p_instance->vec_thread.end(); itr != end; itr++) {
        *itr = std::thread([this]()->void {Process(); });
    }
}

void ButiTaskSystem::TaskQueue::Dispose()
{
    if (p_instance->isEnd) {
        throw ButiEngine::ButiException(L"タスクシステムを複数回終了しています");
        return;
    }
    p_instance->isEnd = true;
    p_instance->lq_task.Exit();
    for (auto itr = p_instance->vec_thread.begin(), end = p_instance->vec_thread.end(); itr != end; itr++) {
        itr->join();
    }
    p_instance->vec_thread.clear();
}

void ButiTaskSystem::TaskQueue::Process()
{
    while (true) {
        auto task = p_instance->lq_task.Pop();

        if (task) {
            task->Run();
        }
        else {
            break;
        }

    }
}

ButiTaskSystem::LockedQueue<std::unique_ptr<ButiTaskSystem::ITask>>& ButiTaskSystem::TaskQueue::GetTaskQueue()
{
    return p_instance->lq_task;
}
