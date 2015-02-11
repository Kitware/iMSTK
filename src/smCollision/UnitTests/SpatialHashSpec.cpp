#include <bandit/bandit.h>

#include "smCollision/smSpatialHash.h"
#include "smCore/smErrorLog.h"
#include "smCore/smDoubleBuffer.h"

using namespace bandit;

go_bandit([](){
    describe("the spatial hash collision detector", []() {
        it("initializes properly ", []() {

            std::unique_ptr<smErrorLog> errorLog(new smErrorLog);
			std::unique_ptr<smSpatialHash> spatialHash(new smSpatialHash(errorLog.get(),10,1.0,1.0,1.0,1));

            AssertThat( spatialHash->pipe->getElements(), Equals( 1 ) );
            AssertThat( spatialHash->pipeTriangles->getElements(), Equals( 1 ) );
            AssertThat( spatialHash->pipeModelPoints->getElements(), Equals( 1 ) );
            AssertThat( spatialHash->enableDuplicateFilter, IsFalse() );
        });
    });

});

