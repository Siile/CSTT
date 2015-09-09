/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <math.h>
#include <engine/map.h>
#include <engine/kernel.h>

#include <game/mapitems.h>
#include <game/layers.h>
#include <game/collision.h>

CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;
}

void CCollision::Init(class CLayers *pLayers)
{
	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));
	
	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index = m_pTiles[i].m_Index;

		if(Index > 128)
			continue;

		switch(Index)
		{
		case TILE_DEATH:
			m_pTiles[i].m_Index = COLFLAG_DEATH;
			break;
		case TILE_SOLID:
			m_pTiles[i].m_Index = COLFLAG_SOLID;
			break;
		case TILE_NOHOOK:
			m_pTiles[i].m_Index = COLFLAG_SOLID|COLFLAG_NOHOOK;
			break;
		default:
			m_pTiles[i].m_Index = 0;
		}
	}
	
	// for path finding
	m_pChecked = new bool [m_Width*m_Height+1];
}



bool CCollision::FindPath(vec2 Start, vec2 End, int Direction)
{
	ClearPath();
	
	m_StartX = clamp(int(Start.x)/32, 0, m_Width-1);
	m_StartY = clamp(int(Start.y)/32, 0, m_Height-1);
	
	m_TargetX = clamp(int(End.x)/32, 0, m_Width-1);
	m_TargetY = clamp(int(End.y)/32, 0, m_Height-1);
	
	return CheckPath(m_StartX, m_StartY, Direction);
}


enum Directions
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};


bool CCollision::CheckPath(int x, int y, int Direction, int Distance)
{
	if (x < 1 || y < 1 || x > m_Width-2 || y > m_Height-2)
		return false;
	
	if (m_pChecked[y*m_Width+x])
		return false;
		
	if (m_pTiles[y*m_Width+x].m_Index)
		return false;
		
	m_pChecked[y*m_Width+x] = true;
	
	if (abs(x - m_TargetX) < 2 && abs(y - m_TargetY) < 2)
		return true;
	
	if (Distance > 10)
		Direction = 0;
	
	if (abs(m_TargetX - x) < abs(m_TargetY - y))
	{
		// go up or down first
		if (m_TargetY < y)
		{
			m_CheckOrder[0] = UP;
			
			if (m_TargetX < x)
			{
				m_CheckOrder[1] = LEFT;
				m_CheckOrder[2] = RIGHT;
			}
			else
			{
				m_CheckOrder[1] = RIGHT;
				m_CheckOrder[2] = LEFT;
			}
			
			m_CheckOrder[3] = DOWN;
		}
		else
		{
			m_CheckOrder[0] = DOWN;
			
			if ((m_TargetX < x && Direction != 1) || Direction == -1)
			{
				m_CheckOrder[1] = LEFT;
				m_CheckOrder[2] = RIGHT;
			}
			else
			{
				m_CheckOrder[1] = RIGHT;
				m_CheckOrder[2] = LEFT;
			}
			
			m_CheckOrder[3] = UP;
		}
	}
	else
	{
		// go left or right first
		if ((m_TargetX < x && Direction != 1) || Direction == -1)
		{
			m_CheckOrder[0] = LEFT;
			
			if (m_TargetY < y)
			{
				m_CheckOrder[1] = UP;
				m_CheckOrder[2] = DOWN;
			}
			else
			{
				m_CheckOrder[1] = DOWN;
				m_CheckOrder[2] = UP;
			}
			
			m_CheckOrder[3] = RIGHT;
		}
		else
		{
			m_CheckOrder[0] = RIGHT;
			
			if (m_TargetY < y)
			{
				m_CheckOrder[1] = UP;
				m_CheckOrder[2] = DOWN;
			}
			else
			{
				m_CheckOrder[1] = DOWN;
				m_CheckOrder[2] = UP;
			}
			
			m_CheckOrder[3] = LEFT;
		}
	}
	
	int AddX = 0;
	int AddY = 0;
	
	for (int i = 0; i < 4; i++)
	{
		if (m_CheckOrder[i] == UP){ AddX = 0; AddY = -1; }
		if (m_CheckOrder[i] == DOWN){ AddX = 0; AddY = 1; }
		if (m_CheckOrder[i] == LEFT){ AddX = -1; AddY = 0; }
		if (m_CheckOrder[i] == RIGHT){ AddX = 1; AddY = 0; }
		
		if (CheckPath(x+AddX, y+AddY, Distance+1))
		{
			if (!m_GotVision && !FastIntersectLine(vec2(m_StartX*32+16, m_StartY*32+16), vec2(x*32+16, y*32+16)))
			{
				m_GotVision = true;
				m_VisionPos = vec2(x*32+16, y*32+16);
			}
			return true;
		}
	}

	return false;
}


void CCollision::ClearPath()
{
	m_GotVision = false;
	
	for(int i = 0; i < m_Width*m_Height; i++)
		m_pChecked[i] = false;
}


int CCollision::GetTile(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

	return m_pTiles[Ny*m_Width+Nx].m_Index > 128 ? 0 : m_pTiles[Ny*m_Width+Nx].m_Index;
}

bool CCollision::IsTileSolid(int x, int y)
{
	return GetTile(x, y)&COLFLAG_SOLID;
}



int CCollision::FastIntersectLine(vec2 Pos0, vec2 Pos1)
{
	float Distance = distance(Pos0, Pos1) / 8.0f;
	int End(Distance+1);

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
			return GetCollisionAt(Pos.x, Pos.y);
	}
	return 0;
}


// TODO: rewrite this smarter!
int CCollision::IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision)
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);
	vec2 Last = Pos0;

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(Pos.x, Pos.y);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

// TODO: OPT: rewrite this smarter!
void CCollision::MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces)
{
	if(pBounces)
		*pBounces = 0;

	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	if(CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
		}

		if(Affected == 0)
		{
			pInoutVel->x *= -Elasticity;
			pInoutVel->y *= -Elasticity;
		}
	}
	else
	{
		*pInoutPos = Pos + Vel;
	}
}

bool CCollision::TestBox(vec2 Pos, vec2 Size)
{
	Size *= 0.5f;
	if(CheckPoint(Pos.x-Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x-Size.x, Pos.y+Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y+Size.y))
		return true;
	return false;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity)
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;

	float Distance = length(Vel);
	int Max = (int)Distance;

	if(Distance > 0.00001f)
	{
		//vec2 old_pos = pos;
		float Fraction = 1.0f/(float)(Max+1);
		for(int i = 0; i <= Max; i++)
		{
			//float amount = i/(float)max;
			//if(max == 0)
				//amount = 0;

			vec2 NewPos = Pos + Vel*Fraction; // TODO: this row is not nice

			if(TestBox(vec2(NewPos.x, NewPos.y), Size))
			{
				int Hits = 0;

				if(TestBox(vec2(Pos.x, NewPos.y), Size))
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					Hits++;
				}

				if(TestBox(vec2(NewPos.x, Pos.y), Size))
				{
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
					Hits++;
				}

				// neither of the tests got a collision.
				// this is a real _corner case_!
				if(Hits == 0)
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
				}
			}

			Pos = NewPos;
		}
	}

	*pInoutPos = Pos;
	*pInoutVel = Vel;
}
