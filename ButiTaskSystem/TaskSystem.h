#pragma once
#ifndef BUTITASKSYSTEM_H
#define BUTITASKSYSTEM_H

#ifdef BUTITASKSYSTEM_EXPORTS
#define BUTITASKSYSTEM_API __declspec(dllexport)
#else
#define BUTITASKSYSTEM_API __declspec(dllimport)
#endif
#include<mutex>
#include<future>
#include<queue>
namespace ButiTaskSystem {

constexpr std::int32_t defaultThreadSize = 0b10000;
constexpr std::int32_t defaultTaskSize = 0b1000000;

/// <summary>
/// タスクのインターフェース
/// </summary>
class ITask {
public:
    virtual ~ITask() {}
    /// <summary>
    /// タスクの実行
    /// </summary>
    virtual void Run() = 0;
};
/// <summary>
/// タスクオブジェクト
/// </summary>
/// <typeparam name="T">返り値の型</typeparam>
template < typename T>
class Task :public ITask {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="r">返り値用Promiseオブジェクト</param>
    /// <param name="arg_func">タスク(コールバック関数)</param>
    Task(std::promise<T> r, std::function<T()> arg_func) {
        returnPromise = std::move(r);
        func = arg_func;
    }
    /// <summary>
    /// タスク実行
    /// </summary>
    void Run()override final {
        returnPromise.set_value(func());
    }
private:
    std::promise<T> returnPromise;
    std::function<T()> func;
};
/// <summary>
/// タスクオブジェクト(返り値無し)
/// </summary>
template <>
class Task<void> :public ITask {
public:

    Task(std::function<void()> arg_func) {
        func = arg_func;
    }
    void Run()override final {
        func();
    }
private:
    std::function<void()> func;
};
/// <summary>
/// スレッドセーフなサイズ制限つきキュー
/// </summary>
/// <typeparam name="T">扱う型</typeparam>
template <typename T>
class LockedQueue {
public:
    LockedQueue(const std::int32_t arg_size) :Size(arg_size) {}
    LockedQueue() :Size(0) {}
    /// <summary>
    /// 最大サイズの設定
    /// </summary>
    /// <param name="arg_size">設定サイズ</param>
    void Resize(const std::int32_t arg_size) { Size = arg_size; }
    /// <summary>
    /// 要素を末尾に追加する
    /// 
    /// 最大サイズを超える場合Popされるまで待機
    /// </summary>
    /// <param name="value">要素</param>
    void Push(T value) {
        std::unique_lock<std::mutex> lock_push(mutex_push);
        cv_push.wait(lock_push, [&] {
            return queue_values.size() < Size;
            });

        std::unique_lock<std::mutex> lock_pop(mutex_pop);
        queue_values.push(value);
        cv_pop.notify_all();
    }
    /// <summary>
    /// 要素を先頭から取得
    /// 
    /// 中身が無い場合Pushされるまで待機
    /// </summary>
    /// <returns>先頭の要素</returns>
    T Pop() {

        std::unique_lock<std::mutex> lock_pop(mutex_pop);


        cv_pop.wait(lock_pop, [&] { return queue_values.size() || isEnd; });
        if (!queue_values.size()) {
            ///終了しているので適当な値を返す
            return T();
        }

        std::unique_lock<std::mutex> lock_push(mutex_push);
        auto ret = std::move(queue_values.front());
        queue_values.pop();
        cv_push.notify_all();
        return ret;
    }
    /// <summary>
    /// 終了処理
    /// </summary>
    void Exit() {
        isEnd = true;
        cv_pop.notify_all();
    }
private:
    std::mutex mutex_push, mutex_pop;
    std::condition_variable cv_push, cv_pop;
    std::queue<T> queue_values;
    std::int32_t Size;
    bool isEnd=false;
};
/// <summary>
/// スレッドセーフなサイズ制限つきキュー(std::unique_ptrで所有権を制限)
/// </summary>
/// <typeparam name="T">std::unique_ptrで制御する型</typeparam>
template <typename T>
class LockedQueue <std::unique_ptr<T>> {
public:
    LockedQueue(const std::int32_t arg_size) :Size(arg_size) {}
    LockedQueue() :Size(0) {}

    /// <summary>
    /// 最大サイズの設定
    /// </summary>
    /// <param name="arg_size">設定サイズ</param>
    void Resize(const std::int32_t arg_size) { Size = arg_size; }

    /// <summary>
    /// 要素を末尾に追加する
    /// 
    /// 最大サイズを超える場合Popされるまで待機
    /// </summary>
    /// <param name="value">要素</param>
    void Push(std::unique_ptr<T> value) {
        std::unique_lock<std::mutex> lock_push(mtx_guard);
        cv_push.wait(lock_push, [&]()-> bool {
            return queue_values.size() < Size ||isEnd;
            });

        if (isEnd) {
            //終了しているのでリターン
            return;
        }
        queue_values.push(std::move(value));
        cv_pop.notify_all();
    }
    /// <summary>
    /// 要素を先頭から取得
    /// 
    /// 中身が無い場合Pushされるまで待機
    /// </summary>
    /// <returns>先頭の要素</returns>
    std::unique_ptr<T> Pop() {
        std::unique_lock<std::mutex> lock_pop(mtx_guard);
        cv_pop.wait(lock_pop, [&]()-> bool {
            return queue_values.size() || isEnd; });

        if (isEnd) {
            return nullptr;
        }

        auto ret = std::move(queue_values.front());
        queue_values.pop();
        cv_push.notify_all();
        return std::move(ret);
    }
    /// <summary>
    /// 終了処理
    /// </summary>
    void Exit() {
        isEnd = true;
        cv_pop.notify_all();
        cv_push.notify_all();
    }
private:
    std::mutex mtx_guard;
    std::condition_variable cv_push, cv_pop;
    std::queue<std::unique_ptr <T>> queue_values;
    std::int32_t Size=0;
    bool isEnd = false;
};

/// <summary>
/// タスク制御用キュー
/// </summary>
class TaskQueue {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="taskSize">タスクの最大登録数</param>
    BUTITASKSYSTEM_API TaskQueue(const std::int32_t taskSize = defaultTaskSize);
    /// <summary>
    /// 開始
    /// </summary>
    /// <param name="threadSize">使用するスレッド数</param>
    BUTITASKSYSTEM_API void Start(const std::int32_t threadSize = defaultThreadSize);
    /// <summary>
    /// 停止
    /// </summary>
    BUTITASKSYSTEM_API void Dispose();
    /// <summary>
    /// タスク追加
    /// </summary>
    /// <typeparam name="T">タスクの返り値</typeparam>
    /// <param name="func">タスク(コールバック関数)</param>
    /// <returns>返り値用futureオブジェクト</returns>
    template <typename T>
    std::future<T> PushTask(std::function<T()> func) {
        std::promise<T> p;
        auto output = p.get_future();

        GetTaskQueue().Push(std::make_unique<Task<T>>(std::move(p), func));

        return output;
    }
    /// <summary>
    /// タスク追加(返り値なし)
    /// </summary>
    /// <param name="func">タスク(返り値無しコールバック関数)</param>
    /// <returns>voidのfuture</returns>
    template <>
    std::future<void> PushTask(std::function<void()> func) {
        std::promise<void> p;
        auto output = p.get_future();

        GetTaskQueue().Push(std::make_unique<Task<void>>(func));

        return output;
    }


private:
    /// <summary>
    /// 各スレッドに割り当てる処理
    /// </summary>
    BUTITASKSYSTEM_API void Process();
    class ImplInstance;
    BUTITASKSYSTEM_API LockedQueue<std::unique_ptr<ITask>>& GetTaskQueue();
    ImplInstance* p_instance;
};
/// <summary>
/// タスクシステムの初期化
/// </summary>
BUTITASKSYSTEM_API void Initialize();
/// <summary>
/// タスクシステムの開始
/// </summary>
/// <param name="threadSize"></param>
BUTITASKSYSTEM_API void Start(const std::int32_t threadSize = defaultThreadSize);
/// <summary>
/// タスクシステムの停止
/// </summary>
BUTITASKSYSTEM_API void Dispose();
/// <summary>
/// タスクキューのインスタンス取得
/// </summary>
/// <returns></returns>
BUTITASKSYSTEM_API  std::unique_ptr<TaskQueue>& GetTaskQueue();
/// <summary>
/// タスクの追加
/// </summary>
/// <typeparam name="T">タスクの返り値</typeparam>
/// <param name="func">コールバック関数</param>
/// <returns>返り値用futureオブジェクト</returns>
template <typename T>
std::future<T> PushTask(std::function<T()> func) {
    return GetTaskQueue()->PushTask(func);
}

}

#endif // !BUTITASKSYSTEM_H