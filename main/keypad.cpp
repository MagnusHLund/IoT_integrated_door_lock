#include "keypad.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

Keypad::Keypad(const Config &config)
	: config_(config) {
}

esp_err_t Keypad::init() {
	gpio_config_t rowConfig = {};
	rowConfig.mode = GPIO_MODE_OUTPUT;
	rowConfig.pull_up_en = GPIO_PULLUP_DISABLE;
	rowConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	rowConfig.intr_type = GPIO_INTR_DISABLE;

	for (int row = 0; row < ROWS; ++row) {
		rowConfig.pin_bit_mask |= (1ULL << config_.rowPins[row]);
	}
	ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&rowConfig));

	gpio_config_t colConfig = {};
	colConfig.mode = GPIO_MODE_INPUT;
	colConfig.pull_up_en = GPIO_PULLUP_ENABLE;
	colConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	colConfig.intr_type = GPIO_INTR_DISABLE;

	for (int col = 0; col < COLS; ++col) {
		colConfig.pin_bit_mask |= (1ULL << config_.colPins[col]);
	}
	ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&colConfig));

	for (int row = 0; row < ROWS; ++row) {
		ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(config_.rowPins[row], 1));
	}

	lastRawChangeTick_ = xTaskGetTickCount();
	return ESP_OK;
}

void Keypad::setKeyCallback(KeyCallback callback, void *userContext) {
	callback_ = callback;
	callbackContext_ = userContext;
}

bool Keypad::scan(char &keyOut) {
	keyOut = '\0';

	const uint32_t nowTick = static_cast<uint32_t>(xTaskGetTickCount());
	const char rawKey = readRawKey();

	if (rawKey != lastRawKey_) {
		lastRawKey_ = rawKey;
		lastRawChangeTick_ = nowTick;
	}

	if (debounceComplete(nowTick) && stableKey_ != lastRawKey_) {
		stableKey_ = lastRawKey_;
		if (stableKey_ == '\0') {
			keyLatched_ = false;
		}
	}

	if (stableKey_ != '\0' && !keyLatched_) {
		keyLatched_ = true;
		keyOut = stableKey_;

		if (callback_) {
			callback_(keyOut, callbackContext_);
		}
		return true;
	}

	return false;
}

char Keypad::readRawKey() const {
	for (int row = 0; row < ROWS; ++row) {
		for (int clearRow = 0; clearRow < ROWS; ++clearRow) {
			gpio_set_level(config_.rowPins[clearRow], 1);
		}

		gpio_set_level(config_.rowPins[row], 0);
		esp_rom_delay_us(config_.settleDelayUs);

		for (int col = 0; col < COLS; ++col) {
			if (gpio_get_level(config_.colPins[col]) == 0) {
				return config_.keymap[row][col];
			}
		}
	}

	return '\0';
}

bool Keypad::debounceComplete(uint32_t nowTick) const {
	const uint32_t debounceTicks = static_cast<uint32_t>(pdMS_TO_TICKS(config_.debounceMs));
	return (nowTick - lastRawChangeTick_) >= debounceTicks;
}
