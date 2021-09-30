#include "stdafx.h"
#include <stdlib.h>
#include "VxCpp.h"

//  Factory function for IVoxieBox which will return the new object instance.
EXTEND_API IVoxieBox* _cdecl CreateVoxieBoxObject() {
	return new VoxieBox();
}
