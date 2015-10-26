#include "pathfinding.h"


#include <vector>
#include <game/server/gamecontext.h>

using namespace std;


CWaypointPath *CWaypointPath::GetVisible(CGameContext *pGameServer, vec2 Pos)
{
	if (m_pNext)
	{
		if (!pGameServer->Collision()->FastIntersectLine(m_pNext->m_Pos, Pos))
			return m_pNext->GetVisible(pGameServer, Pos);
	}
	
	if (!pGameServer->Collision()->FastIntersectLine(m_Pos, Pos))
	{
		return this;
	}
	else
	{
		if (m_pParent)
		{
			//if (!pGameServer->Collision()->FastIntersectLine(m_pParent->m_Pos, Pos))
			//	return m_pParent;
		}
		
	}

	return 0;
}





void CWaypoint::SetCenter(int Distance)
{
	// set self's distance
	m_PathDistance = Distance;
	
	// set connections' distance
	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			if (m_apConnection[i]->m_PathDistance == 0)
			{
				m_apConnection[i]->m_PathDistance = Distance + m_aDistance[i];
			}
		}
	}
	
	// visit connections
  	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			if (m_apConnection[i]->m_PathDistance >= Distance + m_aDistance[i])
			{
				m_apConnection[i]->SetCenter(Distance + m_aDistance[i]);
			}
		}
   }
}


CWaypointPath *CWaypoint::FindPathToCenter(CWaypointPath *Path)
{
	// not connected
	if (m_PathDistance == 0)
		return NULL;
	
	CWaypointPath *P = Path;
	
	// first
	if (!P)
		P = new CWaypointPath(m_Pos);
	
	if (m_PathDistance == 1)
	{
		return new CWaypointPath(m_Pos, P);
	}
	
	
	// find closest to center
	CWaypoint *Closest = NULL;
	
  	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			if (m_apConnection[i]->m_PathDistance < m_PathDistance)
			{
				if (!Closest)
					Closest = m_apConnection[i];
				else
				{
					if (Closest->m_PathDistance > m_apConnection[i]->m_PathDistance)
						Closest = m_apConnection[i];
				}
			}
		}
	}
	
	if (Closest)
		return Closest->FindPathToCenter(new CWaypointPath(m_Pos, P));
	
	return NULL;
}




CWaypointPath *CWaypoint::FindPath(CWaypoint *Target, int Distance)
{
	Clear();
	SetCenter();
	CWaypointPath *Path = Target->FindPathToCenter();
	
	if (Path)
		return Path;
	else
		return 0;
	
	
	
	
	
	// old path finding
	/*
	if (Distance == 0)
	{
		Clear();
		Distance = 1;
	}
	
	m_PathDistance = Distance;
	CWaypointPath *Path = 0;
		
	// found the target
	if (this == Target)
	{
		Path = new CWaypointPath(this->m_Pos);
		return Path;
	}

	
	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			m_apConnection[i]->m_Score = abs(Target->m_X - m_apConnection[i]->m_X) + abs(Target->m_Y - m_apConnection[i]->m_Y);
			
			if (m_apConnection[i]->m_PathDistance == 0)
			{
				m_apConnection[i]->m_PathDistance = Distance + m_aDistance[i];
			}
		}
   }
   
	// sort the connections by score
	int i = 1;
	
	while (i < m_ConnectionCount)
	{
		if (m_apConnection[i]->m_Score < m_apConnection[i-1]->m_Score)
		{
			CWaypoint *w = m_apConnection[i];
			m_apConnection[i] = m_apConnection[i-1];
			m_apConnection[i-1] = w;
			if (i > 1)
				i--;
		}
		else
			i++;
	}
	

	
	  
	// check all possible paths
	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			//if (m_apConnection[i]->m_PathDistance > Distance + m_aDistance[i] || m_apConnection[i]->m_PathDistance == 0)
				
			if (m_apConnection[i]->m_PathDistance >= Distance + m_aDistance[i])
			{
				Path = m_apConnection[i]->FindPath(Target, Distance + m_aDistance[i]);
				
				if (Path)
					return new CWaypointPath(this->m_Pos, Path);
				
				/*
				//Path = m_apConnection[i]->FindPath(Target, Distance + m_aDistance[i]+1);
				CWaypointPath *P = m_apConnection[i]->FindPath(Target, Distance + m_aDistance[i]+1);
					
				// store the path for later use
				if (P)
				{
					// delete the existing path first
					//if (Path)
					//	delete Path;
						
					//Path = P;
					return new CWaypointPath(this->m_Pos, Path);
				}
				
			}
		}
	}
		
	// push the existing path to a new one
	if (Path)
	{
		CWaypointPath *P = new CWaypointPath(this->m_Pos, Path);
		return P;
	}
		
	return 0;
	*/
}