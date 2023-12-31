#include "ConjunctionModule.h"

void ConjunctionModule::AddUpstreamModule(Module* upstreamModule) {
	Module::AddUpstreamModule(upstreamModule);
	upstreamModuleLevels[upstreamModule->name] = LOW;
}

void ConjunctionModule::ReceivePulse(Pulse* pulse) {
	auto upstreamName = pulse->fromModule->name;
	if (!upstreamModuleLevels.contains(upstreamName)) {
		throw "This conjuction module does not recognize upstream module: " + upstreamName;
	}

	upstreamModuleLevels[upstreamName] = pulse->level;

	bool areAllUpstreamLevelsHigh = true;
	for (auto upstreamModuleLevel : upstreamModuleLevels) {
		if (upstreamModuleLevel.second == LOW) {
			areAllUpstreamLevelsHigh = false;
			break;
		}
	}
	if (areAllUpstreamLevelsHigh) {
		SendPulse(LOW);
		lastSentPulse = 0;
	}
	else {
		SendPulse(HIGH);
		lastSentPulse = 1;
		if (this->name == "tx") {
			//throw;
		}
	}
}