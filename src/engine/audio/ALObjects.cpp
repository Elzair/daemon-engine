/*
===========================================================================

daemon gpl source code
copyright (c) 2013 unvanquished developers

this file is part of the daemon gpl source code (daemon source code).

daemon source code is free software: you can redistribute it and/or modify
it under the terms of the gnu general public license as published by
the free software foundation, either version 3 of the license, or
(at your option) any later version.

daemon source code is distributed in the hope that it will be useful,
but without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.  see the
gnu general public license for more details.

you should have received a copy of the gnu general public license
along with daemon source code.  if not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "AudioPrivate.h"

#define AL_ALEXT_PROTOTYPES
#include <al.h>
#include <alc.h>
#include <efx.h>
#include <efx-presets.h>

namespace Audio {
namespace AL {

    std::string ALErrorToString(int error) {
        switch (error) {
            case AL_NO_ERROR:
                return "No Error";

            case AL_INVALID_NAME:
                return "Invalid name";

            case AL_INVALID_ENUM:
                return "Invalid enum";

            case AL_INVALID_VALUE:
                return "Invalid value";

            case AL_INVALID_OPERATION:
                return "Invalid operation";

            case AL_OUT_OF_MEMORY:
                return "Out of memory";

            default:
                return "Unknown OpenAL error";
        }
    }

    // Used to avoid unnecessary calls to alGetError
    static Cvar::Cvar<bool> checkAllCalls("sound.al.checkAllCalls", "check all OpenAL calls for errors", Cvar::ARCHIVE, false);
    int ClearALError(int line = -1) {
        int error = alGetError();

        if (line >= 0) {
            if (error != AL_NO_ERROR) {
                audioLogs.Warn("Unhandled OpenAL error on line %i: %s", line, ALErrorToString(error));
            }
        } else if (checkAllCalls.Get()) {
            audioLogs.Warn("Unhandled OpenAL error: %s", ALErrorToString(error));
        }

        return error;
    }

    void CheckALError(int line) {
        if (checkAllCalls.Get()) {
            ClearALError(line);
        }
    }

    #define CHECK_AL_ERROR() CheckALError(__LINE__)

    // Converts an snd_codec_t format to an OpenAL format
    ALuint Format(int width, int channels) {
        if (width == 1) {

            if (channels == 1) {
                return AL_FORMAT_MONO8;
            } else if (channels == 2) {
                return AL_FORMAT_STEREO8;
            }

        } else if (width == 2) {

            if (channels == 1) {
                return AL_FORMAT_MONO16;
            } else if (channels == 2) {
                return AL_FORMAT_STEREO16;
            }
        }

        return AL_FORMAT_MONO16;
    }

    // Buffer implementation

    Buffer::Buffer() {
        alGenBuffers(1, &alHandle);
        CHECK_AL_ERROR();
    }

    Buffer::Buffer(Buffer&& other) {
        alHandle = other.alHandle;
        other.alHandle = 0;
    }

    Buffer::~Buffer() {
        if (alHandle != 0) {
            alDeleteBuffers(1, &alHandle);
            CHECK_AL_ERROR();
        }
        alHandle = 0;
    }

    unsigned Buffer::Feed(snd_info_t info, void* data) {
        ALuint format = Format(info.width, info.channels);

        CHECK_AL_ERROR();
		alBufferData(alHandle, format, data, info.size, info.rate);

        return ClearALError();
    }

    Buffer::Buffer(unsigned handle): alHandle(handle) {
    }

    unsigned Buffer::Acquire() {
        unsigned temp = alHandle;
        alHandle = 0;
        return temp;
    }

    Buffer::operator unsigned() const {
        return alHandle;
    }

    // ReverbEffectPreset implementation

    struct ReverbEffectPreset {
        ReverbEffectPreset(EFXEAXREVERBPROPERTIES builtinPreset);

        float density;
        float diffusion;
        float gain;
        float gainHF;
        float gainLF;
        float decayTime;
        float decayHFRatio;
        float decayLFRatio;
        float reflectionsGain;
        float reflectionsDelay;
        float lateReverbGain;
        float lateReverbDelay;
        float echoTime;
        float echoDepth;
        float modulationTime;
        float modulationDepth;
        float airAbsorptionGainHF;
        float HFReference;
        float LFReference;
        bool decayHFLimit;
    };

    ReverbEffectPreset::ReverbEffectPreset(EFXEAXREVERBPROPERTIES builtinPreset):
        density(builtinPreset.flDensity),
        diffusion(builtinPreset.flDiffusion),
        gain(builtinPreset.flGain),
        gainHF(builtinPreset.flGainHF),
        gainLF(builtinPreset.flGainLF),
        decayTime(builtinPreset.flDecayTime),
        decayHFRatio(builtinPreset.flDecayHFRatio),
        decayLFRatio(builtinPreset.flDecayLFRatio),
        reflectionsGain(builtinPreset.flReflectionsGain),
        reflectionsDelay(builtinPreset.flReflectionsDelay),
        lateReverbGain(builtinPreset.flLateReverbGain),
        lateReverbDelay(builtinPreset.flLateReverbDelay),
        echoTime(builtinPreset.flEchoTime),
        echoDepth(builtinPreset.flEchoDepth),
        modulationTime(builtinPreset.flModulationTime),
        modulationDepth(builtinPreset.flModulationDepth),
        airAbsorptionGainHF(builtinPreset.flAirAbsorptionGainHF),
        HFReference(builtinPreset.flHFReference),
        LFReference(builtinPreset.flLFReference),
        decayHFLimit(builtinPreset.iDecayHFLimit) {
    }

    ReverbEffectPreset& GetHangarEffectPreset() {
        static ReverbEffectPreset preset(EFX_REVERB_PRESET_HANGAR);
        return preset;
    }

    // Effect implementation

    Effect::Effect() {
        alGenEffects(1, &alHandle);
        CHECK_AL_ERROR();
    }

    Effect::Effect(Effect&& other) {
        alHandle = other.alHandle;
        other.alHandle = 0;
    }

    Effect::~Effect() {
        if (alHandle != 0) {
            alDeleteEffects(1, &alHandle);
            CHECK_AL_ERROR();
        }
        alHandle = 0;
    }

    void Effect::MakeReverb() {
        alEffecti(alHandle, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbDensity(float density){
        alEffectf(alHandle, AL_EAXREVERB_DENSITY, density);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbDiffusion(float diffusion) {
        alEffectf(alHandle, AL_EAXREVERB_DIFFUSION, diffusion);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbGain(float gain) {
        alEffectf(alHandle, AL_EAXREVERB_GAIN, gain);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbGainHF(float gain) {
        alEffectf(alHandle, AL_EAXREVERB_GAINHF, gain);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbGainLF(float gain) {
        alEffectf(alHandle, AL_EAXREVERB_GAINLF, gain);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbDecayTime(float time) {
        alEffectf(alHandle, AL_EAXREVERB_DECAY_TIME, time);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbDecayHFRatio(float ratio) {
        alEffectf(alHandle, AL_EAXREVERB_DECAY_HFRATIO, ratio);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbDecayLFRatio(float ratio) {
        alEffectf(alHandle, AL_EAXREVERB_DECAY_LFRATIO, ratio);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbReflectionsGain(float gain) {
        alEffectf(alHandle, AL_EAXREVERB_REFLECTIONS_GAIN, gain);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbReflectionsDelay(float delay) {
        alEffectf(alHandle, AL_EAXREVERB_REFLECTIONS_DELAY, delay);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbLateReverbGain(float gain) {
        alEffectf(alHandle, AL_EAXREVERB_LATE_REVERB_GAIN, gain);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbLateReverbDelay(float delay) {
        alEffectf(alHandle, AL_EAXREVERB_LATE_REVERB_DELAY, delay);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbEchoTime(float time) {
        alEffectf(alHandle, AL_EAXREVERB_ECHO_TIME, time);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbEchoDepth(float depth) {
        alEffectf(alHandle, AL_EAXREVERB_ECHO_DEPTH, depth);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbModulationTime(float time) {
        alEffectf(alHandle, AL_EAXREVERB_MODULATION_TIME, time);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbModulationDepth(float depth) {
        alEffectf(alHandle, AL_EAXREVERB_MODULATION_DEPTH, depth);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbAirAbsorptionGainHF(float gain) {
        alEffectf(alHandle, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, gain);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbHFReference(float reference) {
        alEffectf(alHandle, AL_EAXREVERB_HFREFERENCE, reference);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbLFReference(float reference) {
        alEffectf(alHandle, AL_EAXREVERB_LFREFERENCE, reference);
        CHECK_AL_ERROR();
    }

    void Effect::SetReverbDelayHFLimit(bool delay) {
        alEffecti(alHandle, AL_EAXREVERB_DECAY_HFLIMIT, delay);
        CHECK_AL_ERROR();
    }

    void Effect::ApplyReverbPreset(ReverbEffectPreset& preset) {
        MakeReverb();
        SetReverbDensity(preset.density);
        SetReverbDiffusion(preset.diffusion);
        SetReverbGain(preset.gain);
        SetReverbGainHF(preset.gainHF);
        SetReverbGainLF(preset.gainLF);
        SetReverbDecayTime(preset.decayTime);
        SetReverbDecayHFRatio(preset.decayHFRatio);
        SetReverbDecayLFRatio(preset.decayLFRatio);
        SetReverbReflectionsGain(preset.reflectionsGain);
        SetReverbReflectionsDelay(preset.reflectionsDelay);
        SetReverbLateReverbGain(preset.lateReverbGain);
        SetReverbLateReverbDelay(preset.lateReverbDelay);
        SetReverbEchoTime(preset.echoTime);
        SetReverbEchoDepth(preset.echoDepth);
        SetReverbModulationTime(preset.modulationTime);
        SetReverbModulationDepth(preset.modulationDepth);
        SetReverbAirAbsorptionGainHF(preset.airAbsorptionGainHF);
        SetReverbHFReference(preset.HFReference);
        SetReverbLFReference(preset.LFReference);
        SetReverbDelayHFLimit(preset.decayHFLimit);
    }

    Effect::operator unsigned() const {
        return alHandle;
    }

    // EffectSlot Implementation

    EffectSlot::EffectSlot() {
        alGenAuxiliaryEffectSlots(1, &alHandle);
        CHECK_AL_ERROR();
    }

    EffectSlot::EffectSlot(EffectSlot&& other) {
        alHandle = other.alHandle;
        other.alHandle = 0;
    }

    EffectSlot::~EffectSlot() {
        if (alHandle != 0) {
            alDeleteAuxiliaryEffectSlots(1, &alHandle);
            CHECK_AL_ERROR();
        }
        alHandle = 0;
    }

    void EffectSlot::SetGain(float gain) {
        alAuxiliaryEffectSlotf(alHandle, AL_EFFECTSLOT_GAIN, gain);
        CHECK_AL_ERROR();
    }

    void EffectSlot::SetEffect(Effect& effect) {
        alAuxiliaryEffectSloti(alHandle, AL_EFFECTSLOT_EFFECT, effect);
        CHECK_AL_ERROR();
    }

    EffectSlot::operator unsigned() const {
        return alHandle;
    }

    // Listener function implementations

    void SetListenerGain(float gain) {
        alListenerf(AL_GAIN, gain);
        CHECK_AL_ERROR();
    }

    void SetListenerPosition(const vec3_t position) {
        alListenerfv(AL_POSITION, position);
        CHECK_AL_ERROR();
    }

    void SetListenerVelocity(const vec3_t velocity) {
        alListenerfv(AL_VELOCITY, velocity);
        CHECK_AL_ERROR();
    }

    void SetListenerOrientation(const vec3_t orientation[3]) {
        float alOrientation[6] = {
            orientation[0][0],
            orientation[0][1],
            orientation[0][2],
            orientation[2][0],
            orientation[2][1],
            orientation[2][2]
        };

        alListenerfv(AL_ORIENTATION, alOrientation);
        CHECK_AL_ERROR();
    }

    // OpenAL state functions

    void SetSpeedOfSound(float speed) {
        alSpeedOfSound(speed);
        CHECK_AL_ERROR();
    }

    void SetDopplerExaggerationFactor(float factor) {
        alDopplerFactor(factor);
        CHECK_AL_ERROR();
    }

    // Source implementation

    Source::Source() {
        alGenSources(1, &alHandle);
        CHECK_AL_ERROR();
    }

    Source::Source(Source&& other) {
        alHandle = other.alHandle;
        other.alHandle = 0;
    }

    Source::~Source() {
        RemoveAllQueuedBuffers();
        if (alHandle != 0) {
            alDeleteSources(1, &alHandle);
            CHECK_AL_ERROR();
        }
        alHandle = 0;
    }

    void Source::Play() {
        alSourcePlay(alHandle);
        CHECK_AL_ERROR();
    }

    void Source::Pause() {
        alSourcePause(alHandle);
        CHECK_AL_ERROR();
    }

    void Source::Stop() {
        alSourceStop(alHandle);
        CHECK_AL_ERROR();
    }

    bool Source::IsStopped() {
        ALint state;
        alGetSourcei(alHandle, AL_SOURCE_STATE, &state);
        CHECK_AL_ERROR();
        return state == AL_STOPPED;
    }

    void Source::SetBuffer(Buffer& buffer) {
        alSourcei(alHandle, AL_BUFFER, buffer);
        CHECK_AL_ERROR();
    }

    void Source::QueueBuffer(Buffer buffer) {
        ALuint bufHandle = buffer.Acquire();
        alSourceQueueBuffers(alHandle, 1, &bufHandle);
        CHECK_AL_ERROR();
    }

    int Source::GetNumProcessedBuffers() {
        int res;
        alGetSourcei(alHandle, AL_BUFFERS_PROCESSED, &res);
        CHECK_AL_ERROR();
        return res;
    }

    int Source::GetNumQueuedBuffers() {
        int res;
        alGetSourcei(alHandle, AL_BUFFERS_QUEUED, &res);
        CHECK_AL_ERROR();
        return res;
    }

    Buffer Source::PopBuffer() {
        unsigned bufferHandle;
        alSourceUnqueueBuffers(alHandle, 1, &bufferHandle);
        CHECK_AL_ERROR();
        return {bufferHandle};
    }

    void Source::RemoveAllQueuedBuffers() {
        // OpenAL gives an error if the source isn't stopped or if it is an AL_STATIC source.
        if (GetType() != AL_STREAMING) {
            return;
        }
        Stop();
        int toBeRemoved = GetNumQueuedBuffers();
        while (toBeRemoved --> 0) {
            // The buffer will be copied and destroyed immediately as well as it's OpenAL buffer
            PopBuffer();
        }
    }

    void Source::ResetBuffer() {
        alSourcei(alHandle, AL_BUFFER, 0);
        CHECK_AL_ERROR();
    }

    void Source::SetGain(float gain) {
        alSourcef(alHandle, AL_GAIN, gain);
        CHECK_AL_ERROR();
    }

    void Source::SetPosition(const vec3_t position) {
        alSourcefv(alHandle, AL_POSITION, position);
        CHECK_AL_ERROR();
    }

    void Source::SetVelocity(const vec3_t velocity) {
        alSourcefv(alHandle, AL_VELOCITY, velocity);
        CHECK_AL_ERROR();
    }

    void Source::SetLooping(bool loop) {
        alSourcei(alHandle, AL_LOOPING, loop);
        CHECK_AL_ERROR();
    }

    void Source::SetRolloff(float factor) {
        alSourcef(alHandle, AL_ROLLOFF_FACTOR, factor);
        CHECK_AL_ERROR();
    }

    void Source::SetReferenceDistance(float distance) {
        alSourcef(alHandle, AL_REFERENCE_DISTANCE, distance);
        CHECK_AL_ERROR();
    }

    void Source::SetRelative(bool relative) {
        alSourcei(alHandle, AL_SOURCE_RELATIVE, relative);
        CHECK_AL_ERROR();
    }

    void Source::EnableEffect(int slot, EffectSlot& effect) {
        alSource3i(alHandle, AL_AUXILIARY_SEND_FILTER, effect, slot, AL_FILTER_NULL);
        CHECK_AL_ERROR();
    }

    void Source::DisableEffect(int slot) {
        alSource3i(alHandle, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, slot, AL_FILTER_NULL);
        CHECK_AL_ERROR();
    }

    Source::operator unsigned() const {
        return alHandle;
    }

    int Source::GetType() {
        ALint type;
        alGetSourcei(alHandle, AL_SOURCE_TYPE, &type);
        CHECK_AL_ERROR();
        return type;
    }

    // Implementation of Device

    Device* Device::FromName(Str::StringRef name) {
        ALCdevice* alHandle = alcOpenDevice(name.c_str());
        if (alHandle) {
            return new Device(alHandle);
        } else {
            return nullptr;
        }
    }

    Device* Device::GetDefaultDevice() {
        ALCdevice* alHandle = alcOpenDevice(nullptr);
        if (alHandle) {
            return new Device(alHandle);
        } else {
            return nullptr;
        }
    }

    Device::Device(Device&& other) {
        alHandle = other.alHandle;
        other.alHandle = nullptr;
    }

    Device::~Device() {
        if (alHandle) {
            alcCloseDevice((ALCdevice*)alHandle);
        }
        alHandle = nullptr;
    }

    Device::operator void*() {
        return alHandle;
    }

    std::vector<std::string> Device::ListByName() {
        const char* list = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);

        if (!list) {
            return {};
        }

        // OpenAL gives the concatenation of null-terminated strings, followed by a '\0' (it ends with a double '\0')
        std::vector<std::string> res;
        while (*list) {
            res.push_back(list);
            list += res.back().size() + 1;
        }

        return res;
    }

    Device::Device(void* alHandle): alHandle(alHandle) {
    }

    // Implementation of Context

    Context* Context::GetDefaultContext(Device& device) {
        ALCcontext* alHandle = alcCreateContext((ALCdevice*)(void*)device, nullptr);
        if (alHandle) {
            return new Context(alHandle);
        } else {
            return nullptr;
        }
    }

    Context::Context(Context&& other) {
        alHandle = other.alHandle;
        other.alHandle = nullptr;
    }

    Context::~Context() {
        if (alHandle) {
            alcDestroyContext((ALCcontext*)alHandle);
        }
        alHandle = nullptr;
    }

    void Context::MakeCurrent() {
        alcMakeContextCurrent((ALCcontext*)alHandle);
    }

    Context::operator void*() {
        return alHandle;
    }

    Context::Context(void* alHandle): alHandle(alHandle) {
    }

    // Implementation of CaptureDevice

    CaptureDevice* CaptureDevice::FromName(Str::StringRef name, int rate) {
        ALCdevice* alHandle = alcCaptureOpenDevice(name.c_str(), rate, AL_FORMAT_MONO16, 2 * rate / 16);
        if (alHandle) {
            return new CaptureDevice(alHandle);
        } else {
            return nullptr;
        }
    }

    CaptureDevice* CaptureDevice::GetDefaultDevice(int rate) {
        ALCdevice* alHandle = alcCaptureOpenDevice(DefaultDeviceName().c_str(), rate, AL_FORMAT_MONO16, 2 * rate / 16);
        if (alHandle) {
            return new CaptureDevice(alHandle);
        } else {
            return nullptr;
        }
    }

    CaptureDevice::CaptureDevice(CaptureDevice&& other) {
        alHandle = other.alHandle;
        other.alHandle = nullptr;
    }

    CaptureDevice::~CaptureDevice() {
        if (alHandle) {
            alcCaptureCloseDevice((ALCdevice*)alHandle);
        }
        alHandle = nullptr;
    }

    void CaptureDevice::Start() {
        alcCaptureStart((ALCdevice*)alHandle);
    }

    void CaptureDevice::Stop() {
        alcCaptureStop((ALCdevice*)alHandle);
    }

    int CaptureDevice::GetNumCapturedSamples() {
        int numSamples = 0;
        alcGetIntegerv((ALCdevice*)alHandle, ALC_CAPTURE_SAMPLES, sizeof(numSamples), &numSamples);
        return numSamples;
    }

    void CaptureDevice::Capture(int numSamples, void* buffer) {
        alcCaptureSamples((ALCdevice*)alHandle, buffer, numSamples);
    }

    CaptureDevice::operator void*() {
        return alHandle;
    }

    std::string CaptureDevice::DefaultDeviceName() {
        return alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    }

    std::vector<std::string> CaptureDevice::ListByName() {
        const char* list = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

        if (!list) {
            return {};
        }

        // OpenAL gives the concatenation of null-terminated strings, followed by a '\0' (it ends with a double '\0')
        std::vector<std::string> res;
        while (*list) {
            res.push_back(list);
            list += res.back().size() + 1;
        }

        return res;
    }

    CaptureDevice::CaptureDevice(void* alHandle): alHandle(alHandle) {
    }

}
}
