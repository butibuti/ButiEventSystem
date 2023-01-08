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
/// �^�X�N�̃C���^�[�t�F�[�X
/// </summary>
class ITask {
public:
    virtual ~ITask() {}
    /// <summary>
    /// �^�X�N�̎��s
    /// </summary>
    virtual void Run() = 0;
};
/// <summary>
/// �^�X�N�I�u�W�F�N�g
/// </summary>
/// <typeparam name="T">�Ԃ�l�̌^</typeparam>
template < typename T>
class Task :public ITask {
public:
    /// <summary>
    /// �R���X�g���N�^
    /// </summary>
    /// <param name="r">�Ԃ�l�pPromise�I�u�W�F�N�g</param>
    /// <param name="arg_func">�^�X�N(�R�[���o�b�N�֐�)</param>
    Task(std::promise<T> r, std::function<T()> arg_func) {
        returnPromise = std::move(r);
        func = arg_func;
    }
    /// <summary>
    /// �^�X�N���s
    /// </summary>
    void Run()override final {
        returnPromise.set_value(func());
    }
private:
    std::promise<T> returnPromise;
    std::function<T()> func;
};
/// <summary>
/// �^�X�N�I�u�W�F�N�g(�Ԃ�l����)
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
/// �X���b�h�Z�[�t�ȃT�C�Y�������L���[
/// </summary>
/// <typeparam name="T">�����^</typeparam>
template <typename T>
class LockedQueue {
public:
    LockedQueue(const std::int32_t arg_size) :Size(arg_size) {}
    LockedQueue() :Size(0) {}
    /// <summary>
    /// �ő�T�C�Y�̐ݒ�
    /// </summary>
    /// <param name="arg_size">�ݒ�T�C�Y</param>
    void Resize(const std::int32_t arg_size) { Size = arg_size; }
    /// <summary>
    /// �v�f�𖖔��ɒǉ�����
    /// 
    /// �ő�T�C�Y�𒴂���ꍇPop�����܂őҋ@
    /// </summary>
    /// <param name="value">�v�f</param>
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
    /// �v�f��擪����擾
    /// 
    /// ���g�������ꍇPush�����܂őҋ@
    /// </summary>
    /// <returns>�擪�̗v�f</returns>
    T Pop() {

        std::unique_lock<std::mutex> lock_pop(mutex_pop);


        cv_pop.wait(lock_pop, [&] { return queue_values.size() || isEnd; });
        if (!queue_values.size()) {
            ///�I�����Ă���̂œK���Ȓl��Ԃ�
            return T();
        }

        std::unique_lock<std::mutex> lock_push(mutex_push);
        auto ret = std::move(queue_values.front());
        queue_values.pop();
        cv_push.notify_all();
        return ret;
    }
    /// <summary>
    /// �I������
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
/// �X���b�h�Z�[�t�ȃT�C�Y�������L���[(std::unique_ptr�ŏ��L���𐧌�)
/// </summary>
/// <typeparam name="T">std::unique_ptr�Ő��䂷��^</typeparam>
template <typename T>
class LockedQueue <std::unique_ptr<T>> {
public:
    LockedQueue(const std::int32_t arg_size) :Size(arg_size) {}
    LockedQueue() :Size(0) {}

    /// <summary>
    /// �ő�T�C�Y�̐ݒ�
    /// </summary>
    /// <param name="arg_size">�ݒ�T�C�Y</param>
    void Resize(const std::int32_t arg_size) { Size = arg_size; }

    /// <summary>
    /// �v�f�𖖔��ɒǉ�����
    /// 
    /// �ő�T�C�Y�𒴂���ꍇPop�����܂őҋ@
    /// </summary>
    /// <param name="value">�v�f</param>
    void Push(std::unique_ptr<T> value) {
        std::unique_lock<std::mutex> lock_push(mtx_guard);
        cv_push.wait(lock_push, [&]()-> bool {
            return queue_values.size() < Size ||isEnd;
            });

        if (isEnd) {
            //�I�����Ă���̂Ń��^�[��
            return;
        }
        queue_values.push(std::move(value));
        cv_pop.notify_all();
    }
    /// <summary>
    /// �v�f��擪����擾
    /// 
    /// ���g�������ꍇPush�����܂őҋ@
    /// </summary>
    /// <returns>�擪�̗v�f</returns>
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
    /// �I������
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
/// �^�X�N����p�L���[
/// </summary>
class TaskQueue {
public:
    /// <summary>
    /// �R���X�g���N�^
    /// </summary>
    /// <param name="taskSize">�^�X�N�̍ő�o�^��</param>
    BUTITASKSYSTEM_API TaskQueue(const std::int32_t taskSize = defaultTaskSize);
    /// <summary>
    /// �J�n
    /// </summary>
    /// <param name="threadSize">�g�p����X���b�h��</param>
    BUTITASKSYSTEM_API void Start(const std::int32_t threadSize = defaultThreadSize);
    /// <summary>
    /// ��~
    /// </summary>
    BUTITASKSYSTEM_API void Dispose();
    /// <summary>
    /// �^�X�N�ǉ�
    /// </summary>
    /// <typeparam name="T">�^�X�N�̕Ԃ�l</typeparam>
    /// <param name="func">�^�X�N(�R�[���o�b�N�֐�)</param>
    /// <returns>�Ԃ�l�pfuture�I�u�W�F�N�g</returns>
    template <typename T>
    std::future<T> PushTask(std::function<T()> func) {
        std::promise<T> p;
        auto output = p.get_future();

        GetTaskQueue().Push(std::make_unique<Task<T>>(std::move(p), func));

        return output;
    }
    /// <summary>
    /// �^�X�N�ǉ�(�Ԃ�l�Ȃ�)
    /// </summary>
    /// <param name="func">�^�X�N(�Ԃ�l�����R�[���o�b�N�֐�)</param>
    /// <returns>void��future</returns>
    template <>
    std::future<void> PushTask(std::function<void()> func) {
        std::promise<void> p;
        auto output = p.get_future();

        GetTaskQueue().Push(std::make_unique<Task<void>>(func));

        return output;
    }


private:
    /// <summary>
    /// �e�X���b�h�Ɋ��蓖�Ă鏈��
    /// </summary>
    BUTITASKSYSTEM_API void Process();
    class ImplInstance;
    BUTITASKSYSTEM_API LockedQueue<std::unique_ptr<ITask>>& GetTaskQueue();
    ImplInstance* p_instance;
};
/// <summary>
/// �^�X�N�V�X�e���̏�����
/// </summary>
BUTITASKSYSTEM_API void Initialize();
/// <summary>
/// �^�X�N�V�X�e���̊J�n
/// </summary>
/// <param name="threadSize"></param>
BUTITASKSYSTEM_API void Start(const std::int32_t threadSize = defaultThreadSize);
/// <summary>
/// �^�X�N�V�X�e���̒�~
/// </summary>
BUTITASKSYSTEM_API void Dispose();
/// <summary>
/// �^�X�N�L���[�̃C���X�^���X�擾
/// </summary>
/// <returns></returns>
BUTITASKSYSTEM_API  std::unique_ptr<TaskQueue>& GetTaskQueue();
/// <summary>
/// �^�X�N�̒ǉ�
/// </summary>
/// <typeparam name="T">�^�X�N�̕Ԃ�l</typeparam>
/// <param name="func">�R�[���o�b�N�֐�</param>
/// <returns>�Ԃ�l�pfuture�I�u�W�F�N�g</returns>
template <typename T>
std::future<T> PushTask(std::function<T()> func) {
    return GetTaskQueue()->PushTask(func);
}

}

#endif // !BUTITASKSYSTEM_H