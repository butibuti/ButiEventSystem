#include"stdafx.h"
#include"EventSystem.h"
#include"TaskSystem.h"
//イベント監視スレッド
class CountPerSecond
{
public:
    CountPerSecond()
    {
        isEnd = false;
        ButiEventSystem::AddEventMessenger<int>("Count");
        thread = std::thread([&]() ->void
            {
                auto count = 0;
                while (true)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    if (isEnd)
                    {
                        break;
                    }
                    count++;
                    ButiEventSystem::Execute<int>("Count", count);
                }
            });
    };

    inline void Regist
    (std::shared_ptr<ButiEventSystem::EventListenerRegister<int>> arg_func,const int priority)
    {
        ButiEventSystem::RegistEventListner<int>("Count",arg_func,priority);

    }
    inline void UnRegist
    (std::shared_ptr<ButiEventSystem::EventListenerRegister<int>> arg_func)
    {
        ButiEventSystem::UnRegistEventListner<int>("Count", arg_func);

    }

    void Dispose()
    {
        isEnd = true;
        thread.join();
        delete this;
    }
private:

    std::thread thread;
    bool isEnd;
    ~CountPerSecond()
    {
    };
};

class CountUp {
public:
    CountUp(const std::string& arg_name) {
        name = arg_name;
        rf = ButiEngine::ObjectFactory::Create<ButiEventSystem::EventListenerRegister<int>>([&](int value) ->void {std::cout << name << value << std::endl; });
    }
    void Regist(CountPerSecond* arg_p_sub,const int priority) {

        arg_p_sub->Regist(rf,priority);
    }
    void UnRegist(CountPerSecond* arg_p_sub) {

        arg_p_sub->UnRegist(rf);
    }
private:
    int i = 0;
    std::shared_ptr<ButiEventSystem::EventListenerRegister<int>> rf;
    std::string name;
};


int main() {

    ButiTaskSystem::Start();

    int end = 0;
    int c=0;
    while (true) {
        ButiTaskSystem::PushTask(std::function<void(void)>([=]()-> void{
            for(int i=0;i<1000;i++)
                std::cout<<c<<":" << i << std::endl;
            }
        ));
        c++;
        std::cin >> end;
        if (end == 0) {
            break;
        }
    }

    ButiTaskSystem::Dispose();


	return 0;
}



