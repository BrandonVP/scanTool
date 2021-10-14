// 
// 
// 

#include "Variables.h"

// false if locked
bool isVar8Unlocked(uint8_t pos)
{
	return !((g_var8Lock) & (1 << (pos)));
}

// false if locked
bool isVar16Unlocked(uint8_t pos)
{
	return !((g_var16Lock) & (1 << (pos)));
}

// false if locked
bool isVar32Unlocked(uint8_t pos)
{
	return !((g_var32Lock) & (1 << (pos)));
}

bool lockVar8(uint8_t lock)
{
	uint8_t temp = g_var8Lock;

	if ((temp | lock) > g_var8Lock)
	{
		g_var8Lock += lock;
		return true;
	}
	else
	{
		return false;
	}
}

bool unlockVar8(uint8_t unlock)
{
	uint8_t temp = g_var8Lock;
	if ((temp ^ unlock) < g_var8Lock)
	{
		g_var8Lock -= unlock;
		return true;
	}
	else
	{
		return false;
	}
}

bool lockVar16(uint8_t lock)
{
	uint8_t temp = g_var16Lock;
	if ((temp | lock) > g_var16Lock)
	{
		g_var16Lock += lock;
		return true;
	}
	else
	{
		return false;
	}
}

bool unlockVar16(uint8_t unlock)
{
	uint8_t temp = g_var16Lock;
	if ((temp ^ unlock) < g_var16Lock)
	{
		g_var16Lock -= unlock;
		return true;
	}
	else
	{
		return false;
	}
}

bool lockVar32(uint8_t lock)
{
	uint8_t temp = g_var32Lock;
	if ((temp | lock) > g_var32Lock)
	{
		g_var32Lock += lock;
		return true;
	}
	else
	{
		return false;
	}
}

bool unlockVar32(uint8_t unlock)
{
	uint8_t temp = g_var32Lock;
	if ((temp ^ unlock) < g_var32Lock)
	{
		g_var32Lock -= unlock;
		return true;
	}
	else
	{
		return false;
	}
}