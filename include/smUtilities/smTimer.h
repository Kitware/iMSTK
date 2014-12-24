/*
****************************************************
                  SIMMEDTK LICENSE
****************************************************
*/

#ifndef SMTIMER_H
#define SMTIMER_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
/// \brief definitions
#define SMTIMER_FRAME_MILLISEC2SECONDS(X) ( (smLongDouble) (smLongDouble)X/1000.0)
#define SMTIMER_FRAME_MICROSEC2SECONDS(X) ( (smLongDouble) (smLongDouble)X /1000000.0)

/// \brief time in milliseconds or seconds
enum smTimerType
{
    SIMMEDTK_TIMER_INMILLISECONDS,
    SIMMEDTK_TIMER_INMICROSECONDS
};

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
#include <windows.h>
#endif
/// \brief timer class
class smTimer: public smCoreClass
{
public:
    /// \brief constructor
    smTimer()
    {
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
        QueryPerformanceFrequency(&m_liPerfFreq);
#endif
        start();
    }
    /// \brief start the timer
    inline  void start()
    {
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
        QueryPerformanceCounter(&m_liPerfStart);
#endif
    }
    /// \brief gets the time when now is called
    inline smLongDouble now(smTimerType p_type)  // Returns # of microseconds since Start was called
    {
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
        smLongInt perSecond;

        if (p_type == SIMMEDTK_TIMER_INMILLISECONDS)
        {
            perSecond = 1000;    //timer for milliseconds
        }
        else
        {
            perSecond = 1000000;    //timer for microseconds
        }

        QueryPerformanceCounter(&m_liPerfNow);
        return (((smLongDouble)(m_liPerfNow.QuadPart - m_liPerfStart.QuadPart) * perSecond) / ((smLongDouble)m_liPerfFreq.QuadPart));
#endif
    }

private:
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
    LARGE_INTEGER m_liPerfFreq;     // Counts per second
    LARGE_INTEGER m_liPerfStart;    // Starting count
    LARGE_INTEGER m_liPerfNow;  // Starting count
#endif

};

#endif
