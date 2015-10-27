#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/entities/bomb.h>
#include <game/server/entities/flag.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "cstt_ai.h"


CAIcstt::CAIcstt(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	m_TargetBombArea = NULL;
}


void CAIcstt::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	int Weapon = HAMMER_BASIC;
	
	m_WaypointDir = vec2(0, 0);
	m_TargetBombArea = NULL;
	
	int Round = GameServer()->m_pController->GetRound();
	
	Weapon = GUN_PISTOL;
	
	if (frandom()*8 < (Round-1)*3)
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
	
	//pChr->SetHealth(100);
	
	str_copy(pChr->GetPlayer()->m_TeeInfos.m_SkinName, "bluestripe", 64);

}


void CAIcstt::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	if (GameServer()->m_pController->GetRoundStatus() < 2) // GAMESTATE_ROUND
		return;
	
	// using this later
	CBomb *Bomb = GameServer()->m_pController->GetBomb();
	
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();
	
	// if we see a player
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
	if (Player()->GetTeam() == TEAM_RED)
	{
		// seek & protect the bomb
		if (Bomb && Bomb->m_Status != BOMB_CARRYING)
		{
			// seek bomb
			m_TargetPos = Bomb->m_Pos;
			
			if (Bomb->m_Status == BOMB_PLANTED)
			{
				// ...unless we're near it && it's in sight
				if (distance(m_Pos, m_TargetPos) < 400 && !GameServer()->Collision()->FastIntersectLine(m_Pos, m_TargetPos))
				{
					if (SeekClosestEnemy())
						m_TargetPos = m_PlayerPos;
				}
			}
		}
		else
		if (Bomb->m_pCarryingCharacter == Player()->GetCharacter())
		{
			//SeekBombArea();
			if (m_TargetBombArea)
				m_TargetPos = m_TargetBombArea->m_Pos;
			else
				m_TargetBombArea = GameServer()->m_pController->GetRandomBombArea();
		}
		else
		{
			if (Bomb && Bomb->m_Status == BOMB_CARRYING && distance(m_Pos, Bomb->m_Pos) > 400)
				m_TargetPos = Bomb->m_Pos;
			else
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
	else
	// blue team
	{
		// horry to bomb
		if (Bomb && Bomb->m_Status == BOMB_PLANTED)
			m_TargetPos = Bomb->m_Pos;
		else
		{
			// seek bomb area
			SeekBombArea();
				
			// ...unless we're near it
			if (distance(m_Pos, m_TargetPos) < 1500)
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
			
			/*
			if (!m_WayFound)
			{
				m_TargetPos = Bomb->m_Pos;
				m_WaypointPos = m_TargetPos;
			}
			*/
		}
	}
		
	if (Bomb->m_pCarryingCharacter == Player()->GetCharacter())
		UpdateWaypoint(50000);
	else
		UpdateWaypoint();
	MoveTowardsWaypoint(10);
	
	//if (!m_WayFound)
	//	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "path", "No path found!");
		
	// jump if waypoint is above us
	if (abs(m_WaypointPos.x - m_Pos.x) < 60 && m_WaypointPos.y < m_Pos.y - 100 && frandom()*20 < 4)
		m_Jump = 1;
		
	HookMove();
	DoJumping();
	AirJump();
	Unstuck();
	

	
	// go plant the bomb
	if (Player()->GetTeam() == TEAM_RED)
	{
		if (Bomb && (Bomb->m_Status == BOMB_CARRYING || Bomb->m_Status == BOMB_PLANTING) && Player()->GetCharacter() == Bomb->m_pCarryingCharacter)
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
	if (Player()->GetTeam() == TEAM_BLUE)
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
	
	/*
	if (m_Hook != 0 && Player()->GetCharacter()->Hooking())
	{
		vec2 CorePos = Player()->GetCharacter()->GetCore().m_Pos;
		vec2 HookPos = Player()->GetCharacter()->GetCore().m_HookPos;
		
		if (distance(CorePos, HookPos) > 100)
		{
			if (HookPos > CorePos)
				m_Move = -1;
			if (HookPos > CorePos)
				m_Move = 1;
		}
	}
	*/
	
	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}