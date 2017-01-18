#include "../../maths.h"
#include "movement.h"

namespace mission {

class ShmSetter {
	public:
		ShmSetter(Shm::Var* v): m_v{v} {}

		void operator()(int val) { m_v->set(val); }
		void operator()(float val) { m_v->set(val); }
		void operator()(bool val) { m_v->set(val); }

	private:
		Shm::Var* m_v;
};

}

using namespace mission;

Setter::Setter(V<float> target, 
		std::function<void(float)> desireSetter, 
		V<float> current, 
		float deadband, 
		bool moduloError):
	m_target{target},
	m_desireSetter{desireSetter},
	m_current{current},
	m_deadband{deadband},
	m_moduloError{moduloError} {}

bool AbsoluteSetter::onRun() {
	m_desireSetter(m_target());
	return withinDeadband(m_target(), m_current(), m_deadband, m_moduloError);
}

DeltaSetter::DeltaSetter(V<float> target, 
		std::function<void(float)> desireSetter, 
		V<float> current, 
		float deadband, 
		bool moduloError):
	Setter{target, desireSetter, current, deadband, moduloError} {}

bool DeltaSetter::onFirstRun() {
	m_initialValue = m_current();
	return false;
}

bool DeltaSetter::onRun() {
	float absoluteTarget = m_initialValue + m_target();
	m_desireSetter(absoluteTarget);
	return withinDeadband(absoluteTarget, m_current(), m_deadband, m_moduloError);
}

bool RelativeSetter::onRun() {
	float absoluteTarget = m_current() + m_target();
	m_desireSetter(absoluteTarget);
	return withinDeadband(absoluteTarget, m_current(), m_deadband, m_moduloError);
}

Yaw::Yaw(V<float> target, float deadband):
	AbsoluteSetter{
		target,
		ShmSetter(Shm::var("controller.yawDesire")),
		V<float>(Shm::var("placement.yaw")),
		deadband,
		true} {}

Pitch::Pitch(V<float> target, float deadband):
	AbsoluteSetter{
		target,
		ShmSetter(Shm::var("controller.pitchDesire")),
		V<float>(Shm::var("placement.pitch")),
		deadband,
		true} {}

Roll::Roll(V<float> target, float deadband):
	AbsoluteSetter{
		target,
		ShmSetter(Shm::var("controller.rollDesire")),
		V<float>(Shm::var("placement.roll")),
		deadband,
		true} {}

Force::Force(V<float> target):
	AbsoluteSetter{
		target,
		ShmSetter(Shm::var("controller.verticalForce")),
		V<float>(Shm::var("controller.verticalForce")),
		0,
		false} {}

DeltaYaw::DeltaYaw(V<float> delta, float deadband):
	DeltaSetter{
		delta,
		ShmSetter(Shm::var("controller.yawDesire")),
		V<float>(Shm::var("placement.yaw")),
		deadband,
		true} {}

DeltaPitch::DeltaPitch(V<float> delta, float deadband):
	DeltaSetter{
		delta,
		ShmSetter(Shm::var("controller.pitchDesire")),
		V<float>(Shm::var("placement.pitch")),
		deadband,
		true} {}

DeltaRoll::DeltaRoll(V<float> delta, float deadband):
	DeltaSetter{
		delta,
		ShmSetter(Shm::var("controller.rollDesire")),
		V<float>(Shm::var("placement.roll")),
		deadband,
		true} {}

DeltaForce::DeltaForce(V<float> delta):
	DeltaSetter{
		delta,
		ShmSetter(Shm::var("controller.forceDesire")),
		V<float>(Shm::var("placement.force")),
		0,
		false} {}

RelativeYaw::RelativeYaw(V<float> offset, float deadband):
	RelativeSetter{
		offset,
		ShmSetter(Shm::var("controller.yawDesire")),
		V<float>(Shm::var("placement.yaw")),
		deadband,
		true} {}

RelativePitch::RelativePitch(V<float> offset, float deadband):
	RelativeSetter{
		offset,
		ShmSetter(Shm::var("controller.pitchDesire")),
		V<float>(Shm::var("placement.pitch")),
		deadband,
		true} {}

RelativeRoll::RelativeRoll(V<float> offset, float deadband):
	RelativeSetter{
		offset,
		ShmSetter(Shm::var("controller.rollDesire")),
		V<float>(Shm::var("placement.roll")),
		deadband,
		true} {}

RelativeForce::RelativeForce(V<float> offset):
	RelativeSetter{
		offset,
		ShmSetter(Shm::var("controller.forceDesire")),
		V<float>(Shm::var("placement.force")),
		0,
		false} {}
