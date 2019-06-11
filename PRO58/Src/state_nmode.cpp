#include "state_nmode.h"

#include "settings.h"
#include "settings_internal.h"
#include "settings_eeprom.h"
#include "timer.h"
#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "ui.h"


using StateMachine::NmodeStateHandler;

void NmodeStateHandler::onEnter() {
	this->orderedChanelIndex = EepromSettings.startChannel;
	Receiver::setChannel(orderedChanelIndex);

	// --- calc number of bands
	BandCount = CHANNELS_SIZE / 8;

	// --- clear scan array, start scan
	for (uint8_t i=0;i<64;i++) scan_array[i] = 0;
	startScan();
}

void NmodeStateHandler::onUpdate() {
	if(!Receiver::isRssiStable())
		return;

	// --- handle scanning
	if (scanning) {
		// -- fetch rssi
		scan_array[scan_pos] = Receiver::rssiB >> 1;
		// -- handle counter
		if (scan_pos >= 57) {
			stopScan();
		} else {
			scan_pos++;
			scan_freq = scan_base + (uint16_t)((float)scan_pos * scan_step);
			Receiver::setScannerFrequency(scan_freq);
		}
		// -- full redraw please
		redraw_all = true;
	}

    Ui::needUpdate();
}

void NmodeStateHandler::startScan() {
	// --- set defaults
	scan_pos = 0;
	scanning = true;
	redraw_all = true;

	// --- find min/max frequency in band
	BandBaseFreq = 10000;
	BandTopFreq = 0;
	BandBaseChannel = Receiver::activeChannel - (Receiver::activeChannel % 8);
	scan_freq = 0;
	for (uint8_t i=0;i<8;i++) {
		scan_freq = Channels::getFrequency(BandBaseChannel+i);
		if (BandBaseFreq > scan_freq) BandBaseFreq = scan_freq;
		if (BandTopFreq < scan_freq) BandTopFreq = scan_freq;
	}

	// --- calc and apply top/bottom margin
	uint16_t tmpdiff = (BandTopFreq - BandBaseFreq) / 15;
	scan_base = BandBaseFreq - (tmpdiff<<1);
	scan_top = BandTopFreq + tmpdiff;

	// --- calc step size, set sweep start frequency
	scan_step = (float)(scan_top - scan_base) / 52;
	scan_freq = scan_base;

	// --- backup diversity mode and force receiver A
	diversityModeBackup = EepromSettings.diversityMode;
	EepromSettings.diversityMode = Receiver::DiversityMode::FORCE_A;

	// --- tune to 1st frequency
	Receiver::setScannerFrequency(scan_freq);
}

void NmodeStateHandler::stopScan() {
	scanning = false;
	// --- switch back to saved diversity mode
	Receiver::setScannerFrequency(Channels::getFrequency(Receiver::activeChannel));
	EepromSettings.diversityMode = diversityModeBackup;
}

void NmodeStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
	bool switchChannel = false;

		if (inBandSelect) {
			if (pressType == Buttons::PressType::SHORT) {
				switch (button) {
					case Button::MODE:
						inBandSelect = false;
						redraw_all = true;
						break;
					case Button::UP:
						orderedChanelIndex -= 8;
						if (orderedChanelIndex > 247) orderedChanelIndex = (orderedChanelIndex % 8) + (BandCount * 8) - 8;
						switchChannel = true;
						break;
					case Button::DOWN:
						orderedChanelIndex += 8;
						if (orderedChanelIndex > CHANNELS_SIZE) orderedChanelIndex %= 8;
						switchChannel = true;
						break;
					default:
						break;
				}
			}
		} else {
			switch (pressType) {
				case Buttons::PressType::SHORT:
					switch (button) {
						case Button::MODE:
							if (!scanning) startScan();
							break;
						case Button::UP:
							orderedChanelIndex += 1;
							switchChannel = true;
							break;
						case Button::DOWN:
							orderedChanelIndex -= 1;
							switchChannel = true;
							break;
						default:
							break;
					}
					break;
				case Buttons::PressType::LONG:
				case Buttons::PressType::HOLDING:
					switch (button) {
						case Button::UP:
							inBandSelect = true;
							redraw_all = true;
							break;
						case Button::DOWN:
							inBandSelect = true;
							redraw_all = true;
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}

		if (orderedChanelIndex == 255) {
			orderedChanelIndex = CHANNELS_SIZE - 1;
		} else
		if (orderedChanelIndex >= CHANNELS_SIZE) {
			orderedChanelIndex = 0;
		}

	if (switchChannel) {
		this->setAndSaveChannel();
		redraw_all = true;
		// -- start scan if band changed
		if ((orderedChanelIndex - BandBaseChannel) > 7) startScan();
		if (orderedChanelIndex < BandBaseChannel) startScan();
	}
}

void NmodeStateHandler::setAndSaveChannel() {
    Receiver::setChannel(orderedChanelIndex, true);
}

void NmodeStateHandler::onFSPinsChange(uint8_t state){
	uint8_t bandOffset = Receiver::activeChannel - (Receiver::activeChannel % 8);
	orderedChanelIndex = bandOffset + state;
	this->setAndSaveChannel();
}
