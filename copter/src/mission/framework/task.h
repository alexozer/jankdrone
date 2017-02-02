#pragma once

#include <memory>

namespace mission {

template <typename T>
class V {
	public:
		V(T v): m_source{Source::CONST}, m_constant{v} {}
		V(std::function<T()> v): m_source{Source::FUNC}, m_func{v} {}
		V(T* v): m_source{Source::PTR}, m_ptr{v} {}

		T operator()() {
			switch (m_source) {
				case Source::CONST:
					return m_constant;
				case Source::FUNC:
					return m_func();
				case Source::PTR:
					return *m_ptr;
				default:
					return m_constant;
			}
		}

	private:
		enum class Source { CONST, FUNC, PTR };

		// Can't use union; T may not be trivially destructable
		Source m_source;
		T m_constant;
		T* m_ptr;
		std::function<T()> m_func;
};

class BaseTask;
using Task = std::shared_ptr<BaseTask>;

class BaseTask {
	public:
		BaseTask();
		BaseTask(Task task);
		virtual ~BaseTask();

		// Run one tick of the task
		bool operator()();

		// Did the task finish on its last run?
		// (Finished state is reset every run)
		bool finished() const;

		// Has the task ever returned finished?
		bool hasEverFinished() const;

		// Is the task currently in a success state?
		// (Success state persists across runs)
		bool success() const;

	protected:
		// Return true on finish
		virtual bool onFirstRun();
		virtual bool onRun();
		virtual void onFinish();
		
		// Default is success
		void succeed();
		void fail();
		void success(bool state);

	private:
		Task m_task;
		bool m_hasRun;
		bool m_finished;
		bool m_hasEverFinished;
		bool m_success;
};

template <typename T, typename... Args>
std::shared_ptr<T> make(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace mission
