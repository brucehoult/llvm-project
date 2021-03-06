#include <clc/clc.h>

_CLC_DEF uint get_local_id(uint dim)
{
	switch(dim) {
	case 0: return __builtin_amdgcn_workitem_id_x();
	case 1: return __builtin_amdgcn_workitem_id_y();
	case 2: return __builtin_amdgcn_workitem_id_z();
	default: return 1;
	}
}
