// Win64Coroutine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <functional>
#include <iostream>
#include <vector>

using namespace std;

const size_t DEFAULT_STACK_SIZE = 1024 * 1024 * 2;
const size_t MAX_THREADS = 4;

extern "C" {
    void _switch(void* old_ctx, void* new_ctx);
}

enum class State {
    Available,
    Running,
    Ready,
};

struct ThreadContext {
    void* rsp;
    void* r15, * r14, * r13, * r12;
    void* rbx;
    void* rbp;
    void* rcx;
    //void* rip;
};

using Job = std::function<void()>;

class Thread {
public:
    size_t id;
    vector<char> stack;
    ThreadContext ctx{};
    State state = State::Available;
    Job job;
    Thread(size_t id_) : id(id_) {
        stack.resize(DEFAULT_STACK_SIZE);
    }
};

typedef void(*F)();

class Runtime;
Runtime* RUNTIME;
void guard();
void _exec(Thread* t) {
    t->job();
}

class Runtime {
public:
    vector<Thread> threads;
    size_t current = 0;

    Runtime() {
        threads.reserve(MAX_THREADS);
        threads.emplace_back(0);    // 占位
        threads[0].state = State::Running;
        for (size_t i = 1; i < MAX_THREADS; i++)
        {
            threads.emplace_back(i);
        }
    }

    void init() {
        RUNTIME = this;
    }

    int run() {
        while (t_yield()) {
        }
        return 0;
    }

    void t_return() {
        if (current)
        {
            threads[current].state = State::Available;
            t_yield();
        }
        else {
            assert(false);
        }
    }

    bool t_yield() {
        size_t pos = current;
        while (threads[pos].state != State::Ready) {
            if (++pos == threads.size()) {
                pos = 0;
            }
            if (pos == current)
            {
                return false;   // 结束
            }
        }
        if (threads[current].state != State::Available) {
            threads[current].state = State::Ready;
        }
        threads[pos].state = State::Running;
        size_t old_pos = current;
        current = pos;
        _switch(&threads[old_pos].ctx, &threads[pos].ctx);
        return threads.size();
    }

    void spawn(Job && f) {
        auto available = find_if(threads.begin(), threads.end(), [](Thread const& t) {return t.state == State::Available; });
        if (available == threads.end())
        {
            return;
        }
        available->job = f;
        void** stack = (void**)(available->stack.data() + available->stack.size());
        // stack[-1] = &*available;
        stack[-3] = guard;
        stack[-4] = _exec;  // f return的时候，就把guard弹入 rip
        available->ctx.rsp = stack - 4;
        available->ctx.rbp = stack - 3;
        available->ctx.rcx = &*available;   // _exec 的参数
        //available->ctx.rip = f;
        available->state = State::Ready;
    }
};

void guard() {
    RUNTIME->t_return();
}

void yield_thread() {
    RUNTIME->t_yield();
}

void func1() {
    cout << "THREAD 1 STARTING" << endl;
    size_t id = 1;
    for (size_t i = 0; i < 10; i++)
    {
        cout << "thread: " << id << " counter: " << i << endl;
        yield_thread();
    }
    cout << "THREAD 1 FINISHED" << endl;
}

void func2() {
    cout << "THREAD 2 STARTING" << endl;
    size_t id = 2;
    for (size_t i = 0; i < 15; i++)
    {
        cout << "thread: " << id << " counter: " << i << endl;
        yield_thread();
    }
    cout << "THREAD 2 FINISHED" << endl;
}

int main()
{
    Runtime runtime;
    runtime.init();
    runtime.spawn(func1);
    runtime.spawn(func2);
    int sum = 0;
    runtime.spawn([&sum]() {
        cout << "THREAD lambda STARTING" << endl;
        string id = "lambda";
        for (size_t i = 0; i < 25; i++)
        {
            sum += i;
            cout << "thread: " << id << " counter: " << i << ",sum=" << sum << endl;
            yield_thread();
        }
        cout << "THREAD lambda FINISHED" << endl;
    });
    cout << "main start : sum=" << sum << endl;
    int ret = runtime.run();
    cout << "main end : sum=" << sum << endl;
    return ret;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
