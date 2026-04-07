#pragma once

#include "esp_err.h"
#include "driver/gpio.h"

class Keypad {
public:
	static constexpr int ROWS = 4;
	static constexpr int COLS = 4;

	struct Config {
		gpio_num_t rowPins[ROWS];
		gpio_num_t colPins[COLS];
		char keymap[ROWS][COLS];
		uint32_t debounceMs = 30;
		uint32_t settleDelayUs = 5;
	};

	using KeyCallback = void (*)(char key, void *userContext);

	explicit Keypad(const Config &config);

	esp_err_t init();

	bool scan(char &keyOut);

	void setKeyCallback(KeyCallback callback, void *userContext = nullptr);

private:
	Config config_;
	KeyCallback callback_ = nullptr;
	void *callbackContext_ = nullptr;

	char lastRawKey_ = '\0';
	char stableKey_ = '\0';
	bool keyLatched_ = false;
	uint32_t lastRawChangeTick_ = 0;

	char readRawKey() const;
	bool debounceComplete(uint32_t nowTick) const;
};
