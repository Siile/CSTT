/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_BOMB_H
#define GAME_SERVER_ENTITIES_BOMB_H

#include <game/server/entity.h>

enum BombStatus
{
	BOMB_IDLE,
	BOMB_CARRYING,
	BOMB_PLANTING,
	BOMB_PLANTED,
};

class CBomb : public CEntity
{
public:
	static const int ms_PhysSize = 14;
	CCharacter *m_pCarryingCharacter;
	vec2 m_Vel;

	int m_DropTick;
	int m_GrabTick;
	
	bool m_Hide;
	
	int m_Status;
	int m_Timer;
	int m_Owner;
	
	CBomb(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
};

#endif
