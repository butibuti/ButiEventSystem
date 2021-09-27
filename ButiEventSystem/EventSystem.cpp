#include"stdafx.h"
#include "EventSystem.h"

template<typename T,typename U>
inline void ReleaseMapPtr(std::map<T, U*>& arg_map) {
	for (auto itr = arg_map.begin(), end = arg_map.end(); itr != end; itr++) {
		delete itr->second;
	}
	arg_map.clear();
}

struct MessengersInstance {
	std::map<std::string, ButiEventSystem::EventMessenger<int>*> map_eventMessenger_int;
	std::map<std::string, ButiEventSystem::EventMessenger<float>*> map_eventMessenger_float;
	std::map<std::string, ButiEventSystem::EventMessenger<std::string>*> map_eventMessenger_str;
	std::map<std::string, ButiEventSystem::EventMessenger<void*>*> map_eventMessenger_p_void;
	std::map<std::string, ButiEventSystem::EventMessenger<void>*> map_eventMessenger_void;
	std::map<std::string, ButiEventSystem::IEventMessenger*>map_eventMessenger_ex;
	~MessengersInstance() {
		ReleaseMapPtr(map_eventMessenger_int);
		ReleaseMapPtr(map_eventMessenger_float);
		ReleaseMapPtr(map_eventMessenger_str);
		ReleaseMapPtr(map_eventMessenger_p_void);
		ReleaseMapPtr(map_eventMessenger_void);
		ReleaseMapPtr(map_eventMessenger_ex);
	}

};
MessengersInstance* p_messengersInstance;

MessengersInstance* GetMessengersInstance() {
	if (!p_messengersInstance) {
		p_messengersInstance = new MessengersInstance();
	}
	return p_messengersInstance;
}

struct MessengersRelease
{
	~MessengersRelease() {
		if (p_messengersInstance) {
			delete p_messengersInstance;
		}
	}
};
MessengersRelease releaser;

ButiEventSystem::IEventMessenger* ButiEventSystem::GetExEventMessenger(const std::string& arg_evMessengerName)
{
	if (!GetMessengersInstance()->map_eventMessenger_ex.count(arg_evMessengerName)) {
		return nullptr;
	}
	return GetMessengersInstance()->map_eventMessenger_ex.at(arg_evMessengerName);
}


void ButiEventSystem::AddExEventMessenger(const std::string& arg_evMessengerName,IEventMessenger* arg_evMessenger)
{
	if (GetMessengersInstance()->map_eventMessenger_ex.count(arg_evMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_ex.emplace(arg_evMessengerName,arg_evMessenger);
}

template<>
void ButiEventSystem::AddEventMessenger <int>(const std::string& arg_eventMessengerName)
{
	if (GetMessengersInstance()->map_eventMessenger_int.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_int.emplace(arg_eventMessengerName, new ButiEventSystem::EventMessenger<int>());
}
template<>
void ButiEventSystem::AddEventMessenger <float>(const std::string& arg_eventMessengerName)
{
	if (GetMessengersInstance()->map_eventMessenger_float.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_float.emplace(arg_eventMessengerName, new ButiEventSystem::EventMessenger<float>());
}
template<>
void ButiEventSystem::AddEventMessenger <std::string>(const std::string& arg_eventMessengerName)
{
	if (GetMessengersInstance()->map_eventMessenger_str.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_str.emplace(arg_eventMessengerName, new ButiEventSystem::EventMessenger<std::string>());
}
template<>
void ButiEventSystem::AddEventMessenger <void*>(const std::string& arg_eventMessengerName)
{
	if (GetMessengersInstance()->map_eventMessenger_p_void.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_p_void.emplace(arg_eventMessengerName, new ButiEventSystem::EventMessenger<void*>());
}
template<>
void ButiEventSystem::AddEventMessenger <void>(const std::string& arg_eventMessengerName)
{
	if (GetMessengersInstance()->map_eventMessenger_void.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_void.emplace(arg_eventMessengerName, new ButiEventSystem::EventMessenger<void>());
}

template<>
std::string ButiEventSystem::RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(int)> arg_func, const bool canDuplicate, const int priority)
{
	if (!GetMessengersInstance()->map_eventMessenger_int.count(arg_eventMessengerName)) {
		return arg_key;
	}
	return GetMessengersInstance()->map_eventMessenger_int.at(arg_eventMessengerName)->Regist(arg_key, arg_func,canDuplicate, priority);
}
template<>
std::string ButiEventSystem::RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(float)> arg_func, const bool canDuplicate, const int priority)
{
	if (!GetMessengersInstance()->map_eventMessenger_float.count(arg_eventMessengerName)) {
		return arg_key;
	}
	return GetMessengersInstance()->map_eventMessenger_float.at(arg_eventMessengerName)->Regist(arg_key, arg_func, canDuplicate, priority);
}
template<>
std::string ButiEventSystem::RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(std::string)> arg_func, const bool canDuplicate, const int priority)
{
	if (!GetMessengersInstance()->map_eventMessenger_str.count(arg_eventMessengerName)) {
		return arg_key;
	}
	return GetMessengersInstance()->map_eventMessenger_str.at(arg_eventMessengerName)->Regist(arg_key, arg_func, canDuplicate, priority);
}
template<>
std::string ButiEventSystem::RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(void*)> arg_func, const bool canDuplicate, const int priority)
{
	if (!GetMessengersInstance()->map_eventMessenger_p_void.count(arg_eventMessengerName)) {
		return arg_key;
	}
	return GetMessengersInstance()->map_eventMessenger_p_void.at(arg_eventMessengerName)->Regist(arg_key, arg_func, canDuplicate, priority);
}
std::string ButiEventSystem::RegistEventListner(const std::string& arg_eventMessengerName, const std::string& arg_key, std::function<void(void)> arg_func, const bool canDuplicate, const int priority)
{
	if (!GetMessengersInstance()->map_eventMessenger_void.count(arg_eventMessengerName)) {
		return arg_key;
	}
	return GetMessengersInstance()->map_eventMessenger_void.at(arg_eventMessengerName)->Regist(arg_key, arg_func, canDuplicate, priority);
}

template<>
void ButiEventSystem::UnRegistEventListner<int>(const std::string& arg_eventMessengerName, const std::string & arg_key)
{
	if (!GetMessengersInstance()->map_eventMessenger_int.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_int.at(arg_eventMessengerName)->UnRegist(arg_key);
}
template<>
void ButiEventSystem::UnRegistEventListner<float>(const std::string& arg_eventMessengerName, const std::string& arg_key)
{
	if (!GetMessengersInstance()->map_eventMessenger_float.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_float.at(arg_eventMessengerName)->UnRegist(arg_key);
}
template<>
void ButiEventSystem::UnRegistEventListner<std::string>(const std::string& arg_eventMessengerName, const std::string& arg_key)
{
	if (!GetMessengersInstance()->map_eventMessenger_str.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_str.at(arg_eventMessengerName)->UnRegist(arg_key);
}
template<>
void ButiEventSystem::UnRegistEventListner<void*>(const std::string& arg_eventMessengerName, const std::string& arg_key)
{
	if (!GetMessengersInstance()->map_eventMessenger_p_void.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_p_void.at(arg_eventMessengerName)->UnRegist(arg_key);
}
template<>
void ButiEventSystem::UnRegistEventListner<void>(const std::string& arg_eventMessengerName, const std::string& arg_key)
{
	if (!GetMessengersInstance()->map_eventMessenger_void.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_void.at(arg_eventMessengerName)->UnRegist(arg_key);
}

using PV = void*;
template<>
void ButiEventSystem::Execute(const std::string& arg_eventMessengerName, const int& arg_event)
{
	if (!GetMessengersInstance()->map_eventMessenger_int.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_int.at(arg_eventMessengerName)->Execute(arg_event);
}
template<>
void ButiEventSystem::Execute(const std::string& arg_eventMessengerName, const float& arg_event)
{
	if (!GetMessengersInstance()->map_eventMessenger_float.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_float.at(arg_eventMessengerName)->Execute(arg_event);
}
template<>
void ButiEventSystem::Execute(const std::string& arg_eventMessengerName, const std::string& arg_event)
{
	if (!GetMessengersInstance()->map_eventMessenger_str.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_str.at(arg_eventMessengerName)->Execute(arg_event);
}
template<>
void ButiEventSystem::Execute(const std::string& arg_eventMessengerName, const PV& arg_event)
{
	if (!GetMessengersInstance()->map_eventMessenger_p_void.count(arg_eventMessengerName)) {
		return;
	}
	GetMessengersInstance()->map_eventMessenger_p_void.at(arg_eventMessengerName)->Execute(arg_event);
}
void ButiEventSystem::Execute(const std::string& arg_eventMessengerName)
{
	if (!GetMessengersInstance()->map_eventMessenger_void.count(arg_eventMessengerName)) {
		return;
	}
	auto p_m = GetMessengersInstance()->map_eventMessenger_void.at(arg_eventMessengerName);

	p_m->Execute();
}

template void ButiEventSystem::AddEventMessenger<int>(const std::string& );
template void ButiEventSystem::AddEventMessenger<float>(const std::string& );
template void ButiEventSystem::AddEventMessenger<std::string>(const std::string& );
template void ButiEventSystem::AddEventMessenger<void*>(const std::string&);
template void ButiEventSystem::AddEventMessenger<void>(const std::string& );

template std::string ButiEventSystem::RegistEventListner<int>			(const std::string&, const std::string&, std::function<void(int)>, const bool, const int);
template std::string ButiEventSystem::RegistEventListner<float>		(const std::string&, const std::string&, std::function<void(float)>, const bool, const int);
template std::string ButiEventSystem::RegistEventListner<std::string>	(const std::string&, const std::string&, std::function<void(std::string)>, const bool, const int);
template std::string ButiEventSystem::RegistEventListner<void*>(const std::string&, const std::string&, std::function<void(void*)>, const bool, const int);

template void ButiEventSystem::UnRegistEventListner<int>		(const std::string&,const std::string& );
template void ButiEventSystem::UnRegistEventListner<float>		(const std::string&, const std::string&);
template void ButiEventSystem::UnRegistEventListner<std::string>(const std::string&, const std::string&);
template void ButiEventSystem::UnRegistEventListner<void*>(const std::string&, const std::string&);
template void ButiEventSystem::UnRegistEventListner<void>		(const std::string&, const std::string&);

template void ButiEventSystem::Execute<int>			(const std::string&,const int&);
template void ButiEventSystem::Execute<float>		(const std::string&,const float&);
template void ButiEventSystem::Execute<std::string>	(const std::string&,const std::string&);
template void ButiEventSystem::Execute<void*>		(const std::string&,const PV&);