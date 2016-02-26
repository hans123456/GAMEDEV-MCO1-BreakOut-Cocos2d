#include "PseudoRandomDistribution.h"

float PseudoRandomDistribution::CfromP( float p )
{
	float Cupper = p;
	float Clower = 0.0f;
	float Cmid;
	float p1;
	float p2 = 1.0f;

	while(true)
	{
		Cmid = ( Cupper + Clower ) / 2.0f;
		p1 = PfromC( Cmid );
		if ( abs( p1 - p2 ) <= 0.f ) break;

		if ( p1 > p )
		{
			Cupper = Cmid;
		}
		else
		{
			Clower = Cmid;
		}

		p2 = p1;
	}

	return Cmid;
}

float PseudoRandomDistribution::PfromC( float C )
{
	float pProcOnN = 0.0f;
	float pProcByN = 0.0f;
	float sumNpProcOnN = 0.0f;

	int maxFails = (int) ceil( 1.0f / C );
	for (int N = 1; N <= maxFails; ++N)
	{
		pProcOnN = std::min( 1.0f, N * C ) * (1.0f - pProcByN);
		pProcByN += pProcOnN;
		sumNpProcOnN += N * pProcOnN;
	}

	return ( 1.0f / sumNpProcOnN );
}
