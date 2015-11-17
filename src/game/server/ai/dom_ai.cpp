#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "dom_ai.h"


CAIdom::CAIdom(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
	m_TargetBase = NULL;
}


void CAIdom::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	
	if (g_Config.m_SvRandomWeapons)
		pChr->GiveRandomWeapon();
}


void CAIdom::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;

	
	
	
	HeadToMovingDirection();

	SeekClosestEnemyInSight();

	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ShootAtClosestEnemy();
		ReactToPlayer();
	}
	else
		m_AttackTimer = 0;

	int f = 400+m_EnemiesInSight*100;

	// if we got a target base
	if (m_TargetBase)
	{
		m_TargetPos = m_TargetBase->m_Pos;
		
		if (m_TargetBase->m_CaptureTeam == Player()->GetTeam() && GameServer()->m_pController->Defenders(m_TargetBase) > 1 && frandom()*50 < 2)
		{
			// seek undefended base
			CFlag *Base = GameServer()->m_pController->GetUndefendedBase(Player()->GetTeam());

			if (Base)
				m_TargetBase = Base;
			else
				m_TargetBase = GameServer()->m_pController->GetRandomBase();
		}
	}
	else
	{
		m_TargetBase = GameServer()->m_pController->GetRandomBase();
		
		/*
		if (SeekClosestFriend())
		{
			m_TargetPos = m_PlayerPos;
			
			if (m_PlayerDistance < f)
			{
				if (SeekRandomEnemy())
				{
					m_TargetPos = m_PlayerPos;
							
					if (m_EnemiesInSight > 1)
					{
						// distance to the player
						if (m_PlayerPos.x < m_Pos.x)
							m_TargetPos.x = m_PlayerPos.x + WeaponShootRange()/2*(0.75f+frandom()*0.5f);
						else
							m_TargetPos.x = m_PlayerPos.x - WeaponShootRange()/2*(0.75f+frandom()*0.5f);
					}
				}
			}
		}
		*/
	}
	
	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint(10);
		HookMove();
		AirJump();
		
		// jump if waypoint is above us
		if (abs(m_WaypointPos.x - m_Pos.x) < 60 && m_WaypointPos.y < m_Pos.y - 100 && frandom()*20 < 4)
			m_Jump = 1;
	}
	else
	{
		m_Hook = 0;
	}
	
	
	DoJumping();
	Unstuck();

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}