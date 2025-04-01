#include "FGResourceNodeAlkali.h"
#include "FGResourceDescriptorAlkali.h"

AFGResourceNodeAlkali::AFGResourceNodeAlkali() : Super()
{
    // Disable portable miners and vanilla extractors
    mCanPlacePortableMiner = false;
}
