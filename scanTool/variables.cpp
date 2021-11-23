// 
// 
// 

#include "Variables.h"

/* How to use

* Returns true if able to lock, false if already locked *
Lock first variable: lockVar8(LOCK0);
Lock second variable: lockVar8(LOCK1);
Lock another variable lockVar32(LOCK0);

* Use locked variables *
g_var8[POS0] = 0;
g_var8[POS1] = 2;
g_var32[POS0] = g_var8[POS0] + g_var8[POS1]

* Unlock variable when finished *
Unlock first variable: unlockVar8(LOCK0);
Unlock sewcond variable variable: unlockVar8(LOCK1);
Unlock another variable unlockVar8(LOCK0);

*/

/*Example Application

bool error = false;
(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : error = true;
if (error)
{
	DEBUG_ERROR("Error: Variable locked");
}

*/

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