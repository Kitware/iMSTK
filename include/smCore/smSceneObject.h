/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMSCENEOBJECT_H
#define SMSCENEOBJECT_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smConfigRendering.h"
#include "smRendering/smCustomRenderer.h"
#include "smCore/smMemoryBlock.h"
#include <QAtomicInt>

//forward class declaration
class smObjectSimulator;
class smCustomRenderer;

///booleans for objects indicate whether they're initialized or not.
struct smObjectInitFlags{
	smBool isViewerInit;
	smBool isSimulatorInit;

};
/// \brief  scene object for visualization
class smSceneObject:public smCoreClass{
	/// \brief object simulator that will simulate the object
	smObjectSimulator *objectSim;
	/// \brief custom renderer 
	smCustomRenderer *customRender;

	friend class smSDK;
	friend class smViewer;
	friend class smScene;
	friend class smObjectSimulator;

private:
protected:

public:
	/// \brief memory block for allocations that will be made for the object
	smMemoryBlock *memBlock;
	/// \brief flags
	smObjectInitFlags flags;
	/// \brief constructor
	smSceneObject();
	/// \brief returns object id
	smInt getObjectId(){
		return uniqueId.ID;
	}
	/// \brief get unified object id
	smUnifiedID getObjectUnifiedID(){
		return uniqueId;
	}
	/// \brief attach simulator to the object. This function that needs to be called to associate the simulator to the object
	virtual void attachObjectSimulator(smObjectSimulator *p_objectSim);
	/// \brief to release the simulator
	virtual void releaseObjectSimulator();
	/// \brief get object simulator 
	smObjectSimulator* getObjectSimulator();
	/// \brief attach custome renderer for the object. If the default rendering is not helpful
	void attachCustomRenderer(smCustomRenderer *p_customeRenderer);
	void releaseCustomeRenderer();

	///serialize function explicity writes the object to the memory block
	///each scene object should know how to write itself to a memory block
	virtual void serialize(void *p_memoryBlock)=0;

	///Unserialize function can recover the object from the memory location
	virtual void unSerialize(void *p_memoryBlock)=0;

	///this function may not be used
	///every Scene Object should know how to clone itself. Since the data structures will be 
	///in the beginning of the modules(such as simulator, viewer, collision etc.)
	virtual smSceneObject*clone()=0;

	//comment: make the this routine abstact so that enforce everybody to write their initiazaliton routines
	///the initialization routuines belong to the objects should be called herein
	virtual void init(){
	};
};

#endif
