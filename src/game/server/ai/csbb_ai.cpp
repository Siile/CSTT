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
	
	int Weapon = HAMMER_BASIC;
	
	m_WaypointDir = vec2(0, 0);
	
	int Round = GameServer()->m_pController->GetRound();
	
	Weapon = GUN_PISTOL;
	
	if (frandom()*8 < Round*3)
	{
		if (frandom()*10 < 2)
			Weapon = SWORD_KATANA;
		else if (frandom()*10 < 2)
			Weapon = GUN_MAGNUM;
		else if (frandom()*10 < 3)
			Weapon = RIFLE_ASSAULTRIFLE;
		else if (frandom()*10 < 3)
			Weapon = RIFLE_LASERRIFLE;
		else if (frandom()*10 < 3)
			Weapon = SHOTGUN_DOUBLEBARREL;
		else if (frandom()*10 < 3)
			Weapon = RIFLE_LIGHTNINGRIFLE;
		else if (Round > 3)
		{
			if (frandom()*10 < 3)
				Weapon = SHOTGUN_COMBAT;
			else if (frandom()*10 < 3)
				Weapon = RIFLE_STORMRIFLE;
			else if (frandom()*10 < 3)
				Weapon = RIFLE_DOOMRAY;
		}
	}

	pChr->GiveCustomWeapon(Weapon);
	pChr->SetCustomWeapon(Weapon);
	
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
		// angry face
		if (m_PlayerSpotCount == 1)
			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);
		
		if (frandom()*30 < 3)
			m_Jump = 1;
		
		if (m_AttackTimer++ > 4)
			ShootAtClosestEnemy();
	}
	else
		m_AttackTimer = 0;

	
	
	// main logic
	{
		// seek bomb area
		SeekBombArea();		

		// ...unless we're near it or didn't find it
		if (!m_WayFound || (distance(m_Pos, m_TargetPos) < g_Config.m_SvBaseCaptureDistance*2 && Player()->GetTeam() == GameServer()->m_pController->GetDefendingTeam()))
		{
			if (SeekClosestEnemy())
			{
				m_TargetPos = m_PlayerPos;
						
				if (m_PlayerSpotCount > 0)
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
		

	UpdateWaypoint();
	MoveTowardsWaypoint(30);
		
	
	// jump if waypoint is above us
	if (abs(m_WaypointPos.x - m_Pos.x) < 100 && m_WaypointPos.y < m_Pos.y - 100 && frandom()*20 < 4)
		m_Jump = 1;
	
	HookMove();
	AirJump();
	Unstuck();
	
	
	if (Player()->GetCharacter()->IsGrounded() && frandom()*20 < 3)
		m_Jump = 1;
	
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
	
	
	// stop attacking for no reason at random times 
	if (frandom()*10 < 4)
		m_Attack = 0;
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}