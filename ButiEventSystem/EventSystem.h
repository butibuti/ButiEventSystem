#ifndef BUTIEVENTSYSTEM_H
#define BUTIEVENTSYSTEM_H

#pragma once

#include<functional>

#define IMPLEVENT( TYPE)\
template<> void ButiEventSystem::AddEventMessenger < TYPE >(const std::string& );\
template<> void ButiEventSystem::RegistEventListner < TYPE >(const std::string& ,std::shared_ptr<EventListenerRegister< TYPE >> , const int );\
template<> void ButiEventSystem::UnRegistEventListner < TYPE >(const std::string& ,std::shared_ptr<EventListenerRegister< TYPE >> );\
template<> void ButiEventSystem::Execute < TYPE >(const std::string& ,const TYPE & );\


namespace ButiEventSystem
{
/// <summary>
/// �C�x���g�̗D��x�萔
/// Priority_Min =�����ɒǉ�
/// Priority_Min =�擪�ɒǉ�
/// </summary>
enum ButiEventPriority {
    Priority_Min = -1, Priority_Max = -2,
};

/// <summary>
/// �C�x���g���X�i�[�̓o�^�p�C���X�^���X
/// </summary>
/// <typeparam name="T">�����̌^</typeparam>
template<typename T>
class EventListenerRegister :public ButiEngine::IObject {
public:
    /// <summary>
    /// 
    /// </summary>
    /// <param name="arg_func">�C�x���g���X�i�[(�R�[���o�b�N�֐�)</param>
    EventListenerRegister(std::function<void(T)>arg_func) { func = arg_func;  p_key = malloc(1); }
    ~EventListenerRegister() {
        delete p_key;
    }
    /// <summary>
    /// �Ǘ����Ă���֐��̎擾
    /// </summary>
    /// <returns></returns>
    std::function<void(T)>GetFunc() { return func; }
    /// <summary>
    /// �o�^�p�L�[�̎擾
    /// </summary>
    /// <returns></returns>
    long long int GetKey()const {
        if (!p_key) {
            return -1;
        }

        return *(long long int*) & p_key;
    }
private:
    std::function<void(T)>func;
    void* p_key;
};
/// <summary>
/// �C�x���g���b�Z���W���[�̃C���^�[�t�F�[�X
/// </summary>
class IEventMessenger {
public:
    virtual ~IEventMessenger()
    {
    };
};

/// <summary>
/// �C�x���g���b�Z���W���[
/// </summary>
/// <typeparam name="T">�o�^�C�x���g�̈����̌^</typeparam>
template <typename T>
class EventMessenger :public IEventMessenger
{
public:
    EventMessenger() {};

    ~EventMessenger()
    {
        Clear();
    };
    /// <summary>
    /// ���L���Ă���I�u�W�F�N�g�̉��
    /// </summary>
    inline void Clear() {
        vec_functions.clear();
        vec_functionAndPriority.clear();
        for (auto itr = vec_functionIndex.begin(), end = vec_functionIndex.end(); itr != end; itr++) {
            delete* itr;
        }
        vec_functionIndex.clear();
        map_funtionIndexPtr.clear();
        minPriority = 0;
        maxPriority = 0;
    }
    /// <summary>
    /// �C�x���g����
    /// </summary>
    /// <param name="event">�C�x���g����</param>
    inline void Execute(const T& event)
    {
        if (!sorted) {
            CreateSortedVec();
        }
        for (auto function : vec_functions)
        {
            function(event);
        }
    }
    /// <summary>
    /// �C�x���g���X�i�[�̓o�^
    /// </summary>
    /// <param name="arg_func">�C�x���g���X�i�[�̓o�^�p�I�u�W�F�N�g</param>
    /// <param name="priority">�D��x</param>
    inline void Regist(std::shared_ptr<EventListenerRegister<T>> arg_func, const int priority = ButiEventPriority::Priority_Min)
    {
        long long int key = arg_func->GetKey();
        if (key < 0) {
            throw ButiEngine::ButiException(L"���̂̂Ȃ��R�[���o�b�N�֐���o�^���悤�Ƃ��Ă��܂�");
        }
        if (map_funtionIndexPtr.count(key)) {
            return;
        }
        int currentEvPriority = priority == ButiEventPriority::Priority_Max ? currentEvPriority = maxPriority + 1 : priority == ButiEventPriority::Priority_Min ? currentEvPriority = minPriority : priority;



        vec_functionIndex.push_back(new int(vec_functionAndPriority.size()));
        map_funtionIndexPtr.emplace(key, vec_functionIndex.back());

        vec_functionAndPriority.push_back({ arg_func->GetFunc(),currentEvPriority });


        //�D��x���ŏ��A�ő�̏ꍇ�͖����A�擪�ɒǉ�����̂�
        if (currentEvPriority <= minPriority) {
            minPriority = currentEvPriority;

            if (sorted) {
                vec_functions.push_back(arg_func->GetFunc());
            }

            return;
        }
        else if (currentEvPriority > maxPriority) {
            maxPriority = currentEvPriority;
            if (sorted) {
                vec_functions.insert(vec_functions.begin(), arg_func->GetFunc());
            }
            return;
        }

        sorted = false;
    }
    /// <summary>
    /// �C�x���g���X�i�[�̓o�^����
    /// </summary>
    /// <param name="arg_func">�C�x���g���X�i�[�̓o�^�p�I�u�W�F�N�g</param>
    inline void UnRegist(std::shared_ptr<EventListenerRegister<T>> arg_func)
    {
        auto key = arg_func->GetKey();
        if (!map_funtionIndexPtr.count(key)) {
            return;
        }


        int index = *map_funtionIndexPtr.at(key);

        vec_functionAndPriority.erase(vec_functionAndPriority.begin() + index);
        map_funtionIndexPtr.erase(key);
        delete vec_functionIndex.at(index);

        for (auto indexItr = vec_functionIndex.erase(vec_functionIndex.begin() + index), end = vec_functionIndex.end(); indexItr != end; indexItr++) {
            *(*indexItr) -= 1;
        }
        sorted = false;
    }

private:
    /// <summary>
    /// �D��x���ɃC�x���g���X�i�[���\�[�g����
    /// </summary>
    inline void CreateSortedVec() {

        vec_functions.clear();

        std::vector<std::pair< std::function<void(T)>, int>> copied_vec_functionAndPriority = vec_functionAndPriority;

        std::sort(copied_vec_functionAndPriority.begin(), copied_vec_functionAndPriority.end(),
            [](std::pair< std::function<void(T)>, int> x, std::pair< std::function<void(T)>, int> y)->bool {return x.second > y.second; });

        vec_functions.reserve(copied_vec_functionAndPriority.size());
        for (auto itr = copied_vec_functionAndPriority.begin(), end = copied_vec_functionAndPriority.end(); itr != end; itr++) {
            vec_functions.push_back(itr->first);
        }

        sorted = true;
    }

    std::vector<std::pair< std::function<void(T)>, int>> vec_functionAndPriority;
    std::vector<std::function<void(T)>> vec_functions;
    std::vector<int*> vec_functionIndex;
    std::map<long long int, int*> map_funtionIndexPtr;
    std::map<int, int*> map_priority;
    int maxPriority = 0, minPriority = 0;
    bool sorted = true;
};
/// <summary>
/// �C�x���g���b�Z���W���[(��������)
/// </summary>
template <>
class EventMessenger <void>:public IEventMessenger
{
public:
    EventMessenger() {};

    ~EventMessenger()
    {
        Clear();
    };

    /// <summary>
    /// ���L���Ă���I�u�W�F�N�g�̉��
    /// </summary>
    inline void Clear() {
        vec_functions.clear();
        vec_functionAndPriority.clear();
        for (auto itr = vec_functionIndex.begin(), end = vec_functionIndex.end(); itr != end; itr++) {
            delete* itr;
        }
        vec_functionIndex.clear();
        map_funtionIndexPtr.clear();
        minPriority = 0;
        maxPriority = 0;
    }

    /// <summary>
    /// �C�x���g����
    /// </summary>
    /// <param name="event">�C�x���g����</param>
    inline void Execute()
    {
        if (!sorted) {
            CreateSortedVec();
        }
        for (auto function : vec_functions)
        {
            function();
        }
    }

    /// <summary>
    /// �C�x���g���X�i�[�̓o�^
    /// </summary>
    /// <param name="arg_func">�C�x���g���X�i�[�̓o�^�p�I�u�W�F�N�g</param>
    /// <param name="priority">�D��x</param>
    inline void Regist(std::shared_ptr<EventListenerRegister<void>> arg_func, const int priority = ButiEventPriority::Priority_Min)
    {
        long long int key = arg_func->GetKey();
        if (key < 0) {
            throw ButiEngine::ButiException(L"���̂̂Ȃ��R�[���o�b�N�֐���o�^���悤�Ƃ��Ă��܂�");
        }
        if (map_funtionIndexPtr.count(key)) {
            return;
        }
        int currentEvPriority = priority == ButiEventPriority::Priority_Max ? currentEvPriority = maxPriority + 1 : priority == ButiEventPriority::Priority_Min ? currentEvPriority = minPriority : priority;



        vec_functionIndex.push_back(new int(vec_functionAndPriority.size()));
        map_funtionIndexPtr.emplace(key, vec_functionIndex.back());

        vec_functionAndPriority.push_back({ arg_func->GetFunc(),currentEvPriority });


        //�D��x���ŏ��A�ő�̏ꍇ�͖����A�擪�ɒǉ�����̂�
        if (currentEvPriority <= minPriority) {
            minPriority = currentEvPriority;

            if (sorted) {
                vec_functions.push_back(arg_func->GetFunc());
            }

            return;
        }
        else if (currentEvPriority > maxPriority) {
            maxPriority = currentEvPriority;
            if (sorted) {
                vec_functions.insert(vec_functions.begin(), arg_func->GetFunc());
            }
            return;
        }

        sorted = false;
    }

    /// <summary>
    /// �C�x���g���X�i�[�̓o�^����
    /// </summary>
    /// <param name="arg_func">�C�x���g���X�i�[�̓o�^�p�I�u�W�F�N�g</param>
    inline void UnRegist(std::shared_ptr<EventListenerRegister<void>> arg_func)
    {
        auto key = arg_func->GetKey();
        if (!map_funtionIndexPtr.count(key)) {
            return;
        }


        int index = *map_funtionIndexPtr.at(key);

        vec_functionAndPriority.erase(vec_functionAndPriority.begin() + index);
        map_funtionIndexPtr.erase(key);
        delete vec_functionIndex.at(index);

        for (auto indexItr = vec_functionIndex.erase(vec_functionIndex.begin() + index), end = vec_functionIndex.end(); indexItr != end; indexItr++) {
            *(*indexItr) -= 1;
        }
        sorted = false;
    }

private:

    /// <summary>
    /// �D��x���ɃC�x���g���X�i�[���\�[�g����
    /// </summary>
    inline void CreateSortedVec() {

        vec_functions.clear();

        std::vector<std::pair< std::function<void()>, int>> copied_vec_functionAndPriority = vec_functionAndPriority;

        std::sort(copied_vec_functionAndPriority.begin(), copied_vec_functionAndPriority.end(),
            [](std::pair< std::function<void()>, int> x, std::pair< std::function<void()>, int> y)->bool {return x.second > y.second; });

        vec_functions.reserve(copied_vec_functionAndPriority.size());
        for (auto itr = copied_vec_functionAndPriority.begin(), end = copied_vec_functionAndPriority.end(); itr != end; itr++) {
            vec_functions.push_back(itr->first);
        }

        sorted = true;
    }

    std::vector<std::pair< std::function<void()>, int>> vec_functionAndPriority;
    std::vector<std::function<void()>> vec_functions;
    std::vector<int*> vec_functionIndex;
    std::map<long long int, int*> map_funtionIndexPtr;
    std::map<int, int*> map_priority;
    int maxPriority = 0, minPriority = 0;
    bool sorted = true;
};
/// <summary>
/// �C�x���g���b�Z���W���[�̎擾
/// </summary>
/// <param name="arg_evMessengerName">�C�x���g��</param>
/// <returns>�C�x���g���b�Z���W���[</returns>
extern ButiEventSystem::IEventMessenger* GetExEventMessenger(const std::string& arg_evMessengerName);
/// <summary>
/// �C�x���g���b�Z���W���[�̒ǉ�
/// </summary>
/// <param name="arg_evMessengerName">�C�x���g��</param>
/// <param name="arg_evMessenger">�C�x���g���b�Z���W���[</param>
extern void AddExEventMessenger(const std::string& arg_evMessengerName,IEventMessenger* arg_evMessenger);
/// <summary>
/// �C�x���g���b�Z���W���[�̒ǉ�
/// </summary>
/// <typeparam name="T">�ǉ�����C�x���g���b�Z���W���[�̈����^</typeparam>
/// <param name="arg_eventMessengerName">�C�x���g��</param>
template <typename T>
extern void AddEventMessenger(const std::string& arg_eventMessengerName) 
{
    AddExEventMessenger(arg_eventMessengerName, new EventMessenger<T>());
}
/// <summary>
/// �C�x���g���X�i�[�̒ǉ�
/// </summary>
/// <typeparam name="T">�ǉ�����C�x���g���X�i�[�̌^</typeparam>
/// <param name="arg_eventMessengerName">�C�x���g��</param>
/// <param name="arg_func">�ǉ�����C�x���g���X�i�[(�R�[���o�b�N�֐�)�o�^�p�I�u�W�F�N�g</param>
/// <param name="priority"></param>
template <typename T>
void RegistEventListner(const std::string& arg_eventMessengerName, std::shared_ptr<EventListenerRegister<T>> arg_func, const int priority = ButiEventPriority::Priority_Min)
{
    auto p_m = GetExEventMessenger(arg_eventMessengerName);
    if (p_m) {
        ((EventMessenger<T>*)p_m)->Regist(arg_func, priority);
    }
}
/// <summary>
/// �C�x���g���X�i�[�̓o�^����
/// </summary>
/// <typeparam name="T">�폜����C�x���g���X�i�[�̌^</typeparam>
/// <param name="arg_eventMessengerName">�C�x���g��</param>
/// <param name="arg_func">�폜����C�x���g���X�i�[(�R�[���o�b�N�֐�)�o�^�p�I�u�W�F�N�g</param>
template <typename T>
void UnRegistEventListner(const std::string& arg_eventMessengerName, std::shared_ptr<EventListenerRegister<T>> arg_func) {
    auto p_m = GetExEventMessenger(arg_eventMessengerName);
    if (p_m) {
        ((EventMessenger<T>*)p_m)->UnRegist(arg_func);
    }
}
/// <summary>
/// �C�x���g����
/// </summary>
/// <typeparam name="T">�C�x���g�̌^</typeparam>
/// <param name="arg_eventMessengerName">�C�x���g��</param>
/// <param name="arg_event">�C�x���g�̎���</param>
template <typename T>
void Execute(const std::string& arg_eventMessengerName, const T& arg_event) {
    auto p_m = GetExEventMessenger(arg_eventMessengerName);
    if (p_m) {
        ((EventMessenger<T>*)p_m)->Execute(arg_event);
    }
}
/// <summary>
/// �C�x���g����(void)
/// </summary>
/// <param name="arg_eventMessengerName">�C�x���g��</param>
extern void Execute(const std::string& arg_eventMessengerName);

template<> void ButiEventSystem::AddEventMessenger < void >(const std::string&); 
template<> void ButiEventSystem::RegistEventListner < void >(const std::string&, std::shared_ptr<EventListenerRegister< void >>, const int); 
template<> void ButiEventSystem::UnRegistEventListner < void >(const std::string&, std::shared_ptr<EventListenerRegister< void >>); 
using PV = void*;

IMPLEVENT(int)
IMPLEVENT(float)
IMPLEVENT(std::string)
IMPLEVENT(PV)
}
#undef IMPLEVENT
#endif // !BUTIEVENTSYSTEM_H