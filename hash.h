#pragma once

unsigned long StringHash(const unsigned char *str);
unsigned long StringHash(const char *str)             { return StringHash((const unsigned char*)str); }


