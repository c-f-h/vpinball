#include "StdAfx.h"

BSTR BstrFromVariant(VARIANT *pvar, LCID lcid)
	{
	VARIANT var;
	VariantInit(&var);

	if (V_VT(pvar) == VT_BOOL)
		{
		const OLECHAR *pszBool = (V_BOOL(pvar)) ? L"True" : L"False";
		V_BSTR(&var) = SysAllocString(pszBool);

		return V_BSTR(&var);
		}
	else
		{
		// Ask OLEAUT32 to handle this.
		const HRESULT hr = ::VariantChangeTypeEx(&var, pvar, lcid, 0, VT_BSTR);
#ifdef DEBUG
		if (hr == hrNoError)
			{
			Assert(V_VT(&var) == VT_BSTR);
			}
#endif
		if (hr == hrNoError)
			return V_BSTR(&var);	
		else
			return NULL;
		}
	}
