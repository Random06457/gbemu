#include "core/audio.hpp"

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>
#include <cassert>
#include "common/logging.hpp"
#include "common/fs.hpp"
#include <vector>
#include <cmath>

namespace gbemu::core
{

// TODO: remove
static SDL_AudioDeviceID s_device;

void Audio::initPlayer()
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        UNREACHABLE("SDL init error: {}", SDL_GetError());

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 2048;
    want.callback = nullptr;
    want.userdata = nullptr;
    s_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (s_device == 0)
        UNREACHABLE("SDL_OpenAudio error: {}", SDL_GetError());

    SDL_PauseAudioDevice(s_device, 0);
}

void Audio::destroyPlayer()
{
}

size_t Audio::getBuffered()
{
    return SDL_GetQueuedAudioSize(s_device) / (2 * sizeof(s16));
}

size_t Audio::getDesiredBuffered()
{
    return 2048;
}

static std::vector<u8> genWaveform(const void* buffer, size_t data_size)
{
    struct
    {
        char riff_magic[4]; // "RIFF"
        u32 wave_section_size; // -8
        char wave_magic[4]; // "WAVE"
        char fmt_magic[4]; // "fmt "
        u32 header_size;
        u16 format; // PCM=1
        u16 channel_count;
        u32 sample_rate;
        u32 stride;
        u16 byte_rate;
        u16 bits_per_sample;
        char data_magic[4]; // "data"
        u32 data_section_size;
    } PACKED wave_header;

    size_t byte_per_sample = 2;
    size_t channel_count = 2;

    size_t file_size = sizeof(wave_header) + data_size;

    std::memcpy(wave_header.riff_magic, "RIFF", 4);
    wave_header.wave_section_size = file_size - 8;
    std::memcpy(wave_header.wave_magic, "WAVE", 4);
    std::memcpy(wave_header.fmt_magic, "fmt ", 4);
    wave_header.header_size = 0x10;
    wave_header.format = 1; // PCM
    wave_header.channel_count = 2;
    wave_header.sample_rate = AUDIO_SAMPLE_RATE;
    wave_header.stride = AUDIO_SAMPLE_RATE * byte_per_sample * channel_count;
    wave_header.byte_rate = byte_per_sample * channel_count;
    wave_header.bits_per_sample = byte_per_sample * 8;
    std::memcpy(wave_header.data_magic, "data", 4);
    wave_header.data_section_size = data_size;

    std::vector<u8> output;
    output.resize(file_size);

    std::memcpy(output.data(), &wave_header, sizeof(wave_header));
    std::memcpy(output.data() + sizeof(wave_header), buffer, data_size);

    return output;
}



void Audio::play(const void* data, size_t size)
{
    SDL_QueueAudio(s_device, data, size);

    // static std::vector<s16> buff;
    // buff.resize(buff.size() + size/sizeof(s16));
    // s16* dst = buff.data() + buff.size() - size/sizeof(s16);
    // std::memcpy(dst, data, size);
    // static bool wrote = false;
    // if (buff.size() >= AUDIO_SAMPLE_RATE * 2 * 6 && !wrote)
    // {
    //     auto wave = genWaveform(buff.data(), buff.size() * sizeof(s16));
    //     File::writeAllBytes("audio.wav", wave.data(), wave.size());
    //     wrote = true;
    // }
}

}
