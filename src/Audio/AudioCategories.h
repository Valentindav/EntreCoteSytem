#ifndef AUDIO_AUDIOCATEGORY_H_INCLUDED
#define AUDIO_AUDIOCATEGORY_H_INCLUDED

#include "define.h"
#include <xaudio2.h>

namespace audio
{
    enum class Category
    {
        NONE,
        MUSIC,
        SFX,
        DIALOGUE,
        UI,
        AMBIENT,
        COUNT
    };

    class AudioCategory
    {
    public:

        void Init(IXAudio2* xaudio, Category type = Category::NONE , bool isMono = false);
        void SetVolume(int32 volume);
        void Mute(bool state);
        float GetVolume() const;
        bool  IsMuted() const;

        IXAudio2SubmixVoice* GetSubmixVoice() const;

    private:
        Category m_type;
        IXAudio2SubmixVoice* m_pSubmix = nullptr;
        float m_volume = 1.0f;
        bool  m_muted = false;
    };
}

#endif