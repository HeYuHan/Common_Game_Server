#include "tools.h"
#include <time.h>
#include <stdlib.h>
static bool m_RandomFirst = true;
int RandomRange(int a, int b)
{
	if (m_RandomFirst) {
		m_RandomFirst = false;
		srand((unsigned int)time(NULL));
	}
	return (int)((double)rand() / ((RAND_MAX + 1.0) / (b - a + 1.0)) + a);
}
