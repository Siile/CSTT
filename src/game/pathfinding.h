#ifndef GAME_WAYPOINT_H
#define GAME_WAYPOINT_H

#include <base/vmath.h>


#define MAX_WAYPOINTS 10000
#define MAX_WAYPOINTCONNECTIONS 10


class CWaypointPath
{
private:
	int m_DistanceToNext;

public:
	CWaypointPath *m_pNext;
	CWaypointPath *m_pParent;
	vec2 m_Pos;
	
	CWaypointPath(vec2 Pos, CWaypointPath *Next = 0)
	{
		m_Pos = Pos;
		m_pNext = Next;
		m_pParent = 0;		
		
		if (Next)
			Next->m_pParent = this;
		
		if (m_pNext)
			m_DistanceToNext = distance(Pos, m_pNext->m_Pos);
		else
			m_DistanceToNext = 0;
	}
	
	~CWaypointPath()
	{
		if (m_pNext)
			delete m_pNext;
	}
	
	int Length(int i = 0)
	{
		if (m_pNext)
			//return m_pNext->Length(i+m_DistanceToNext);
			return m_pNext->Length(i+1);
		else
			return i;
	}
	
	CWaypointPath *GetVisible(class CGameContext *pGameServer, vec2 Pos);
};



class CWaypoint
{
private:
	int m_aDistance[MAX_WAYPOINTCONNECTIONS];
	
	void SetCenter(int Distance = 1);
	CWaypointPath *FindPathToCenter(CWaypointPath *Path = 0);
	
public:
	int m_X, m_Y; // tileset position
	vec2 m_Pos; // world position

	// to be used to prioritize connection check order
	int m_Score;
	
	CWaypoint *m_apConnection[MAX_WAYPOINTCONNECTIONS];
	int m_ConnectionCount;
	
	// preventing loops and finding the shortest way to target
	int m_PathDistance;
	
	CWaypoint(vec2 Pos)
	{
		m_PathDistance = 0;
		m_X = Pos.x;
		m_Y = Pos.y;
		m_Pos = vec2(Pos.x*32+16, Pos.y*32+16);
		
		m_ConnectionCount = 0;
		for (int i = 0; i < MAX_WAYPOINTCONNECTIONS; i++)
			m_apConnection[i] = 0;
	}
	
	
	bool Connected(CWaypoint *Waypoint)
	{
		if (!Waypoint || Waypoint == this)
			return false;
		
		// check if we're connected already
		for (int i = 0; i < MAX_WAYPOINTCONNECTIONS; i++)
		{
			if (Waypoint == m_apConnection[i])
				return true;
		}
		
		return false;
	}
	
	
	// create a two way connection between this and given waypoint
	void Connect(CWaypoint *Waypoint)
	{
		if (!Waypoint || Waypoint == this || m_ConnectionCount >= MAX_WAYPOINTCONNECTIONS)
			return;
		
		// check if we're connected already
		if (Connected(Waypoint))
			return;
		
		// connect
		m_apConnection[m_ConnectionCount] = Waypoint;
		m_aDistance[m_ConnectionCount] = distance(m_Pos, Waypoint->m_Pos);
		m_ConnectionCount++;
		
		Waypoint->Connect(this);
	}
	
	void Clear()
	{
		m_PathDistance = 0;
		
		// clear all possible paths
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i])
			{
				if (m_apConnection[i]->m_PathDistance != 0)
					m_apConnection[i]->Clear();
			}
		}
	}
	
	
	void ClearConnections()
	{
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i])
			{
				m_apConnection[i] = 0;
			}
		}
		
		m_ConnectionCount = 0;
	}
	
	void Unconnect(CWaypoint *Target)
	{
		if (Target == 0)
			return;
		
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i] == Target)
			{
				m_apConnection[i] = 0;
				Target->Unconnect(this);
			}
		}
	}
	
	
	CWaypointPath *FindPath(CWaypoint *Target, int Distance = 0);
};


#endif
