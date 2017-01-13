#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace Logger {
	enum class Level;
}

namespace mission {

// TODO support shm vars?
template <typename T>
class V {
	public:
		V(std::function<T()> func): m_func{func} {}
		V(T constant): m_constant{constant} {}
		
		T operator()() { return m_func ? m_func() : m_constant; }

	private:
		std::function<T()> m_func;
		T m_constant;
};

class Task {
	public:
		typedef std::shared_ptr<Task> task;

		Task();
		virtual ~Task();

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
		bool m_hasRun;
		bool m_finished;
		bool m_hasEverFinished;
		bool m_success;
};

class Sequential : public Task {
	public:
		Sequential(std::vector<task> tasks, bool finite = true);
		bool onRun() override;

	private:
		std::vector<task> m_tasks;
		bool m_finite;
		decltype(m_tasks.begin()) m_currentTask;
};

class Concurrent : public Task {
	public:
		Concurrent(std::vector<task> tasks, bool finite = true);
		bool onRun() override;

	private:
		std::vector<task> m_tasks;
		bool m_finite, m_lastSuccess;
};

class MasterConcurrent : public Task {
	public:
		MasterConcurrent(task primaryTask, task secondaryTask);
		bool onRun() override;

	private:
		task m_primaryTask, m_secondaryTask;
};

class Timer : public Task {
	public:
		Timer(size_t period);
		bool onFirstRun() override;
		bool onRun() override;

	private:
		size_t m_period;
		size_t m_startTime;
};

class Timeout : public Task {
	public:
		Timeout(task task, size_t timeout);
		bool onRun() override;
		bool timedOut() const;

	private:
		task m_task, m_timer;
		bool m_timedOut;
};

class NoOp : public Task {
	public:
		NoOp(bool finite = true);
		bool onRun() override;

	private:
		bool m_finite;
};

class Function : public Task {
	public:
		Function(std::function<void()> func);
		bool onRun() override;

	private:
		std::function<void()> m_func;
};

class Log : public Task {
	public:
		Log(Logger::Level level, std::string msg);
		bool onRun() override;

	private:
		Logger::Level m_level;
		std::string m_msg;
};

class Debug : public Log {
	public:
		Debug(std::string msg);
};

class Info : public Log {
	public:
		Info(std::string msg);
};

class Warn : public Log {
	public:
		Warn(std::string msg);
};

class Error : public Log {
	public:
		Error(std::string msg);
};

class Fatal : public Log {
	public:
		Fatal(std::string msg);
};

class Setter : public Task {
	public:
		Setter(V<float> target, 
				std::function<void(float)> desireSetter, 
				V<float> current, 
				float deadBand, 
				bool moduloError);
		bool onRun() override;

	private:
		V<float> m_target;
		std::function<void(float)> m_desireSetter;
		V<float> m_current;
		float m_deadBand;
		bool m_moduloError;
};

} // namespace mission
