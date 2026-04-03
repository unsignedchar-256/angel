// main.c

#include <cstdint>

#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <thread>

#include <alsa/asoundlib.h>

std::array lyrics = std::to_array<std::pair<uint32_t, std::string_view>>({
    {0x0000U, "Like an old song"},
    {0x0C88U, "Our story today"},
    {0x1552U, "Like a diary book in a cramped library"},
    {0x2CEEU, "Inside of our memory"},
    {0x35B8U, "Let's remember it today"},
    {0x3FE2U, "So let's look outside"},
    {0x4FA2U, "and remember today"},
    {0x521CU, "Happy birthday to you"},
    {0x5F48U, "For you"},
    {0x6769U, "Happy birthday to you"},
    {0x7C14U, "Like a present for me"},
    {0x89ECU, "Let's celebrate it"},
    {0x92B0U, "Happy birthday to you"},
    {0xA228U, "I wish you the best"},
    {0xAD88U, "Da ra ra ra ra ra ra ra"},
    {0xB6D8U, "Like today"},
    {0xBC46U, "Like now"},
    {0xC2F2U, "Da ra ra ra ra ra ra ra"},
    {0xCCD8U, "I hope you happy everyday, ooh"},
    {0xD870U, "Da ra ra ra ra ra ra ra"},
    {0xE24CU, "Oh every day, like everyday"},
    {0xEE5AU, "Da ra ra ra ra ra ra ra"},
    {0xF8C4U, "I wish you the best"}
});

std::vector<uint8_t> audioBuffer;
snd_pcm_t* device = nullptr;

void clearScreen() {
    for (uint8_t i = 0x00U; i < 0x40U; ++i) {
        for (int x = 0x00U; x < 0xFFU; ++x) {
            std::cout << "";
        }

        std::cout << "\n";
    }
}

void messageForAngel() {
    std::cout << "\n\n";
    std::cout << "Happy birthday, Angel~\n";
    std::cout << "I wish you the best, for the next sweet 17, for the next 18, and so on. I hope you will always be happy, and have a life worth remembering.\n\n";

    std::cout << "  \"A sign where a life is worth remembering is when there are highs and lows.\"\n  - @unsignedchar256 at 2026.4.4 0:08:16 (UTC+7)\n";
}

void lyricsThread() {
    clearScreen();
    std::cout << "[Im Na-Yeon \"Happy Birthday To You\"]\n\n";

    for (uint8_t i = 0x00U; i < lyrics.size(); i++) {
        std::vector<std::string> words = { "" };

        for (const char character : lyrics.at(i).second) {
            if (character != ' ') {
                words.back().insert(words.back().end(), character);
            } else {
                words.insert(words.end(), "");
            }
        }

        for (uint8_t x = 0x00U; x < words.size(); x++) {
            std::cout << words.at(x);
            std::cout.flush();

            if (x < (words.size() - 0x01U)) {
                std::cout << ' ';
                std::cout.flush();

                std::this_thread::sleep_for(std::chrono::milliseconds(0x60U));
            } else {
                std::cout << '\n';
                std::cout.flush();
            }
        }

        // std::cout << lyrics.at(i).second << '\n';

        if (i < (lyrics.size() - 0x01U)) {
            uint16_t delays = (lyrics.at((i + 0x01U)).first - lyrics.at(i).first) - ((words.size() - 0x01U) * 0x60U);
            std::this_thread::sleep_for(std::chrono::milliseconds(delays));
        }
    }

    messageForAngel();
}

void readFile() {
    std::cout << "reserving 18418002 bytes...\n";
    audioBuffer.reserve(0x1190952U);

    std::ifstream file = std::ifstream("raw.bin", (std::ios::binary | std::ios::in));
    uint8_t* buffer = new uint8_t[0x1000U];

    while ((file.read(reinterpret_cast<char*>(buffer), 0x1000U) || file.gcount() > 0x00)) {
        size_t readSize = file.gcount();
        std::cout << "read " << readSize << " bytes\n";

        audioBuffer.insert(audioBuffer.end(), buffer, (buffer + readSize));
    }

    delete[] buffer;
}

void initializeSoundInstance() {
    if (snd_pcm_open(&device, "default", SND_PCM_STREAM_PLAYBACK, 0x00) < 0x00) {
        std::cout << "couldn't open the device\n";
        return;
    }

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(device, params);

    std::cout << "bytes are interleaved\n";
    snd_pcm_hw_params_set_access(device, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    std::cout << "samples are packed little endian signed 24-bit PCM\n";
    snd_pcm_hw_params_set_format(device, params, SND_PCM_FORMAT_S24_3LE);
    std::cout << "sampling rate 44100 kHz\n";
    snd_pcm_hw_params_set_rate(device, params, 0xAC44U, 0x00);
    std::cout << "stereo channels\n";
    snd_pcm_hw_params_set_channels(device, params, 0x02U);

    if (snd_pcm_hw_params(device, params) < 0x00) {
        std::cout << "couldn't set hardware parameters\n";
        return;
    }

    if (snd_pcm_prepare(device) < 0x00) {
        std::cout << "couldn't prepare device\n";
        return;
    }

    std::cout << "PCM state " << snd_pcm_state(device) << "\n";
}

void soundThread() {

    uint32_t totalFrames = (audioBuffer.size() / (0x03U * 0x02U));
    uint32_t writtenFrames = 0x00U;
    uint32_t bufferSize = (0x4000U * (0x03U * 0x02U));

    while (writtenFrames < totalFrames) {
        uint32_t toBeWritten = bufferSize;
        if ((writtenFrames + bufferSize) > bufferSize) {
            toBeWritten = (totalFrames - writtenFrames);
        }

        snd_pcm_sframes_t result = snd_pcm_writei(device, (audioBuffer.data() + (writtenFrames * (0x03U * 0x02U))), toBeWritten);

        if (result == -EPIPE) {
            snd_pcm_prepare(device);
        } else if (result < 0x00) {
            std::cout << "I/O result result " << result << "\n";

            result = snd_pcm_recover(device, result, 0x00);
            std::cout << "recover result result " << result << "\n";

            if (result < 0x00) {
                continue;
            }
        }

        writtenFrames += toBeWritten;

    }

    snd_pcm_drain(device);
    snd_pcm_close(device);
}

int main() {
    readFile();
    initializeSoundInstance();

    std::thread lyricsInstance = std::thread(lyricsThread);
    lyricsInstance.detach();

    std::thread soundInstance = std::thread(soundThread);
    soundInstance.join();

    return 0x00U;
}
