#ifndef SMAUDIO_H
#define SMAUDIO_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "audiere.h"
#include "smCore/smSDK.h"

using namespace audiere;

/// \brief contains state of the audio
enum smAudioState
{
    SIMMEDTK_AUDIOSTATE_PLAYCONTINUOUS,
    SIMMEDTK_AUDIOSTATE_PLAY,
    SIMMEDTK_AUDIOSTATE_STOP
};

/// \brief contains data for audio rendering
struct smAudioEventData
{
    string sound;
    smAudioState state;
    smFloat volume;

    smAudioEventData()
    {
        volume = -1;
    }

};

/// \brief contains audio query states
enum smAudioReturnType
{
    SIMMEDTK_AUDIO_PLAYING,
    SIMMEDTK_AUDIO_STOPPED,
    SIMMEDTK_AUDIO_SOUNDERROR,
    SIMMEDTK_AUDIO_SOUNDNOOP,
};

/// \brief class to enable audio rendering
class smAudio: public smCoreClass, public smEventHandler
{

    AudioDevicePtr device; ///< audio device
    OutputStreamPtr sound; ///< !!

    smErrorLog *log; ///< log for errors rendering audio
    string referenceName; ///< !!
    smAudioState state; ///< state of audio
    smAudioState prevState; ///< state of audio in previous cycle
    smFloat prevVolume; ///< state of audio volume in previous cycle

    smFloat volume; ///< volume (max volume is 1.0)

public:
    smBool continuousPlaying; ///< true if audio is to be played continuously

    /// \brief constructor initialize various states
    smAudio(char *fileName, string p_referenceName, smErrorLog *p_log = NULL)
    {
        device = AudioDevicePtr(OpenDevice());
        sound = OutputStreamPtr(OpenSound(device, fileName, false));
        volume = 1.0f; //max volume
        prevVolume = 1.0f;
        log = p_log;
        continuousPlaying = true;
        referenceName = p_referenceName;
        prevState = state = SIMMEDTK_AUDIOSTATE_STOP;
        smSDK::getInstance()->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_AUDIO);
    }

    /// \brief destructor
    ~smAudio()
    {
        ///clean up process
        sound = 0;
        device = 0;
    }

    /// \brief play the audio
    smAudioReturnType play()
    {

        if (sound == NULL)
        {
            if (log != NULL)
            {
                log->addError("Error in playing sound.Sound object is NULL");
            }

            return SIMMEDTK_AUDIO_SOUNDERROR;
        }

        switch (state)
        {
        case SIMMEDTK_AUDIOSTATE_PLAY:
            if (sound->isPlaying())
            {
                if (state ==  SIMMEDTK_AUDIOSTATE_PLAY || state ==  SIMMEDTK_AUDIOSTATE_PLAYCONTINUOUS)
                {
                    return SIMMEDTK_AUDIO_PLAYING;
                }

            }
            else
            {
                sound->setVolume(volume);
                sound->play();
                return SIMMEDTK_AUDIO_PLAYING;
            }

            break;

        case SIMMEDTK_AUDIOSTATE_STOP:
            if (state == SIMMEDTK_AUDIOSTATE_STOP && prevState == SIMMEDTK_AUDIOSTATE_STOP)
            {
                return SIMMEDTK_AUDIO_STOPPED;
            }

            if (state == SIMMEDTK_AUDIOSTATE_STOP && prevState != SIMMEDTK_AUDIOSTATE_STOP)
            {
                sound->stop();
                return   SIMMEDTK_AUDIO_STOPPED;
            }


            break;

        case SIMMEDTK_AUDIOSTATE_PLAYCONTINUOUS:
            sound->setRepeat(continuousPlaying);
            return SIMMEDTK_AUDIO_PLAYING;
            break;
        }

        return SIMMEDTK_AUDIO_SOUNDNOOP;

    }

    /// \brief stop the audio
    void stop()
    {
        if (sound != NULL)
        {
            if (sound->isPlaying())
            {
                sound->stop();
            }
        }
    }

    /// \brief set the state of audio and continue playing
    void setState(smAudioState p_state)
    {
        state = p_state;
        play();
        prevState = state;
    }

    /// \brief set volume of audio
    void setVolume(smFloat p_volume)
    {
        if (p_volume > 0.0 && p_volume < 1.0)
        {
            volume = p_volume;
        }
    }

    /// \brief gather input events and change states and volume if needed
    void handleEvent(smEvent *p_event)
    {
        smAudioEventData *audioEvent;

        if (p_event->eventType == SIMMEDTK_EVENTTYPE_AUDIO)
        {
            audioEvent = (smAudioEventData*)p_event->data;

            if (audioEvent->sound.compare(referenceName) == 0)
            {
                setVolume(audioEvent->volume);
                setState(audioEvent->state);
            }
        }
    }

};

#endif
