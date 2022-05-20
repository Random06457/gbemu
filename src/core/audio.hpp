#pragma once

#include "device.hpp"

namespace gbemu::core
{

class Audio : public Device
{
public:
    Audio();

    virtual void mapMemory(Memory* mem) override;

private:
    struct
    {
        u8 so1_output_level : 3;
        u8 so1_output_vin : 1;
        u8 so2_output_level : 3;
        u8 so2_output_vin : 1;
    } PACKED m_nr50; // Channel control / ON-OFF / Volume (R/W)

    struct
    {
        u8 sound_1_so1 : 1;
        u8 sound_2_so1 : 1;
        u8 sound_3_so1 : 1;
        u8 sound_4_so1 : 1;
        u8 sound_1_so2 : 1;
        u8 sound_2_so2 : 1;
        u8 sound_3_so2 : 1;
        u8 sound_4_so2 : 1;
    } PACKED m_nr51; // Selection of Sound output terminal (R/W)

    struct
    {
        u8 sound_1_on : 1;
        u8 sound_2_on : 1;
        u8 sound_3_on : 1;
        u8 sound_4_on : 1;
        u8 : 3;
        u8 all_sound_on : 1;
    } PACKED m_nr52; // Sound on/off

    struct
    {
        u8 sound_length : 6; // write only
        u8 wave_pattern_duty : 2;
    } PACKED m_nr11; // Channel 1 Sound length/Wave pattern duty (R/W)

    struct
    {
        u8 number_of_envelope_sweep : 3;
        u8 envelope_direction : 1;
        u8 initial_envelope_volume : 4;
    } PACKED m_nr12; // Channel 1 Volume Envelope (R/W)

    union
    {
        struct
        {
            // Channel 1 Frequency lo (Write Only)
            u8 m_nr13;
            // Channel 1 Frequency hi (R/W)
            u8 m_nr14;
        };
        struct
        {
            u16 ch1_freq : 11;
            u16 : 3;
            u16 counter_selection : 1;
            u16 initial : 1;
        } m_ch1_freq;
    };
};

}
