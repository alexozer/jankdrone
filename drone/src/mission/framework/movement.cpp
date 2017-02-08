#include "../../maths.h"
#include "../../shm.h"
#include "movement.h"

using namespace mission;

Setter::Setter(V<float> target, 
		float* desire, 
		V<float> current, 
		float deadband, 
		bool moduloError):
	m_target{target},
	m_desire{desire},
	m_current{current},
	m_deadband{deadband},
	m_moduloError{moduloError} {}

bool AbsoluteSetter::onRun() {
	*m_desire = m_target();
	return withinDeadband(m_target(), m_current(), m_deadband, m_moduloError);
}

DeltaSetter::DeltaSetter(V<float> target, 
		float* desire, 
		V<float> current, 
		float deadband, 
		bool moduloError):
	Setter{target, desire, current, deadband, moduloError} {}

bool DeltaSetter::onFirstRun() {
	m_initialValue = m_current();
	return false;
}

bool DeltaSetter::onRun() {
	float absoluteTarget = m_initialValue + m_target();
	*m_desire = absoluteTarget;
	return withinDeadband(absoluteTarget, m_current(), m_deadband, m_moduloError);
}

bool RelativeSetter::onRun() {
	float absoluteTarget = m_current() + m_target();
	*m_desire = absoluteTarget;
	return withinDeadband(absoluteTarget, m_current(), m_deadband, m_moduloError);
}

Yaw::Yaw(V<float> target, float deadband):
	AbsoluteSetter{
		target,
		&shm().desires.yaw,
		&shm().placement.yaw,
		deadband,
		true} {}

Pitch::Pitch(V<float> target, float deadband):
	AbsoluteSetter{
		target,
		&shm().desires.pitch,
		&shm().placement.pitch,
		deadband,
		true} {}

Roll::Roll(V<float> target, float deadband):
	AbsoluteSetter{
		target,
		&shm().desires.roll,
		&shm().placement.roll,
		deadband,
		true} {}

Force::Force(V<float> target):
	AbsoluteSetter{
		target,
		&shm().desires.force,
		&shm().desires.force,
		0,
		false} {}

DeltaYaw::DeltaYaw(V<float> delta, float deadband):
	DeltaSetter{
		delta,
		&shm().desires.yaw,
		&shm().placement.yaw,
		deadband,
		true} {}

DeltaPitch::DeltaPitch(V<float> delta, float deadband):
	DeltaSetter{
		delta,
		&shm().desires.pitch,
		&shm().placement.pitch,
		deadband,
		true} {}

DeltaRoll::DeltaRoll(V<float> delta, float deadband):
	DeltaSetter{
		delta,
		&shm().desires.roll,
		&shm().placement.roll,
		deadband,
		true} {}

DeltaForce::DeltaForce(V<float> delta):
	DeltaSetter{
		delta,
		&shm().desires.force,
		&shm().desires.force,
		0,
		false} {}

RelativeYaw::RelativeYaw(V<float> offset, float deadband):
	RelativeSetter{
		offset,
		&shm().desires.yaw,
		&shm().placement.yaw,
		deadband,
		true} {}

RelativePitch::RelativePitch(V<float> offset, float deadband):
	RelativeSetter{
		offset,
		&shm().desires.pitch,
		&shm().placement.pitch,
		deadband,
		true} {}

RelativeRoll::RelativeRoll(V<float> offset, float deadband):
	RelativeSetter{
		offset,
		&shm().desires.roll,
		&shm().placement.roll,
		deadband,
		true} {}

RelativeForce::RelativeForce(V<float> offset):
	RelativeSetter{
		offset,
		&shm().desires.force,
		&shm().desires.force,
		0,
		false} {}
