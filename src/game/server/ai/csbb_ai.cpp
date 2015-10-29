#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/entities/bomb.h>
#include <game/server/entities/flag.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "csbb_ai.h"


CAIcsbb::CAIcsbb(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
}


void CAIcsbb::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	
	int Round = GameServer()->m_pController->GetRound();

	
	if (g_Config.m_SvRandomWeapons)
	{
		pChr->GiveRandomWeapon();
	}
	else
	{
		int Weapon = GUN_PISTOL;
		
		if (frandom()*10 < (Round+1)*3)
		{
			if (frandom()*12 < 2)
				Weapon = SWORD_KATANA;
			else if (frandom()*12 < 2)
				Weapon = GUN_MAGNUM;
			else if (frandom()*12 < 3)
				Weapon = RIFLE_ASSAULTRIFLE;
			else if (frandom()*12 < 3)
				Weapon = GRENADE_GRENADELAUNCHER;
			else if (frandom()*12 < 3)
				Weapon = SHOTGUN_DOUBLEBARREL;
			else if (frandom()*12 < 3)
				Weapon = RIFLE_LIGHTNINGRIFLE;
			else if (frandom()*12 < 3)
				Weapon = RIFLE_LASERRIFLE;
			else if (Round > 3)
			{
				if (frandom()*12 < 3)
					Weapon = SHOTGUN_COMBAT;
				else if (frandom()*12 < 3)
					Weapon = RIFLE_STORMRIFLE;
				else if (frandom()*12 < 3)
					Weapon = RIFLE_DOOMRAY;
				else if (frandom()*12 < 3)
					Weapon = GRENADE_DOOMLAUNCHER;
				else if (frandom()*12 < 3)
					Weapon = RIFLE_HEAVYRIFLE;
			}
		}

		pChr->GiveCustomWeapon(Weapon);
		pChr->SetCustomWeapon(Weapon);
	}
	
	//pChr->SetHealth(100);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "bluestripe", 64);

}


void CAIcsbb::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	// using this later
	CBomb *Bomb = GameServer()->m_pController->GetBomb();


	HeadToMovingDirection();
	SeekClosestEnemyInSight();
	
	// if we see a player
	if (m_PlayerSpotCount > 0)
	{
		// jump at random times
		if (Player()->GetCharacter()->IsGrounded() && frandom()*20 < 3)
			m_Jump = 1;

		ShootAtClosestEnemy();
		ReactToPlayer();
	}
	else
		m_AttackTimer = 0;

	
	
	// main logic
	if (Player()->GetTeam() == GameServer()->m_pController->GetDefendingTeam())
	{
		// seek bomb area
		if (SeekBombArea())
		{
			// ...unless we're near it && it's in sight
			if (!m_WayFound || (distance(m_Pos, m_TargetPos) < 400 && !GameServer()->Collision()->FastIntersectLine(m_Pos, m_TargetPos)))
			{
				if (SeekClosestEnemy())
					m_TargetPos = m_PlayerPos;
			}
		}
		else
		{
			if (SeekClosestEnemy())
				m_TargetPos = m_PlayerPos;
		}
	}
	else
	{
		if (SeekBombArea())
		{
			// ...unless we're near it or didn't find it
			if (!m_WayFound || (distance(m_Pos, m_TargetPos) < g_Config.m_SvBaseCaptureDistance/2))
			{
				if (SeekClosestEnemy())
				{
					m_TargetPos = m_PlayerPos;
				}
			}
		}
		else
		{
			if (SeekClosestEnemy())
				m_TargetPos = m_PlayerPos;
		}
	}
		

	/*
	UpdateWaypoint();
	MoveTowardsWaypoint(10);
	*/
	
	if (UpdateWaypoint(Bomb->m_pCarryingCharacter == Player()->GetCharacter() ? 5000 : 0))
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
	
	
	
	// go plant the bomb
	if (Player()->GetTeam() != GameServer()->m_pController->GetDefendingTeam())
	{
		if (Bomb && (Bomb->m_Status == BOMB_CARRYING || Bomb->m_Status == BOMB_PLANTING))
		{
			CFlag *BombArea = GameServer()->m_pController->GetClosestBombArea(m_Pos);
	
			if (BombArea && abs(BombArea->m_Pos.x-m_Pos.x) < 100 && abs(BombArea->m_Pos.y-m_Pos.y-50) < 100)
			{
				m_Move = 0;
				m_Jump = 0;
				m_Hook = 0;
			}
		}
	}	
	
	// don't move if defusing the bomb
	if (Player()->GetTeam() == GameServer()->m_pController->GetDefendingTeam())
	{
		if (Bomb && Bomb->m_Status == BOMB_PLANTED)
		{
			if (abs(Bomb->m_Pos.x-m_Pos.x) < 100 && abs(Bomb->m_Pos.y-m_Pos.y-50) < 100)
			{
				m_Move = 0;
				m_Jump = 0;
				m_Hook = 0;
			}
		}
	}
	
	// don't move if defusing the bomb
	if (GameServer()->m_pController->GetRoundStatus() == 0) // CSBB_NEWBASE
	{
		if (abs(m_TargetPos.x-m_Pos.x) < 200 && abs(m_TargetPos.y-m_Pos.y-50) < 100 && frandom()*10 < 8)
		{
			m_Move = 0;
			m_Jump = 0;
			m_Hook = 0;
		}
	}
	
	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}