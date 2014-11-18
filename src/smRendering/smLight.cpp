#include "smRendering/smLight.h"
#include "smUtilities/smQuat.h"

smVec3<smFloat> smLight::defaultDir(0,0,-1.0);
smVec3<smFloat> smLight::defaultUpDir(0,1,0.0);
smVec3<smFloat> smLight::defaultTransDir(1,0,0.0);

void smLight::updateDirection()
{
	smQuat<smFloat> rot;
	smFloat angle;
	smVec3<smFloat> dirNorm;

	dirNorm=direction.unit();
	angle=acos(dirNorm.dot(defaultDir));
	smVec3<smFloat> axisOfRot=dirNorm.cross(defaultDir);
	axisOfRot.normalize();
	rot.fromAxisAngle(axisOfRot,-angle);
	upVector=rot.rotate(defaultUpDir);
	transverseDir=rot.rotate(defaultTransDir);
}
