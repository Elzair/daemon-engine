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

//TODO check all inputs for validity
namespace Audio {

    Log::Logger audioLogs("audio");

    static Cvar::Cvar<float> masterVolume("sound.volume.master", "the global audio volume", Cvar::ARCHIVE, 0.8f);

    //TODO make them the equivalent of LATCH and ROM for available*
    static Cvar::Cvar<std::string> deviceString("s_alDevice", "the OpenAL device to use", Cvar::ARCHIVE, "");
    static Cvar::Cvar<std::string> availableDevices("s_alAvailableDevices", "the available OpenAL devices", 0, "");

    static Cvar::Cvar<std::string> inputDeviceString("s_alInputDevice", "the OpenAL input device to use", Cvar::ARCHIVE, "");
    static Cvar::Cvar<std::string> availableInputDevices("s_alAvailableInputDevices", "the available input OpenAL devices", 0, "");

    // We mimic the behavior of the previous sound system by allowing only one looping ousnd per entity.
    // (and only one entities) CGame will add at each frame all the loops: if a loop hasn't been given
    // in a frame, it means it sould be destroyed.
    struct entityLoop_t {
        bool addedThisFrame;
        std::shared_ptr<LoopingSound> sound;
    };
    static entityLoop_t entityLoops[MAX_GENTITIES];

    static bool initialized = false;

    static AL::Device* device;
    static AL::Context* context;

    // Like in the previous sound system, we only have a single music
    std::shared_ptr<MusicSound> music;

    bool Init() {
        if (initialized) {
            return true;
        }

        // Initializes a device
        std::string deviceToTry = deviceString.Get();
        if (not deviceToTry.empty()) {
            device = AL::Device::FromName(deviceToTry);

            if (not device) {
                Log::Warn("Could not open OpenAL device '%s', trying the default device.", deviceToTry);
                device = AL::Device::GetDefaultDevice();
            }
        } else {
            device = AL::Device::GetDefaultDevice();
        }

        if (not device) {
            Log::Warn("Could not open an OpenAL device.");
            return false;
        }

        // Initializes a context
        context = AL::Context::GetDefaultContext(*device);

        if (not context) {
            delete device;
            device = nullptr;
            Log::Warn("Could not create an OpenAL context.");
            return false;
        }

        std::stringstream deviceList;
        for (auto deviceName : AL::Device::ListByName()) {
            deviceList << deviceName << "\n";
        }
        availableDevices.Set(deviceList.str());

        context->MakeCurrent();

        // Initializes the list of input devices

        std::stringstream inputDeviceList;
        for (auto inputDeviceName : AL::CaptureDevice::ListByName()) {
            inputDeviceList << inputDeviceName << "\n";
        }
        availableInputDevices.Set(inputDeviceList.str());

        initialized = true;

        // Initializes the rest of the audio system
        InitSamples();
        InitSounds();
        InitEmitters();

        for (int i = 0; i < MAX_GENTITIES; i++) {
            entityLoops[i] = {false, nullptr};
        }

        return true;
    }

    void Shutdown() {
        if (not initialized) {
            return;
        }

        // Shuts down the wrapper
        for (int i = 0; i < MAX_GENTITIES; i++) {
            if (entityLoops[i].sound) {
                entityLoops[i].sound->Stop();
            }
            entityLoops[i] = {false, nullptr};
        }

        StopMusic();

        // Shuts down the rest of the system
        // Shutdown sounds before emitters because they are using a effects defined in emitters
        // and that OpenAL checks the refcount before deleting the object.
        ShutdownSounds();
        ShutdownEmitters();
        ShutdownSamples();

        // Free OpenAL resources
        delete context;
        delete device;

        context = nullptr;
        device = nullptr;
    }

    void Update() {
        for (int i = 0; i < MAX_GENTITIES; i++) {
            if (entityLoops[i].sound and not entityLoops[i].addedThisFrame) {
                // The loop wasn't added this frame, that means it has to be removed.
                entityLoops[i].sound->FadeOutAndDie();
                entityLoops[i] = {false, nullptr};
            }
        }

        AL::SetListenerGain(masterVolume.Get());

        // Update the rest of the system
        UpdateEmitters();
        UpdateSounds();

        for (int i = 0; i < MAX_GENTITIES; i++) {
            entityLoops[i].addedThisFrame = false;
            // if we are the unique owner of a loop pointer, then it means it was stopped, free it.
            if (entityLoops[i].sound.unique()) {
                entityLoops[i] = {false, nullptr};
            }
        }
    }

    sfxHandle_t RegisterSFX(Str::StringRef filename) {
        return RegisterSample(filename)->GetHandle();
    }

    void StartSound(int entityNum, const vec3_t origin, sfxHandle_t sfx) {
        std::shared_ptr<Emitter> emitter;

        // Apparently no origin means it is attached to an entity
        if (not origin) {
            emitter = GetEmitterForEntity(entityNum);

        } else {
            emitter = GetEmitterForPosition(origin);
        }

        AddSound(emitter, std::make_shared<OneShotSound>(Sample::FromHandle(sfx)), 1);
    }

    void StartLocalSound(sfxHandle_t sfx) {
        AddSound(GetLocalEmitter(), std::make_shared<OneShotSound>(Sample::FromHandle(sfx)), 1);
    }

    void AddEntityLoopingSound(int entityNum, sfxHandle_t sfx) {
        entityLoop_t& loop = entityLoops[entityNum];

        loop.addedThisFrame = true;

        if (not loop.sound) {
            loop.sound = std::make_shared<LoopingSound>(Sample::FromHandle(sfx));
            AddSound(GetEmitterForEntity(entityNum), loop.sound, 1);
        }
    }

    void ClearAllLoopingSounds() {
        for (int i = 0; i < MAX_GENTITIES; i++) {
            ClearLoopingSoundsForEntity(i);
        }
    }

    void ClearLoopingSoundsForEntity(int entityNum) {
        if (entityLoops[entityNum].sound) {
            entityLoops[entityNum].addedThisFrame = false;
        }
    }

    void StartMusic(Str::StringRef leadingSound, Str::StringRef loopSound) {
        StopMusic();
        music = std::make_shared<MusicSound>(leadingSound, loopSound);
        AddSound(GetLocalEmitter(), music, 1);
    }

    void StopMusic() {
        if (music) {
            music->Stop();
        }
        music = nullptr;
    }

    void UpdateListener(int entityNum, vec3_t orientation[3]) {
        UpdateListenerEntity(entityNum, orientation);
    }

    void UpdateEntityPosition(int entityNum, const vec3_t position) {
        UpdateRegisteredEntityPosition(entityNum, position);
    }

    void UpdateEntityVelocity(int entityNum, const vec3_t velocity) {
        UpdateRegisteredEntityVelocity(entityNum, velocity);
    }

    void UpdateEntityOcclusion(int entityNum, float ratio) {
        UpdateRegisteredEntityOcclusion(entityNum, ratio);
    }

    static AL::CaptureDevice* capture = nullptr;

    void StartCapture(int rate) {
        if (capture) {
            return;
        }

        capture = AL::CaptureDevice::GetDefaultDevice(rate);
        capture->Start();
    }

    int AvailableCaptureSamples() {
        return capture->GetNumCapturedSamples();
    }

    void GetCapturedData(int numSamples, void* buffer) {
        capture->Capture(numSamples, buffer);
    }

    void StopCapture() {
        capture->Stop();
        delete capture;
        capture = nullptr;
    }
}
