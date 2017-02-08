#pragma once

#include <functional>
#include <vector>

class Thread {
	public:
		static constexpr int SECOND = 1e6;

		Thread(std::function<void()> func, unsigned long intervalMicros, int* tickTime = nullptr);
		void operator()();

	private:
		std::function<void()> m_func;
		unsigned long m_interval;
		int* m_tickTime;

		unsigned long m_lastTime;
		bool m_hasRun;
};

class FuncSet {
	public:
		FuncSet(std::initializer_list<std::function<void()>> funcs);
		void operator()();
	
	private:
		std::vector<std::function<void()>> m_funcs;
};
