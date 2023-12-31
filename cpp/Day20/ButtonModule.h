#pragma once

#include "Module.h"
#include "PulseQueue.h"
#include "BroadcasterModule.h"

class ButtonModule : public Module {
public:

	ButtonModule(BroadcasterModule* broadcasterModule, PulseQueue* pulseQueue) : Module("button", pulseQueue), broadcasterModule(broadcasterModule), pulseQueue(pulseQueue) {};

	void PushButton() {
		Pulse* buttonPulse = new Pulse(this, broadcasterModule, LOW);
		pulseQueue->SendPulse(buttonPulse);
	}

	void ReceivePulse(Pulse* pulse) override {
		throw "Button should never receive a pulse.";
	};

private:
	BroadcasterModule* broadcasterModule;
	PulseQueue* pulseQueue;
};