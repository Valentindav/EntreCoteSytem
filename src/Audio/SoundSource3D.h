#ifndef AUDIO_SoundSource3D_H_INCLUDED
#define AUDIO_SoundSource3D_H_INCLUDED

#include "Sound.h"

namespace audio
{
    class SoundSource3D : public Sound
    {
    public:
        SoundSource3D();
        virtual ~SoundSource3D();

        virtual HRESULT Init(WAVEFORMATEX* wfx, XAUDIO2_BUFFER* buffer, uint32 const id, float gain,
            Category category, IXAudio2SubmixVoice* submix = nullptr) override;

        void Play()                          override;
        void Playing(float deltaTime = 0)    override;
        void Pause()                         override;
        void Resume()                        override;
        void Stop()                          override;
        void SetVolume(float32 volume, float32 category = 1.0f) override;
        void Delay(float deltaTime)          override;

        // 3D positioning
        void SetEmitterPosition(float x, float y, float z);
        void SetListenerPosition(float x, float y, float z);
        void SetListenerForward(float x, float y, float z);

    private:
        // --- 3D audio internals ---
        void  Update3DAudio();
        float DistanceTo() const;
        void  ApplyDistanceVolume();
        void  ApplyDistanceDelay();
        void  ApplyStereoPanning();
        void  ApplyVelocity();

        // --- Orbit helpers ---
        void InitializeOrbitFromCurrentPosition();
        void CircleAroundListener(float deltaTime);

        // --- State ---
        Vector3f32 m_emitterPosition;
        Vector3f32 m_listenerPosition;
        Vector3f32 m_listenerForward;

        float m_minDistance;
        float m_maxDistance;
        float m_travelTime;
        float m_elapsedSincePlay;
        bool  m_waitingDelay;

        // --- Orbit ---
        Vector3f32 m_velocity;
        Vector3f32 m_orbitCenter;
        float      m_orbitRadius;
        float      m_orbitAngle;
        float      m_orbitSpeed;
    };
}

#endif 