/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMSCENE_H
#define SMSCENE_H

#include <QVector>
#include <QMutex>
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smErrorLog.h"
#include "smUtilities/smDataStructs.h"
#include "smCore/smDoubleBuffer.h"

class smPipe;
class smSDK;

struct smSceneLocal
{

public:
    smInt id;
    smSceneLocal()
    {
        sceneUpdatedTimeStamp = 0;
    }

    inline smBool operator ==(smSceneLocal &p_param)
    {
        return id == p_param.id;
    }

    vector<smSceneObject*> sceneObjects;
    smUInt sceneUpdatedTimeStamp;
};

///Physics class should have all parameters such as material properties, mesh etc.. for
///note that when you remove the Physics do not delete it.Since propagation of the physics over the
class smScene: public smCoreClass
{

private:
    /// \brief number of total objects in the scene
    smInt totalObjects;
    /// \brief error logging
    smErrorLog *log;
    /// \brief scene list lock for thread safe manipulation of the scene
    QMutex sceneList;
    /// \brief reference counter to the scene
    smUInt referenceCounter;
    /// \brief last updated time stampe
    smUInt sceneUpdatedTimeStamp;
    /// \brief scene objects addition queue
    vector<smSceneObject*> addQueue;
    smIndiceArray<smSceneLocal*> sceneLocal;
    QHash<smInt, smInt> sceneLocalIndex;
    /// \brief scene objects storage
    vector<smSceneObject*> sceneObjects;
    /// \brief adds the objects in the local scene storage
    void inline copySceneToLocal(smSceneLocal *p_local)
    {
        p_local->sceneObjects.clear();

        for (smInt i = 0; i < sceneObjects.size(); i++)
        {
            p_local->sceneObjects.push_back(sceneObjects[i]);
        }

        p_local->sceneUpdatedTimeStamp = sceneUpdatedTimeStamp;

    }

public:
    /// \brief iterator for scene object. The default iterates over the scene in the order of insertion. For scene graph, this iteration needs to be inherited and modified
    struct smSceneIterator
    {
    protected:
        //smInt startIndex;
        smInt endIndex;
        smInt currentIndex;
        smSceneLocal *sceneLocal;
    public:
        inline smSceneIterator()
        {
            currentIndex = endIndex = 0;
            sceneLocal = NULL;

        }
        /// \brief copy other scene to this current one.
        inline void setScene(smScene *p_scene, smCoreClass *p_core)
        {

            sceneLocal = p_scene->sceneLocal.getByRef(p_scene->sceneLocalIndex[p_core->uniqueId.ID]);

            if (p_scene->sceneUpdatedTimeStamp > sceneLocal->sceneUpdatedTimeStamp)
            {
                p_scene->sceneList.lock();
                p_scene->copySceneToLocal(sceneLocal);
                p_scene->sceneList.unlock();

            }

            endIndex = sceneLocal->sceneObjects.size();
            currentIndex = 0;

        }

        inline smInt start()
        {
            return 0;
        }

        inline smInt end()
        {
            return endIndex;
        }
        /// \brief operator to increment the index to go to the next item in the scene
        inline void operator++()
        {
            currentIndex++;
        }
        /// \brief operator to decrement the index to go to the previous item in the scene
        inline void operator--()
        {
            currentIndex--;
        }
        /// \brief to access a particular entry in the scene with [] notation
        inline smSceneObject* operator[](smInt p_index)
        {
            return sceneLocal->sceneObjects[p_index];
        }
        /// \brief to access a particular entry in the scene with () notation
        inline smSceneObject* operator*()
        {
            return sceneLocal->sceneObjects[currentIndex];
        }
    };

    smScene(smErrorLog *p_log = NULL);
    /// \brief add obejct to the scene, it is thread safe call.
    void registerForScene(smCoreClass *p_simmedtkObject)
    {
        smSceneLocal *local = new smSceneLocal();
        local->id = p_simmedtkObject->uniqueId.ID;
        sceneList.lock();
        copySceneToLocal(local);
        sceneLocalIndex[p_simmedtkObject->uniqueId.ID] = sceneLocal.checkAndAdd(local);
        sceneList.unlock();
    }

    ///add physics in the scene
    void  addSceneObject(smSceneObject *p_sceneObject);

    ///remove the phyics in the scene.
    ///The removal of the phsyics in the scene needs some sync all over the modules
    ///so not implemented yet. Be aware that when you remove the phyics do no free the smPhysics class
    void removeSceneObject(smSceneObject *p_sceneObject);

    ///the same as
    void removeSceneObject(smInt p_objectId);

    ///in order to get the phsyics in the scene call this function.
    ///it is thread safe. but it shouldn't be called frequently.
    ///it should be called in the initialization of the viewer, simulation or any other module.
    ///and the the list should be stored internally.
    ///The scene list removal will be taken care of later since the list should be update.
    vector<smSceneObject*> getSceneObject();
    /// \brief retursn scene id
    smInt getSceneId();
    /// \brief returns the total number of objects in the scene
    inline smInt getTotalObjects();

    ///Same functionality as addSceneObject
    smScene& operator +=(smSceneObject *p_sceneObject);
    /// \brief add and remove references
    void addRef();
    void removeRef();
    void copySceneObjects(smScene*p_scene);
    smScene &operator =(smScene &p_scene) ;

    friend class smSDK;
    friend struct smSceneIterator;

    smInt test;
};

#endif
