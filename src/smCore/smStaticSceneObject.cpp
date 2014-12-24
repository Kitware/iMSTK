#include "smCore/smStaticSceneObject.h"

void smStaticSceneObject::draw(smDrawParam p_params)
{

    p_params.caller = this;
    mesh->draw(p_params);
}
