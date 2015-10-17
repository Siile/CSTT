/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>


#define MAX_WAYPOINTS 10000
#define MAX_WAYPOINTCONNECTIONS 10


class CWaypoint
{
private:
	CWaypoint *m_apConnection[MAX_WAYPOINTCONNECTIONS];
	int m_ConnectionCount;
	
public:
	int m_X, m_Y; // tileset position
	vec2 m_Pos; // world position

	CWaypoint(vec2 Pos)
	{
		m_X = Pos.x;
		m_Y = Pos.y;
		m_Pos = vec2(Pos.x*32+16, Pos.y*32+16);
		
		m_ConnectionCount = 0;
		for (int i = 0; i < MAX_WAYPOINTCONNECTIONS; i++)
			m_apConnection[i] = NULL;
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
		m_apConnection[m_ConnectionCount++] = Waypoint;
		Waypoint->Connect(this);
	}
};



class CCollision
{
	class CTile *m_pTiles;
	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	bool IsTileSolid(int x, int y);
	int GetTile(int x, int y);

	bool *m_pChecked;
	void ClearPath();
	
	int m_TargetX, m_TargetY;
	int m_StartX, m_StartY;
	
	int m_CheckOrder[4];
	
	bool CheckPath(int x, int y, int Direction, int Distance = 0); // -1, 0, 1
	
	int m_WaypointCount;
	int m_ConnectionCount;
	
	void ClearWaypoints();
	void AddWaypoint(vec2 Position);
	CWaypoint *GetWaypointAt(int x, int y);
	void ConnectWaypoints();

	CWaypoint *m_apWaypoint[MAX_WAYPOINTS];
	
public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,
	};
	
	void GenerateWaypoints();
	int WaypointCount() { return m_WaypointCount; }
	int ConnectionCount() { return m_ConnectionCount; }
	
	vec2 m_VisionPos;
	bool m_GotVision;
	

	// for testing
	vec2 m_aPath[99];
	
	bool FindPath(vec2 Start, vec2 End, int Direction = 0); // -1, 0, 1
	
	CCollision();
	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y) { return IsTileSolid(round(x), round(y)); }
	bool CheckPoint(vec2 Pos) { return CheckPoint(Pos.x, Pos.y); }
	int GetCollisionAt(float x, float y) { return GetTile(round(x), round(y)); }
	int GetWidth() { return m_Width; };
	int GetHeight() { return m_Height; };
	int FastIntersectLine(vec2 Pos0, vec2 Pos1);
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces);
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity);
	bool TestBox(vec2 Pos, vec2 Size);
};

#endif
