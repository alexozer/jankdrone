#pragma once

#include <memory>
#include "../shm.h"

namespace mission {

template <typename T>
class V {
	public:
		V(T v): m_source{Source::CONST}, m_constant{v} {}
		V(std::function<T()> v): m_source{Source::FUNC}, m_func{v} {}
		V(Shm::Var* v): m_source{Source::SHM}, m_shm{v} {}

		T operator()() {
			switch (m_source) {
				case Source::CONST:
					return m_constant;
				case Source::FUNC:
					return m_func();
				case Source::SHM:
					return readShm();
				default:
					return m_constant;
			}
		}

	private:
		enum class Source { CONST, FUNC, SHM };

		Source m_source;
		union {
			T m_constant;
			Shm::Var* m_shm;
		};
		std::function<T()> m_func;

		T readShm() {
			// Oh no! Unsupported shm type
			return m_constant;
		}
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

} // namespace mission
