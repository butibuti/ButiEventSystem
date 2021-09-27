#ifndef BUTIEVENTSYSTEM_H
#define BUTIEVENTSYSTEM_H

#pragma once

#include<functional>

#define IMPLEVENT( TYPE)\
template<> void ButiEventSystem::AddEventMessenger < TYPE >(const std::string& );\
template<> std::string ButiEventSystem::RegistEventListner < TYPE >(const std::string& , const std::string& , std::function<void( TYPE)> , const bool  , const int );\
template<> void ButiEventSystem::UnRegistEventListner < TYPE >(const std::string& ,const std::string& );\
template<> void ButiEventSystem::Execute < TYPE >(const std::string& ,const TYPE & );\


namespace ButiEventSystem
{
/// <summary>
/// イベントの優先度定数
/// Priority_Min =末尾に追加
/// Priority_Min =先頭に追加
/// </summary>
enum ButiEventPriority {
    Priority_Min = -1, Priority_Max = -2,
};

/// <summary>
/// イベントメッセンジャーのインターフェース
/// </summary>
class IEventMessenger {
public:
    virtual ~IEventMessenger()
    {
    };
};

/// <summary>
/// イベントメッセンジャー
/// </summary>
/// <typeparam name="T">登録イベントの引数の型</typeparam>
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
    /// 所有しているオブジェクトの解放
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
    /// イベント発火
    /// </summary>
    /// <param name="event">イベント引数</param>
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
    /// イベントリスナーの登録
    /// </summary>
    /// <param name="arg_key">イベントリスナーの名前</param>
    /// <param name="arg_func">イベントリスナーの登録用関数オブジェクト</param>
    /// <param name="canDuplicate">同名イベントリスナーを許容するかどうか</param>
    /// <param name="priority">優先度</param>
    inline std::string Regist(const std::string& arg_key, std::function<void(T)> arg_func,const bool canDuplicate, const int priority = ButiEventPriority::Priority_Min)
    {
        auto key = arg_key;
        if (map_funtionIndexPtr.count(key)) {
            //登録済みかつ同名イベントリスナーを許容しないのでreturn
            if (!canDuplicate) {
                return arg_key;
            }
            map_keyDuplicateCount.at(arg_key)+=1;
            key+="_"+ std::to_string(map_keyDuplicateCount.at(arg_key));
        }
        else {
            map_keyDuplicateCount.emplace(arg_key, 0);
        }



        int currentEvPriority = priority == ButiEventPriority::Priority_Max ? currentEvPriority = maxPriority + 1 : priority == ButiEventPriority::Priority_Min ? currentEvPriority = minPriority : priority;



        vec_functionIndex.push_back(new int(vec_functionAndPriority.size()));
        map_funtionIndexPtr.emplace(key, vec_functionIndex.back());
        

        vec_functionAndPriority.push_back({ arg_func,currentEvPriority });


        //優先度が最小、最大の場合は末尾、先頭に追加するのみ
        if (currentEvPriority <= minPriority) {
            minPriority = currentEvPriority;

            if (sorted) {
                vec_functions.push_back(arg_func);
            }

            return key;
        }
        else if (currentEvPriority > maxPriority) {
            maxPriority = currentEvPriority;
            if (sorted) {
                vec_functions.insert(vec_functions.begin(), arg_func);
            }
            return key;
        }

        sorted = false;
        return key;
    }
    /// <summary>
    /// イベントリスナーの登録解除
    /// </summary>
    /// <param name="arg_func">イベントリスナーの登録用オブジェクト</param>
    inline void UnRegist(const std::string& arg_key)
    {
        if (!map_funtionIndexPtr.count(arg_key)) {
            return;
        }


        int index = *map_funtionIndexPtr.at(arg_key);

        vec_functionAndPriority.erase(vec_functionAndPriority.begin() + index);
        map_funtionIndexPtr.erase(arg_key);
        delete vec_functionIndex.at(index);

        for (auto indexItr = vec_functionIndex.erase(vec_functionIndex.begin() + index), end = vec_functionIndex.end(); indexItr != end; indexItr++) {
            *(*indexItr) -= 1;
        }
        sorted = false;
    }

private:
    /// <summary>
    /// 優先度順にイベントリスナーをソートする
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
    std::map<std::string, int*> map_funtionIndexPtr;
    std::map<std::string, int> map_keyDuplicateCount;
    std::map<int, int*> map_priority;
    int maxPriority = 0, minPriority = 0;
    bool sorted = true;
};
/// <summary>
/// イベントメッセンジャー(引数無し)
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
    /// 所有しているオブジェクトの解放
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
    /// イベント発火
    /// </summary>
    /// <param name="event">イベント引数</param>
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
    /// イベントリスナーの登録
    /// </summary>
    /// <param name="arg_func">イベントリスナーの登録用オブジェクト</param>
    /// <param name="priority">優先度</param>
    inline std::string Regist(const std::string& arg_key, std::function<void(void)> arg_func, const bool canDuplicate, const int priority = ButiEventPriority::Priority_Min)
    {
        auto key = arg_key;
        if (map_funtionIndexPtr.count(key)) {
            //登録済みかつ同名イベントリスナーを許容しないのでreturn
            if (!canDuplicate) {
                return arg_key;
            }
            map_keyDuplicateCount.at(arg_key) += 1;
            key += "_" + std::to_string(map_keyDuplicateCount.at(arg_key));
        }
        else {
            map_keyDuplicateCount.emplace(arg_key, 0);
        }
        int currentEvPriority = priority == ButiEventPriority::Priority_Max ? currentEvPriority = maxPriority + 1 : priority == ButiEventPriority::Priority_Min ? currentEvPriority = minPriority : priority;



        vec_functionIndex.push_back(new int(vec_functionAndPriority.size()));
        map_funtionIndexPtr.emplace(key, vec_functionIndex.back());

        vec_functionAndPriority.push_back({ arg_func,currentEvPriority });


        //優先度が最小、最大の場合は末尾、先頭に追加するのみ
        if (currentEvPriority <= minPriority) {
            minPriority = currentEvPriority;

            if (sorted) {
                vec_functions.push_back(arg_func);
            }

            return key;
        }
        else if (currentEvPriority > maxPriority) {
            maxPriority = currentEvPriority;
            if (sorted) {
                vec_functions.insert(vec_functions.begin(), arg_func);
            }
            return key;
        }
        
        sorted = false;
        return key;
    }

    /// <summary>
    /// イベントリスナーの登録解除
    /// </summary>
    /// <param name="arg_func">イベントリスナーの登録用オブジェクト</param>
    inline void UnRegist(const std::string& arg_key)
    {
        if (!map_funtionIndexPtr.count(arg_key)) {
            return;
        }


        int index = *map_funtionIndexPtr.at(arg_key);

        vec_functionAndPriority.erase(vec_functionAndPriority.begin() + index);
        map_funtionIndexPtr.erase(arg_key);
        delete vec_functionIndex.at(index);

        for (auto indexItr = vec_functionIndex.erase(vec_functionIndex.begin() + index), end = vec_functionIndex.end(); indexItr != end; indexItr++) {
            *(*indexItr) -= 1;
        }
        sorted = false;
    }

private:

    /// <summary>
    /// 優先度順にイベントリスナーをソートする
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
    std::map<std::string, int*> map_funtionIndexPtr;
    std::map<std::string, int> map_keyDuplicateCount;
    std::map<int, int*> map_priority;
    int maxPriority = 0, minPriority = 0;
    bool sorted = true;
};
/// <summary>
/// イベントメッセンジャーの取得
/// </summary>
/// <param name="arg_evMessengerName">イベント名</param>
/// <returns>イベントメッセンジャー</returns>
extern ButiEventSystem::IEventMessenger* GetExEventMessenger(const std::string& arg_evMessengerName);
/// <summary>
/// イベントメッセンジャーの追加
/// </summary>
/// <param name="arg_evMessengerName">イベント名</param>
/// <param name="arg_evMessenger">イベントメッセンジャー</param>
extern void AddExEventMessenger(const std::string& arg_evMessengerName,IEventMessenger* arg_evMessenger);
/// <summary>
/// イベントメッセンジャーの追加
/// </summary>
/// <typeparam name="T">追加するイベントメッセンジャーの引数型</typeparam>
/// <param name="arg_eventMessengerName">イベント名</param>
template <typename T>
extern void AddEventMessenger(const std::string& arg_eventMessengerName) 
{
    AddExEventMessenger(arg_eventMessengerName, new EventMessenger<T>());
}
/// <summary>
/// イベントリスナーの追加
/// </summary>
/// <typeparam name="T">追加するイベントリスナーの型</typeparam>
/// <param name="arg_eventMessengerName">イベント名</param>
/// <param name="arg_eventMessengerName">イベントリスナー名</param>
/// <param name="arg_func">追加するイベントリスナー(コールバック関数)登録用関数オブジェクト</param>
/// <param name="canDuplicate">同名イベントリスナーを許容するかどうか</param>
/// <param name="priority">優先度</param>
template <typename T>
std::string RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(T)> arg_func, const bool canDuplicate, const int priority = ButiEventPriority::Priority_Min)
{
    auto p_m = GetExEventMessenger(arg_eventMessengerName);
    if (p_m) {
        return ((EventMessenger<T>*)p_m)->Regist(arg_key, arg_func, canDuplicate, priority);
    }
    return arg_key;
}
std::string RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(void)> arg_func, const bool canDuplicate, const int priority = ButiEventPriority::Priority_Min);
/// <summary>
/// イベントリスナーの登録解除
/// </summary>
/// <typeparam name="T">削除するイベントリスナーの型</typeparam>
/// <param name="arg_eventMessengerName">イベント名</param>
/// <param name="arg_func">削除するイベントリスナー(コールバック関数)登録用オブジェクト</param>
template <typename T>
void UnRegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key) {
    auto p_m = GetExEventMessenger(arg_eventMessengerName);
    if (p_m) {
        ((EventMessenger<T>*)p_m)->UnRegist(arg_key);
    }
}
/// <summary>
/// イベント発火
/// </summary>
/// <typeparam name="T">イベントの型</typeparam>
/// <param name="arg_eventMessengerName">イベント名</param>
/// <param name="arg_event">イベントの実体</param>
template <typename T>
void Execute(const std::string& arg_eventMessengerName, const T& arg_event) {
    auto p_m = GetExEventMessenger(arg_eventMessengerName);
    if (p_m) {
        ((EventMessenger<T>*)p_m)->Execute(arg_event);
    }
}
/// <summary>
/// イベント発火(void)
/// </summary>
/// <param name="arg_eventMessengerName">イベント名</param>
extern void Execute(const std::string& arg_eventMessengerName);

template<> void ButiEventSystem::AddEventMessenger < void >(const std::string&); 
template<> void ButiEventSystem::UnRegistEventListner < void >(const std::string&, const std::string& ); 

using PV = void*;

IMPLEVENT(int)
IMPLEVENT(float)
IMPLEVENT(std::string)
IMPLEVENT(PV)
}
#undef IMPLEVENT
#endif // !BUTIEVENTSYSTEM_H