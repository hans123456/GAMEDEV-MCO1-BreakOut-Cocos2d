#include "cmath"
#include "algorithm"

// http://www.playdota.com/forums/showthread.php?t=7993
// http://gaming.stackexchange.com/questions/161430/calculating-the-constant-c-in-dota-2-pseudo-random-distribution
class PseudoRandomDistribution
{
public:
	float CfromP(float);
private:
	float PfromC(float);
};