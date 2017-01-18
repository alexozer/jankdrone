#pragma once

#include "task.h"

namespace mission {

constexpr float TILT_DEADBAND = 1,
		  YAW_DEADBAND = 5;

class Setter : public BaseTask {
	public:
		Setter(V<float> target, 
				std::function<void(float)> desireSetter, 
				V<float> current, 
				float deadband, 
				bool moduloError);
		virtual ~Setter();

	protected:
		V<float> m_target;
		std::function<void(float)> m_desireSetter;
		V<float> m_current;
		float m_deadband;
		bool m_moduloError;
};

class AbsoluteSetter : public Setter {
	public:
		using Setter::Setter;

		bool onRun() override;
};

class DeltaSetter : public Setter {
	public:
		DeltaSetter(V<float> target, 
				std::function<void(float)> desireSetter, 
				V<float> current, 
				float deadband, 
				bool moduloError);

		bool onFirstRun() override;
		bool onRun() override;

	private:
		float m_initialValue;
};

class RelativeSetter : public Setter {
	public:
		using Setter::Setter;

		bool onRun() override;
};

class Yaw : public AbsoluteSetter {
	public:
		Yaw(V<float> target, float deadband = YAW_DEADBAND);
};

class Pitch : public AbsoluteSetter {
	public:
		Pitch(V<float> target, float deadband = TILT_DEADBAND);
};

class Roll : public AbsoluteSetter {
	public:
		Roll(V<float> target, float deadband = TILT_DEADBAND);
};

class Force : public AbsoluteSetter {
	public:
		Force(V<float> target);
};

class DeltaYaw : public DeltaSetter {
	public:
		DeltaYaw(V<float> delta, float deadband = YAW_DEADBAND);
};

class DeltaPitch : public DeltaSetter {
	public:
		DeltaPitch(V<float> delta, float deadband = TILT_DEADBAND);
};

class DeltaRoll : public DeltaSetter {
	public:
		DeltaRoll(V<float> delta, float deadband = TILT_DEADBAND);
};

class DeltaForce : public DeltaSetter {
	public:
		DeltaForce(V<float> delta);
};

class RelativeYaw : public RelativeSetter {
	public:
		RelativeYaw(V<float> offset, float deadband = YAW_DEADBAND);
};

class RelativePitch : public RelativeSetter {
	public:
		RelativePitch(V<float> offset, float deadband = TILT_DEADBAND);
};

class RelativeRoll : public RelativeSetter {
	public:
		RelativeRoll(V<float> offset, float deadband = TILT_DEADBAND);
};

class RelativeForce : public RelativeSetter {
	public:
		RelativeForce(V<float> offset);
};

} // namespace mission
