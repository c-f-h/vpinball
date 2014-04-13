#pragma once

HRESULT GetRegStringAsFloat(const char *szKey, const char *szValue, float *pfloat);
HRESULT GetRegString(const char *szKey, const char *szValue, void *szbuffer, DWORD size);
HRESULT GetRegInt(const char *szKey, const char *szValue, int *pint);
HRESULT GetRegValue(const char *szKey, const char *szValue, DWORD *ptype, void *pvalue, DWORD size);

int GetRegIntWithDefault(const char *szKey, const char *szValue, int def);
float GetRegStringAsFloatWithDefault(const char *szKey, const char *szValue, float def);

HRESULT SetRegValue(const char *szKey, const char *szValue, DWORD type, void *pvalue, DWORD size);

HRESULT SetRegValueInt(const char *szKey, const char *szValue, int val);
HRESULT SetRegValueFloat(const char *szKey, const char *szValue, float val);
