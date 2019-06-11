#include "state_nmode.h"

#include "receiver.h"
#include "channels.h"
#include "ui.h"
#include "settings_eeprom.h"

using Ui::display;

void StateMachine::NmodeStateHandler::onInitialDraw() {

	this->onUpdateDraw();
}

void StateMachine::NmodeStateHandler::onUpdateDraw() {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t x_old = 0;
	uint8_t y_old = 45;
	uint16_t wtmp = 0;
	float ftmp = 0;

	// --- full redraw only when needed
	if (redraw_all) {
		// -- reset signal and clear fb
		redraw_all = false;
		Ui::clear();

		// -- spectrum
		for (uint8_t i=1;i<56;i++) {
			x += 2;
			if (scan_array[i] < 44) {
				wtmp = (scan_array[i - 1] + scan_array[i] + scan_array[i+1]) / 3;
				y = 45 - (uint8_t)wtmp;
			} else {
				y = 1;
			}
			Ui::display.drawLine(x, y, x_old, y_old, WHITE);
			x_old = x;
			y_old = y;
			// - scanpos marker
			if (i == scan_pos) {
				Ui::display.drawLine(x, 1, x, 46, WHITE);
			}
		}
		Ui::display.drawRect(0, 0, 111, 47, WHITE);

		// -- draw marker, current channel
		wtmp = Channels::getFrequency(Receiver::activeChannel) - scan_base;
		ftmp = (float)wtmp / scan_step;
		x = (uint8_t)ftmp * 2 + 1;
		Ui::display.drawLine(x, 1, x, 46, WHITE);

		// -- draw marker, all channels in band
		for (uint8_t i=0;i<8;i++) {
			wtmp = Channels::getFrequency(BandBaseChannel + i) - scan_base;
			ftmp = (float)wtmp / scan_step;
			x = (uint8_t)ftmp * 2 + 1;
			Ui::display.drawLine(x, 44, x, 46, WHITE);
			Ui::display.drawPixel(x, 18, WHITE);
			Ui::display.drawPixel(x, 32, WHITE);
			Ui::display.drawLine(x, 1, x, 2, WHITE);
		}

		// -- print active channel name and frequency
		Ui::display.setTextSize(2);
		Ui::display.setTextColor(WHITE);
		Ui::display.setCursor(0, 50);
		Ui::display.print(Channels::getName(Receiver::activeChannel));
		Ui::display.setCursor(65, 50);
		Ui::display.print(Channels::getFrequency(Receiver::activeChannel));

		// -- print AB to rssi bars
		if (!inBandSelect) {
			Ui::display.setTextSize(1);
			Ui::display.setCursor(115, 53);
			Ui::display.print("A");
			Ui::display.setCursor(122, 53);
			Ui::display.print("B");
		}

		// -- print band select menu
		if (inBandSelect) {
			uint8_t offset = (64 - (BandCount * 10)) >> 1;
			for (uint8_t i=0;i<BandCount;i++) {
				Ui::display.setTextSize(1);
				Ui::display.setCursor(119, i*10+2+offset);
				Ui::display.print(BandName[i]);
			}
			offset += (Receiver::activeChannel / 8) * 10;
			Ui::display.drawLine(116, offset, 127, offset, WHITE);
			Ui::display.drawLine(127, offset, 127, offset+10, WHITE);
			Ui::display.drawLine(127, offset+10, 116, offset+10, WHITE);
			Ui::display.drawLine(116, offset+10, 116, offset, WHITE);
		}
	} else if (!inBandSelect) {
		// -- don't redraw everything, but clear rssi bars
		for (int i=0;i<5;i++) {
			Ui::display.drawLine(115+i, 49, 115+i, 0, BLACK);
			Ui::display.drawLine(122+i, 49, 122+i, 0, BLACK);
			x++;
		}
	}

	if (!inBandSelect) {
		// --- rssi bars, always redraw
		x = 115;
		y = 50;
		int h = y - (Receiver::rssiA >> 1) - 1;
		for (int i=0;i<5;i++) {
			Ui::display.drawLine(x, y, x, h, WHITE);
			x++;
		}
		x = 122;
		h = y - (Receiver::rssiB >> 1) - 1;
		for (int i=0;i<5;i++) {
			Ui::display.drawLine(x, y, x, h, WHITE);
			x++;
		}

		// --- mark active receiver
		switch (Receiver::activeReceiver) {
			case Receiver::ReceiverId::A:
				Ui::display.drawLine(115, 62, 119, 62, WHITE);
				Ui::display.drawLine(115, 63, 119, 63, WHITE);
				Ui::display.drawLine(122, 62, 126, 62, BLACK);
				Ui::display.drawLine(122, 63, 126, 63, BLACK);
				break;
			case Receiver::ReceiverId::B:
				Ui::display.drawLine(122, 62, 126, 62, WHITE);
				Ui::display.drawLine(122, 63, 126, 63, WHITE);
				Ui::display.drawLine(115, 62, 119, 62, BLACK);
				Ui::display.drawLine(115, 63, 119, 63, BLACK);
				break;
		}
	}

	Ui::needDisplay();
}
