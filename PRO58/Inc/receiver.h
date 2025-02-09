#ifndef RECEIVER_H
#define RECEIVER_H


#include <stdint.h>
#include "settings.h"
#include "stm32f1xx_hal.h"

#define RECEIVER_LAST_DELAY 50
#define RECEIVER_LAST_DATA_SIZE 24


namespace Receiver {
    enum class ReceiverId : uint8_t {
        A
        #ifdef USE_DIVERSITY
            ,
            B
        #endif
    };

    #ifdef USE_DIVERSITY
        enum class DiversityMode : uint8_t {
            AUTO,
            FORCE_A,
            FORCE_B
        };
    #endif


    extern ReceiverId activeReceiver;
    extern uint8_t activeChannel;

    extern uint8_t rssiA;
    extern uint16_t rssiARaw;
    extern uint8_t rssiALast[RECEIVER_LAST_DATA_SIZE];
    #ifdef USE_DIVERSITY
        extern uint8_t rssiB;
        extern uint16_t rssiBRaw;
        extern uint8_t rssiBLast[RECEIVER_LAST_DATA_SIZE];
    #endif

    void setChannel(uint8_t channel, bool store = false);
    void setBandChannel(uint8_t channel, bool store = false);
    void setScannerFrequency(uint16_t freq);
    void updateRssi();
    void setActiveReceiver(ReceiverId receiver = ReceiverId::A);
    #ifdef USE_DIVERSITY
        void setDiversityMode(DiversityMode mode);
        void switchDiversity();
    #endif

    bool isRssiStable();


    void setup(ADC_HandleTypeDef *adc_handle);
    void update();
}


#endif
