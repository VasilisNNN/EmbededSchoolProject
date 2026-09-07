#include <Arduino.h>
enum TrafficLightState
{
	CanPass,
	FinishPassing,
	CanNotPass,
	FullStop,
	GetReady,
};


void PhaseManager();
void SwitchPhase(TrafficLightState state, uint32_t timercorrectio);