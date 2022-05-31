#include "common/logging.hpp"
#include "apu.hpp"
#include "io.hpp"
#include "memory.hpp"
#include "timer.hpp"

namespace gbemu::core
{

static constexpr f64 WAVE_REG_TO_FREQ(u16 reg) { return 65536.0 / (2048-reg); }
static constexpr u16 FREQ_TO_WAVE_REG(f64 f) { return -65536.0 / f + 2048; }

static constexpr f64 WAVE_REG_TO_DURATION(u8 reg) { return (256 - reg) * (1.0 / 256.0); }
static constexpr u8 DURATION_TO_WAVE_REG(f64 t) { return -t * 256.0 + 256.0; }


static constexpr f64 PULSEA_REG_TO_FREQ(u16 reg) { return 131072.0 / (2048-reg); }
static constexpr u16 FREQ_TO_PULSEA_REG(f64 f) { return -131072.0 / f + 2048; }

static constexpr f64 PULSEA_REG_TO_DURATION(u8 reg) { return (64 - reg) * (1.0 / 256.0); }
static constexpr u8 DURATION_TO_PULSEA_REG(f64 t) { return -t * 256.0 + 64.0; }

using Func1 = std::function<f64(f64)>;

static Func1 constant(f64 c)
{
    return [c] (f64 x) -> f64
    {
        return c;
    };
}

static Func1 linearMap(f64 start, f64 end)
{
    return [start, end] (f64 x) -> f64
    {
        return start + (end - start) * x;
    };
}

static Func1 stepMap(f64 start, f64 end, size_t step_count)
{
    return [start, end, step_count] (f64 x) -> f64
    {
        x = floor(x * step_count) / step_count;
        return start + (end - start) * x;
    };
}

static void generateSamples(f64* prev_x, s16* samples, size_t sample_count, Func1 freq, Func1 volume, Func1 generator)
{

    for (size_t i = 0; i < sample_count; i++)
    {
        // f64 t = (f64)i / sample_count;
        f64 t = 0; //TODO: counter / counter_start;
        f64 f = freq(t);

        f64 x = fmod(*prev_x + (f / AUDIO_SAMPLE_RATE), 1.0);
        *prev_x = x;

        f64 y = generator(x);

        s16 sample = y * volume(t) * 0x7FFF;

        *samples++ = sample; // left
        *samples++ = sample; // right
    }
}

void Apu::decodeChannelWave(s16* samples, size_t sample_count)
{
    // TODO: use duration
    f64 duration = WAVE_REG_TO_DURATION(m_nr31.sound_length);
    f64 freq = WAVE_REG_TO_FREQ(m_nr33_nr34.freq);
    u8* wave = m_wave;

    f64 volume_lut[] = { 0, 1.0, 0.5, 0.25 };
    f64 volume = (volume_lut[m_nr32.volume]);
    volume *= 0.3;

    // todo: replace wave
    auto func = [wave] (f64 t) -> f64
    {
        size_t sample_idx = t * 31;
        size_t shift = (sample_idx % 2 == 0) ? 4 : 0;
        u8 sample = (wave[sample_idx/2] >> shift) & 0xF;
        return sample * 2.0 / 0xF - 1.0;
    };

    generateSamples(&m_ch3_prev_x, samples, sample_count, constant(freq), constant(volume), func);
}

void Apu::decodeChannelPulseA(s16* samples, size_t sample_count)
{
    f64 threshold_lut[] = { 0.125 ,0.25, 0.5, 0.75 };
    f64 threshold = threshold_lut[m_nr11.wave_pattern_duty];
    f64 duration = PULSEA_REG_TO_DURATION(m_nr11.sound_length);
    f64 freq = PULSEA_REG_TO_FREQ(m_nr13_nr14.freq);

    f64 volume_end = m_nr12.envelope_direction ? 1.0f : 0.0f;
    f64 volume_start = (f64)m_nr12.initial_envelope_volume / 0xF;

    u8 sweep_count = m_nr12.number_of_envelope_sweep;
    bool stop = m_nr14.counter_selection;
    bool restart = m_nr14.initial;

    static size_t i = 0;
    if (i++ % 1000 == 0)
    {
        // LOG("PULSE A : envelop={}; threshold={}%; freq={}Hz; volume={}%-{}%",
        //     sweep_count, threshold, floor(freq), floor(volume_start*100), floor(volume_end*100));
        // LOG("{:>20}", "");
    }

    auto func = [threshold] (f64 t) -> f64
    {
        f64 y = t < threshold ? 1.0 : -1.0;
        return y;
    };

    f64 volume = 0.3;
    volume_start *= volume;
    volume_end *= volume;

    Func1 envelop = sweep_count == 0
        ? constant(volume_start)
        : stepMap(volume_start, volume_end, 64 / sweep_count);

    // todo: freq sweep + use duration
    generateSamples(&m_ch1_prev_x, samples, sample_count, constant(freq), envelop, func);
}

void Apu::decodeChannelPulseB(s16* samples, size_t sample_count)
{
    f64 threshold_lut[] = { 0.125 ,0.25, 0.5, 0.75 };
    f64 threshold = threshold_lut[m_nr21.wave_pattern_duty];
    f64 duration = PULSEA_REG_TO_DURATION(m_nr21.sound_length);
    f64 freq = PULSEA_REG_TO_FREQ(m_nr23_nr24.freq);

    f64 volume_end = m_nr22.envelope_direction ? 1.0f : 0.0f;
    f64 volume_start = (f64)m_nr22.initial_envelope_volume / 0xF;

    u8 sweep_count = m_nr22.number_of_envelope_sweep;
    bool stop = m_nr24.counter_selection;
    bool restart = m_nr24.initial;

    static size_t i = 0;
    if (i++ % 1000 == 0)
    {
        // LOG("PULSE B : envelop={}; threshold={}%; freq={}Hz; volume={}%-{}%\n",
        //     sweep_count, threshold, floor(freq), floor(volume_start*100), floor(volume_end*100));
    }

    auto func = [threshold] (f64 t) -> f64
    {
        f64 y = t < threshold ? 1.0 : -1.0;
        return y;
    };

    f64 volume = 0.3;
    volume_start *= volume;
    volume_end *= volume;

    Func1 envelop = sweep_count == 0
        ? constant(volume_start)
        : stepMap(volume_start, volume_end, 64 / sweep_count);

    // todo: freq sweep + use duration
    generateSamples(&m_ch2_prev_x, samples, sample_count, constant(freq), envelop, func);
}


Apu::Apu()
{
    m_audio_buffer_size = 0;

    m_nr10.raw = 0;
    m_nr11.raw = 0;
    m_nr12.raw = 0;
    m_nr13.raw = 0;
    m_nr14.raw = 0;

    m_nr21.raw = 0;
    m_nr22.raw = 0;
    m_nr23.raw = 0;
    m_nr24.raw = 0;

    m_nr31.raw = 0;
    m_nr32.raw = 0;
    m_nr33.raw = 0;
    m_nr34.raw = 0;

    m_ch1_prev_x = 0;
    m_ch2_prev_x = 0;
    m_ch3_prev_x = 0;
    m_ch4_prev_x = 0;

    // m_nr41.raw = 0;
    // m_nr42.raw = 0;
    // m_nr43.raw = 0;
    // m_nr44.raw = 0;

    initPlayer();
}

Apu::~Apu()
{
    destroyPlayer();
}

void Apu::mapMemory(Memory* mem)
{
    mem->mapRW(NR50_ADDR, &m_nr50);
    mem->mapRW(NR51_ADDR, &m_nr51);
    mem->mapRW(NR52_ADDR, &m_nr52, 1, 1 << 7);

    mem->mapRW(NR10_ADDR, &m_nr10);
    mem->mapRW(NR11_ADDR, &m_nr11); // todo: read mask
    mem->mapRW(NR12_ADDR, &m_nr12);
    mem->mapRW(NR13_ADDR, &m_nr13);
    mem->mapRW(NR14_ADDR, &m_nr14);

    mem->mapRW(NR21_ADDR, &m_nr21);
    mem->mapRW(NR22_ADDR, &m_nr22);
    mem->mapRW(NR23_ADDR, &m_nr23);
    mem->mapRW(NR24_ADDR, &m_nr24);

    mem->mapRW(NR31_ADDR, &m_nr31);
    mem->mapRW(NR32_ADDR, &m_nr32);
    mem->mapRW(NR33_ADDR, &m_nr33);
    mem->mapRW(NR34_ADDR, &m_nr34);

    mem->mapRW(W0_ADDR, &m_wave, sizeof(m_wave));
}

void Apu::step(size_t clocks_diff)
{
    m_clocks += clocks_diff;
    size_t sample_period = Timer::SYSTEM_FREQUENCY / AUDIO_SAMPLE_RATE;

    static size_t firstWrite = 0;
    if (m_clocks >= sample_period)
    {
        // size_t sample_count = m_clocks / sample_period;
        size_t sample_count = getDesiredBuffered() - getBuffered();
        m_clocks %= sample_period;

        decodeChannelPulseA(m_ch1_buffer + m_audio_buffer_size, sample_count);
        decodeChannelPulseB(m_ch2_buffer + m_audio_buffer_size, sample_count);
        decodeChannelWave(m_ch3_buffer + m_audio_buffer_size, sample_count);

        // mix
        for (size_t i = 0; i < sample_count * 2; i++)
        {
            s16 sample1 = m_ch1_buffer[m_audio_buffer_size + i];
            s16 sample2 = m_ch2_buffer[m_audio_buffer_size + i];
            s16 sample3 = m_ch3_buffer[m_audio_buffer_size + i];
            s16 sample4 = m_ch4_buffer[m_audio_buffer_size + i];
            m_audio_buffer[m_audio_buffer_size + i] = (sample1 + sample2 + sample3) / 3;
        }

        m_audio_buffer_size += sample_count * 2; // 2 channels


        // write out buffer
        // if (m_audio_buffer_size + sample_count * 2 > AUDIO_BUFFER_SIZE)
        {
            play(m_audio_buffer, m_audio_buffer_size * sizeof(s16));
            m_audio_buffer_size = 0;
        }

    }

}

}
