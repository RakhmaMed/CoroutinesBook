//#include <coroutine>
//#include <iostream>
//#include <memory>
//#include <thread>
//
//template<typename T>
//struct LazyFuture {
//	struct promise_type;
//	using handle_type = std::coroutine_handle<promise_type>;
//
//	handle_type coro;
//
//	LazyFuture(handle_type h) : coro(h) {
//		std::cout << "    MyFuture::MyFuture\n";
//	}
//	~LazyFuture() {
//		std::cout << "    MyFuture::~MyFuture\n";
//		if (coro) coro.destroy();
//	}
//	T get() {
//		std::cout << "    MyFuture::get:"
//			<< "std::this_thread::get_id(): "
//			<< std::this_thread::get_id() << '\n';
//		std::thread t([this] {coro.resume(); });
//		t.join();
//		return coro.promise().result;
//	}
//
//	struct promise_type {
//		promise_type() {
//			std::cout << "        promise_type::promise_type: "
//				<< "std::this_thread::get_id(): "
//				<< std::this_thread::get_id() << '\n';
//		}
//		~promise_type() {
//			std::cout << "        promise_type::~promise_type : "
//				<< "std::this_thread::get_id(): "
//				<< std::this_thread::get_id() << '\n';
//		}
//
//		T result;
//
//		auto get_return_object() {
//			//std::cout << "        promise_type::get_return_object\n";
//			return LazyFuture{ handle_type::from_promise(*this) };
//		}
//		void return_value(T v) {
//			std::cout << "        promise_type::return_value: "
//				<< "std::this_thread::get_id(): "
//				<< std::this_thread::get_id() << '\n';
//			result = v;
//		}
//		std::suspend_always initial_suspend() {
//			//std::cout << "        promise_type::initial_suspend\n";
//			return {};
//		}
//		std::suspend_always final_suspend() noexcept {
//			std::cout << "        promise_type::final_suspend: "
//				<< "std::this_thread::get_id(): "
//				<< std::this_thread::get_id() << '\n';
//			return {};
//		}
//		void unhandled_exception() {
//			std::exit(1);
//		}
//	};
//};
//
//LazyFuture<int> createFuture() {
//	std::cout << "createFuture\n";
//	co_return 2021;
//}
//
//int main() {
//	std::cout << '\n';
//	std::cout << "main: "
//		<< "std::this_thread::get_id(): "
//		<< std::this_thread::get_id() << '\n';
//
//	auto fut = createFuture();
//	auto res = fut.get();
//	std::cout << "res: " << res << '\n';
//
//	std::cout << '\n';
//}