
#ifndef SMMYSTYLUS_H
#define SMMYSTYLUS_H

#include "smSimulators/smStylusObject.h"
class smMeshContainer;

/// \brief class for avatar controlled by phantom omni in virtual space
class MyStylus:public smStylusRigidSceneObject{

public:
	smInt phantomID; ///> ID fo phantom omni
	smBool buttonState[2]; ///> state of the buttons
	smFloat angle; ///> angle of the jaw (if applicable)
	smFloat maxangle; ///> maximum angle of the jaw (if applicable)
	smMeshContainer meshContainer,meshContainerLower,meshContainerUpper; ///> !!

	/// \brief handle keyboard and omni button presses 
	void handleEvent(smEvent *p_event);

	/// \brief constructor
	MyStylus(smChar *p_shaft="../../resources/models/blunt_diss_pivot.3DS",
	         smChar *p_lower="../../resources/models/blunt_diss_lower.3DS",
	         smChar *p_upper="../../resources/models/blunt_diss_upper.3DS");

	/// \brief update the closing and opening of jaws (if applicable)
	void updateOpenClose();
};

/// \brief class for cauter tool avatar in virtual space
class HookCautery:public smStylusRigidSceneObject{

public:
	smInt phantomID; ///> ID of the phantom omni
	smBool buttonState[2]; ///> state of the buttons
	smMeshContainer meshContainer; ///> !!

	/// \brief handle keyboard and omni button presses 
	void handleEvent(smEvent *p_event);

	/// \brief render the avatar
	void HookCautery::draw(smDrawParam p_param);

	/// \brief constructor
	HookCautery(smChar *p_pivot="../../resources/models/hook_cautery_new.3DS");
};

#endif
