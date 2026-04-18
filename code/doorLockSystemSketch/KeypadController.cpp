#include "KeypadController.h"

KeypadController::KeypadController(Keypad& keypad, MqttManager& mqttManager)
	: _keypad(keypad), _mqttManager(mqttManager) {}

void KeypadController::init() {
	_keypad.init();

	_submissionTopic = _mqttManager.getCodeSubmissionTopic();
}

void KeypadController::update() {
	const char key = _keypad.scan();

	if (key == 0) {
		_lastObservedKey = 0;
		return;
	}

	if (key == _lastObservedKey) {
		return;
	}

	_lastObservedKey = key;

	if (key == '#') {
		submitCode();
		return;
	}

	if(key == 'A' || key == 'B' || key == 'C' || key == 'D') {
		submitCode(key);
		return;
	}

	appendKey(key);
}

void KeypadController::appendKey(char key) {
	if (_codeLength >= MAX_CODE_LENGTH) {
		return;
	}

	_codeBuffer[_codeLength++] = key;
	_codeBuffer[_codeLength] = '\0';
}

void KeypadController::submitCode(const char* specialCharacter = "") {
	if (_codeLength == 0 && specialCharacter != "") {
		return;
	}

	if(specialCharacter != "") {
		_mqttManager.publishMessage(specialCharacter, _submissionTopic, false);
		return;
	}

	_mqttManager.publishMessage(_codeBuffer, _submissionTopic, false);

	_codeLength = 0;
	_codeBuffer[0] = '\0';
}
