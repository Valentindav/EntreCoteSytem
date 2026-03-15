#include "SoundSource3D.h"
#include "AudioPlayer.h"
#include <iostream>

namespace audio
{
    /////////////////////////////////////
    /// @brief Default constructor.
    /////////////////////////////////////
    SoundSource3D::SoundSource3D() : Sound()
    {
        m_emitterPosition = {};
        m_listenerPosition = {};
        m_listenerForward = {};
        m_minDistance = 0.0f;
        m_maxDistance = 0.0f;
        m_volume = 1.0f;
        m_travelTime = 0.0f;
        m_elapsedSincePlay = 0.0f;
        m_waitingDelay = false;

        m_velocity = {};
        m_orbitCenter = {};
        m_orbitRadius = 0.0f;
        m_orbitAngle = 0.0f;
        m_orbitSpeed = 1.0f;
    }

    SoundSource3D::~SoundSource3D() {}

    /////////////////////////////////////
    /// @brief Initializes the 3D sound source.
    /////////////////////////////////////
    HRESULT SoundSource3D::Init(WAVEFORMATEX* wfx, XAUDIO2_BUFFER* buffer, uint32 const id, float gain,
        Category category, IXAudio2SubmixVoice* submix)
    {
        Sound::Init(wfx, buffer, id, gain, category, submix);
        m_minDistance = 0.0f;
        m_maxDistance = 80.0f;
        return S_OK;
    }

    /////////////////////////////////////
    /// @brief Starts 3D playback — sets up orbit then waits for travel-time delay.
    /////////////////////////////////////
    void SoundSource3D::Play()
    {
        SetEmitterPosition(-50.0f, 0.0f, 0.0f);
        InitializeOrbitFromCurrentPosition();
        Update3DAudio();
        m_elapsedSincePlay = 0.0f;
        m_waitingDelay = true;
        SetState(AudioState::DELAY);
    }

    /////////////////////////////////////
    /// @brief Per-frame update: orbits the emitter and refreshes 3D effects.
    /////////////////////////////////////
    void SoundSource3D::Playing(float deltaTime)
    {
        CircleAroundListener(deltaTime);
        Update3DAudio();
    }

    void SoundSource3D::Pause()
    {
        if (m_pSourceVoice)
        {
            m_pSourceVoice->Stop(0);
            SetState(AudioState::PAUSE);
        }
    }

    void SoundSource3D::Resume()
    {
        if (m_pSourceVoice)
        {
            m_pSourceVoice->Start(0);
            SetState(AudioState::PLAYING);
        }
    }

    void SoundSource3D::Stop()
    {
        if (m_pSourceVoice)
        {
            m_pSourceVoice->Stop(0);
            SetState(AudioState::STOP);
        }
    }

    void SoundSource3D::SetVolume(float32 volume, float32 category)
    {
        m_volume = volume;
        if (m_pSourceVoice)
        {
            m_pSourceVoice->SetVolume(category * volume * m_gain);
        }
    }

    /////////////////////////////////////
    /// @brief Waits for sound-travel delay before starting playback.
    /////////////////////////////////////
    void SoundSource3D::Delay(float deltaTime)
    {
        if (m_waitingDelay)
        {
            m_elapsedSincePlay += deltaTime;
            if (m_elapsedSincePlay >= m_travelTime)
            {
                m_pSourceVoice->Start(0);
                m_waitingDelay = false;
                SetState(AudioState::PLAYING);
            }
        }
    }

    // -----------------------------------------------------------------------
    //  Position setters
    // -----------------------------------------------------------------------

    void SoundSource3D::SetEmitterPosition(float x, float y, float z)
    {
        m_emitterPosition = { x, y, z };
    }

    void SoundSource3D::SetListenerPosition(float x, float y, float z)
    {
        m_listenerPosition = { x, y, z };
    }

    /// @note The forward vector is normalised automatically.
    void SoundSource3D::SetListenerForward(float x, float y, float z)
    {
        m_listenerForward = Vector3f32(x, y, z).Normalize();
    }

    // -----------------------------------------------------------------------
    //  3D audio pipeline
    // -----------------------------------------------------------------------

    void SoundSource3D::Update3DAudio()
    {
        ApplyDistanceVolume();
        ApplyDistanceDelay();
        ApplyStereoPanning();
        ApplyVelocity();
    }

    float SoundSource3D::DistanceTo() const
    {
        return (m_emitterPosition - m_listenerPosition).Norm();
    }

    /////////////////////////////////////
    /// @brief Inverse-square attenuation based on distance.
    /////////////////////////////////////
    void SoundSource3D::ApplyDistanceVolume()
    {
        if (!m_pSourceVoice) return;

        float distance = DistanceTo();
        float t = (distance - m_minDistance) / (m_maxDistance - m_minDistance);
        float attenuation = 1.0f / (1.0f + t * t * 16.0f);
        float finalVolume = Clamp(attenuation * m_volume, 0.0f, 1.0f);

        m_pSourceVoice->SetVolume(finalVolume);
    }

    /////////////////////////////////////
    /// @brief Computes sound travel time from distance / speed-of-sound.
    /////////////////////////////////////
    void SoundSource3D::ApplyDistanceDelay()
    {
        if (!m_pSourceVoice) return;
        m_travelTime = DistanceTo() / 343.0f;
    }

    /////////////////////////////////////
    /// @brief Maps emitter angle relative to listener into a stereo L/R matrix.
    /////////////////////////////////////
    void SoundSource3D::ApplyStereoPanning()
    {
        if (!m_pSourceVoice) return;

        Vector3f32 toEmitter = (m_emitterPosition - m_listenerPosition).Normalize();
        Vector3f32 up = Vector3f32::Up();
        Vector3f32 right = up.CrossProduct(m_listenerForward).Normalize();

        float pan = Clamp(toEmitter.DotProduct(right), -1.0f, 1.0f);
        float angle = (pan + 1.0f) * (PI * 0.25f);

        float leftVolume = Cos(angle);
        float rightVolume = Sin(angle);

        XAUDIO2_VOICE_DETAILS details;
        m_pSourceVoice->GetVoiceDetails(&details);

        if (details.InputChannels == 1)
        {
            float matrix[2] = { leftVolume, rightVolume };
            m_pSourceVoice->SetOutputMatrix(nullptr, 1, 2, matrix);
        }
    }

    /////////////////////////////////////
    /// @brief Applies Doppler pitch shift from emitter velocity.
    /////////////////////////////////////
    void SoundSource3D::ApplyVelocity()
    {
        Vector3f32 dir = m_emitterPosition - m_listenerPosition;
        float      distance = dir.Norm();
        if (distance < 1e-8f) return;

        Vector3f32 normDir = dir / distance;
        float vEmitter = m_velocity.DotProduct(normDir);
        float doppler = Clamp(343.0f / (343.0f + vEmitter), 0.5f, 2.0f);

        m_pSourceVoice->SetFrequencyRatio(doppler);
    }

    // -----------------------------------------------------------------------
    //  Orbit helpers
    // -----------------------------------------------------------------------

    void SoundSource3D::InitializeOrbitFromCurrentPosition()
    {
        m_orbitCenter = m_listenerPosition;

        Vector3f32 relative = m_emitterPosition - m_orbitCenter;
        m_orbitRadius = relative.Norm();

        if (m_orbitRadius > 0.0001f)
            m_orbitAngle = std::atan2(relative.z, relative.x);
        else
        {
            m_orbitAngle = 0.0f;
            m_orbitRadius = 1.0f;
        }
    }

    void SoundSource3D::CircleAroundListener(float deltaTime)
    {
        if (deltaTime < 0.001f) deltaTime = 0.001f;

        m_orbitSpeed = 5.0f;
        m_orbitAngle += deltaTime * m_orbitSpeed;

        float x = std::cos(m_orbitAngle) * m_orbitRadius;
        float z = std::sin(m_orbitAngle) * m_orbitRadius;

        Vector3f32 newPosition = m_orbitCenter + Vector3f32(x, 0.0f, z);
        SetEmitterPosition(newPosition.x, newPosition.y, newPosition.z);

        float dx = -std::sin(m_orbitAngle) * m_orbitRadius * m_orbitSpeed;
        float dz = std::cos(m_orbitAngle) * m_orbitRadius * m_orbitSpeed;
        m_velocity = Vector3f32(dx, 0.0f, dz);
    }

}