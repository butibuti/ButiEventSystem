#include"stdafx.h"
#include "EventSystem.h"

std::vector<std::string> vec_sceneEventMessengerName;
std::map<std::string, std::vector<std::string>>map_sceneEventListner;
bool isGamePlaying = false;
template<typename T,typename U>
inline void ReleaseMapPtr(std::map<T, U*>& arg_map) {
	for (auto itr = arg_map.begin(), end = arg_map.end(); itr != end; itr++) {
		delete itr->second;
	}
	arg_map.clear();
}

struct MessengersInstance {
	std::map<std::string, ButiEventSystem::IEventMessenger*>map_eventMessenger_ex;
	~MessengersInstance() {
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

	if (isGamePlaying) {
		vec_sceneEventMessengerName.push_back(arg_evMessengerName);
	}

	GetMessengersInstance()->map_eventMessenger_ex.emplace(arg_evMessengerName,arg_evMessenger);
}

void ButiEventSystem::RemoveEventMessenger(const std::string& arg_eventMessengerName)
{
	delete GetMessengersInstance()->map_eventMessenger_ex[arg_eventMessengerName];
	GetMessengersInstance()->map_eventMessenger_ex.erase(arg_eventMessengerName);
}

bool ButiEventSystem::ExistEventMessenger(const std::string& arg_eventMessengerName)
{
	return GetMessengersInstance()->map_eventMessenger_ex.count(arg_eventMessengerName);
}


void ButiEventSystem::StartGame()
{
	isGamePlaying = true;
}

void ButiEventSystem::DestroyGameEvent()
{
	isGamePlaying = false;
	for (auto itr = vec_sceneEventMessengerName.begin(), end = vec_sceneEventMessengerName.end(); itr != end; itr++) {
		RemoveEventMessenger(*itr);
	}

	for (auto itr = map_sceneEventListner.begin(), end = map_sceneEventListner.end(); itr != end; itr++) {
		if (GetMessengersInstance()->map_eventMessenger_ex.count(itr->first)) {
			auto messenger = GetMessengersInstance()->map_eventMessenger_ex[itr->first];
			for (auto listnerItr = itr->second.begin(), listnerEnd = itr->second.end(); listnerItr != listnerEnd; listnerItr++) {
				messenger->UnRegist(*listnerItr);
			}
		}
	}

}



void ButiEventSystem::EventListnerRegist(const std::string& arg_messengerName, const std::string& arg_listnerName)
{
	if (!isGamePlaying) {
		return;
	}
	if (!map_sceneEventListner.count(arg_messengerName)) {
		map_sceneEventListner.emplace(arg_messengerName, std::vector<std::string>());
	}
	map_sceneEventListner[arg_messengerName].push_back(arg_listnerName);
	
}
