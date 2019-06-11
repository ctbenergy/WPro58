#ifndef STATE_Nmode_H
#define STATE_Nmode_H


#include "state_graph_base_ui.h"
#include "ui_state_menu.h"
#include "receiver.h"

#define PEAK_LOOKAHEAD 4


namespace StateMachine {
    class NmodeStateHandler : public StateMachine::BaseGraphUiStateHandler {
        private:
            uint8_t scan_pos = 0;
            uint16_t scan_freq = 0;
            uint8_t scan_array[64];
            bool scanning = false;

            uint8_t BandCount = 0;
        	uint8_t BandBaseChannel = 0;
            uint16_t BandBaseFreq = 0;
            uint16_t BandTopFreq = 0;
            uint16_t scan_base = 0;
			uint16_t scan_top = 0;
			float scan_step = 0;

			char BandName[6] = { 'A', 'B', 'E', 'F', 'R', 'L'};

			bool redraw_all = false;

			bool inBandSelect = false;

			Receiver::DiversityMode diversityModeBackup = Receiver::DiversityMode::AUTO;

            void startScan();
            void stopScan();

            void setAndSaveChannel();

        public:
            uint8_t orderedChanelIndex = 0;

            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

            void onButtonChange(Button button, Buttons::PressType pressType);
            void onFSPinsChange(uint8_t state);
    };
}


#endif
