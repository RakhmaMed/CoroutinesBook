#include <coroutine>
#include <iostream>
#include <functional>
#include <random>
#include <thread>
#include <vector>

std::random_device seed;
auto gen = std::bind_front(std::uniform_int_distribution<>(0, 1),
	std::default_random_engine(seed()));

struct MySuspendAlways {
	bool await_ready() const noexcept {
		std::cout << "\t\tMySuspendAlways::await_ready\n";
		return gen();
	}
	bool await_suspend(std::coroutine_handle<> handle) const noexcept {
		std::cout << "\t\tMySuspendAlways::await_suspend\n";
		handle.resume();
		return true;
	}
	void await_resume() const noexcept {
		std::cout << "\t\tMySuspendAlways::await_resume\n";
	}
};

struct MyAwaitable {
	std::jthread& outerThread;

	bool await_ready() const noexcept {
		auto res = gen();
		if (res)
			std::cout << " (executed)\n";
		else
			std::cout << " (suspended)\n";
		return res;
	}

	void await_suspend(std::coroutine_handle<> h) {
		outerThread = std::jthread([h] { h.resume(); });
	}
	
	void await_resume() {}
};

struct MySuspendNever {
	bool await_ready() const noexcept {
		std::cout << "\t\tMySuspendNever::await_ready\n";
		return true;
	}
	void await_suspend(std::coroutine_handle<>) const noexcept {
		std::cout << "\t\tMySuspendNever::await_suspend\n";
	}
	void await_resume() const noexcept {
		std::cout << "\t\tMySuspendNever::await_resume\n";
	}
};

struct Job {
	static inline int JobCounter{ 1 };

	Job() {
		++JobCounter;
	}

	struct promise_type {
		int JobNumber{ JobCounter };

		Job get_return_object() {
			return {};
		}

		std::suspend_never initial_suspend() {
			std::cout << "\tJob " << JobNumber << " prepared on thread "
				<< std::this_thread::get_id();
			return{};
		}

		std::suspend_never final_suspend() noexcept {
			std::cout << "\tJob " << JobNumber << " finished on thread "
				<< std::this_thread::get_id() << '\n';
			return {};
		}
		void return_void() {}
		void unhandled_exception() {}
	};
};

Job performJob(std::jthread& out) {
	co_await MyAwaitable{out};
}

int main() {
	std::vector<std::jthread> threads(8);

	for (auto& thr : threads)
		performJob(thr);
}