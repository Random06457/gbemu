#pragma once

#include "device.hpp"

namespace gbemu::core
{
static constexpr size_t AUDIO_SAMPLE_RATE = 44100;
static constexpr size_t AUDIO_BUFFER_SIZE = 0x1000;

class Apu : public Device
{
public:
    Apu();
    ~Apu();

    virtual void mapMemory(Memory* mem) override;

    void step(size_t clocks_diff);
    void decodeChannelWave(s16* samples, size_t sample_count);
    void decodeChannelPulseA(s16* samples, size_t sample_count);
    void decodeChannelPulseB(s16* samples, size_t sample_count);
    void decodeChannelNoise(s16* samples, size_t sample_count);

    // backend
    void initPlayer();
    void destroyPlayer();
    void play(const void* data, size_t size);
    size_t getBuffered();
    size_t getDesiredBuffered();


    auto audioBuffer() { return m_audio_buffer; }
    auto audioBufferSize() const { return m_audio_buffer_size; }
    auto setAudioBufferSize(size_t new_size)  { m_audio_buffer_size = new_size; }

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

    union ControlReg
    {
        u8 raw;
        struct
        {
            u8 freq_hi : 3;
            u8 : 3;
            u8 counter_selection : 1; // 1 = Stop when NR31 expires
            u8 initial : 1; // 1 = Restart Sound
        };
    } PACKED;

    union FrequencyReg
    {
        u8 raw;
        u8 freq_lo;
    } PACKED;

    struct FullFrequencyReg
    {
        struct
        {
            u16 freq : 11;
            u16 : 3;
            u16 : 1;
            u16 : 1;
        };
    } PACKED;

    #define DEFINE_NR_3_4(ch) \
    union \
    { \
        struct \
        { \
            FrequencyReg m_nr##ch##3; \
            ControlReg m_nr##ch##4; \
        }; \
        FullFrequencyReg m_nr##ch##3_nr##ch##4; \
    } PACKED;

    union VolumeEnvelopReg
    {
        u8 raw;
        struct
        {
            u8 number_of_envelope_sweep : 3;
            u8 envelope_direction : 1;
            u8 initial_envelope_volume : 4;
        };
    } PACKED;

    union WaveVolumeReg
    {
        u8 raw;
        struct
        {
            u8 : 5;
            u8 volume : 2;
            u8 : 1;
        };
    } PACKED;

    union LengthReg
    {
        u8 raw;
        struct
        {
            u8 sound_length : 6; // write only
            u8 wave_pattern_duty : 2;
        };
    } PACKED;

    union WaveLengthReg
    {
        u8 raw;
        u8 sound_length;
    } PACKED;

    union SweepReg
    {
        u8 raw;
        struct
        {
            u8 number_of_seep : 3;
            u8 sweep_decrease : 1;
            u8 sweep_time : 3;
            u8 : 1;
        };
    } PACKED;

    SweepReg m_nr10;
    LengthReg m_nr11;
    VolumeEnvelopReg m_nr12;
    DEFINE_NR_3_4(1);

    LengthReg m_nr21;
    VolumeEnvelopReg m_nr22;
    DEFINE_NR_3_4(2);

    WaveLengthReg m_nr31;
    WaveVolumeReg m_nr32;
    DEFINE_NR_3_4(3);

    u8 m_wave[16];

    s16 m_ch1_buffer[AUDIO_BUFFER_SIZE];
    s16 m_ch2_buffer[AUDIO_BUFFER_SIZE];
    s16 m_ch3_buffer[AUDIO_BUFFER_SIZE];
    s16 m_ch4_buffer[AUDIO_BUFFER_SIZE];
    s16 m_audio_buffer[AUDIO_BUFFER_SIZE];
    size_t m_audio_buffer_size;

    size_t m_clocks;


    f64 m_ch1_prev_x = 0;
    f64 m_ch2_prev_x = 0;
    f64 m_ch3_prev_x = 0;
    f64 m_ch4_prev_x = 0;
};

}
