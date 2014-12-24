/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMWORKERTHREAD_H
#define SMWORKERTHREAD_H

#include "smCore/smCoreClass.h"
#include "smCore/smSynchronization.h"
#include <Qthread>
/// \brief process(used as conceptual meaning) numbering scheme
enum smProcessNumbering
{
    SIMMEDTK_PROCNUMSCHEME_X__,
    SIMMEDTK_PROCNUMSCHEME_XY_,
    SIMMEDTK_PROCNUMSCHEME_XYZ,
};
/// \brief process id
struct smProcessID
{
public:
    /// \brief numbering scheme in x,y,z
    smUShort x;
    smUShort y;
    smUShort z;

    smUShort totalProcX;
    smUShort totalProcY;
    smUShort totalProcZ;
    /// \brief data
    void *data;
    /// \brief data size
    smInt sizeOfData;
    /// \brief numbering sceheme
    smProcessNumbering numbScheme;

    smProcessID()
    {
        x = y = z = totalProcX = totalProcY = totalProcZ = sizeOfData = 0;
        data = NULL;
        numbScheme = SIMMEDTK_PROCNUMSCHEME_X__;
    }

    inline void operator=(smProcessID p_ID)
    {
        x = p_ID.x;
        y = p_ID.y;
        z = p_ID.z;
        totalProcX = p_ID.totalProcX;
        totalProcY = p_ID.totalProcY;
        totalProcZ = p_ID.totalProcZ;

        data = new smChar[p_ID.sizeOfData];
        memcpy(data, p_ID.data, sizeOfData);
    }

};



/// \brief process. Process is a atomic execution unit(thread).
class smProcess: public smCoreClass
{

protected:
    smProcessID id;
    smBool termination;

public:
    smProcess()
    {
        id.x = 0;
        id.y = 0;
        id.z = 0;
        id.totalProcX = 0;
        id.totalProcY = 0;
        id.totalProcZ = 0;
        id.data = NULL;
        id.sizeOfData = 0;
        id.numbScheme = SIMMEDTK_PROCNUMSCHEME_X__;
        termination = false;
    }

    smProcess(smProcessID p_id)
    {
        id = p_id;
        termination = false;
    }

    void setId(smProcessID p_id)
    {
        id = p_id;
    }

    virtual void kernel() = 0;

    void terminate()
    {
        termination = true;
    }
};
/// \brief worker thread extends process
class smWorkerThread: public QThread, public smProcess
{

protected:
    /// \brief for synchronization
    smSynchronization *synch;

public:
    smWorkerThread()
    {
    }

    smWorkerThread(smProcessID p_ID): smProcess(p_ID)
    {
        termination = false;
    }

    smWorkerThread(smSynchronization &p_synch, smProcessID p_ID): smProcess(p_ID)
    {
        synch = &p_synch;
        termination = false;
    }

    void setSynchObject(smSynchronization &p_synch)
    {
        synch = &p_synch;
    }

    virtual void run() = 0;
};

#endif
