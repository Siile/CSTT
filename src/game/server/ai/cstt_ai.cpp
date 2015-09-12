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
}


void CAIcstt::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	int Weapon = HAMMER_BASIC;
	
	m_WaypointDir = vec2(0, 0);
	
	int Round = GameServer()->m_pController->GetRound();
	
	Weapon = GUN_PISTOL;
	
	if (frandom()*8 < (Round-1)*3)
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


void CAIcstt::DoBehavior()
{
	// reset jump and attack
	m_Jump = 0;
	m_Attack = 0;
	
	if (GameServer()->m_pController->GetRoundStatus() < 2) // GAMESTATE_ROUND
		return;
	
	// using this later
	CBomb *Bomb = GameServer()->m_pController->GetBomb();
	
	int LockMove = 0;
	
	// release hook if hooking an ally
	if (Player()->GetCharacter()->HookedPlayer() >= 0 && m_Hook == 1)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[Player()->GetCharacter()->HookedPlayer()];
		if(pPlayer && pPlayer->GetTeam() == Player()->GetTeam())
			m_Hook = 0;
	}
	
	if (!Player()->GetCharacter()->Hooking())
	{
		if (m_Hook == 1)
			m_Hook = 0;
	}
	else
		LockMove = m_Move;
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();
	
	// if we see a player
	if (m_PlayerSpotCount > 0)
	{
		// angry face
		if (m_PlayerSpotCount == 1)
			Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 1200, 1200);

		JumpIfPlayerIsAbove();
		
		if (frandom()*20 < 3)
			m_Jump = 1;
		
		if (m_AttackTimer++ > 1)
			ShootAtClosestEnemy();
	}
	else
	{
		m_AttackTimer = 0;
	}

	
	
	// release hook at random
	if (frandom()*10 < 4)
		m_Hook = 0;
	
	
	// main logic
	if (Player()->GetTeam() == TEAM_RED)
	{
		// seek & protect the bomb
		if (Bomb && Bomb->m_Status != BOMB_CARRYING)
			m_TargetPos = Bomb->m_Pos;
		else
		if (Bomb->m_pCarryingCharacter == Player()->GetCharacter())
			SeekBombArea();
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
			if (distance(m_Pos, m_TargetPos) < 1800)
			{
				if (SeekClosestEnemy())
				{
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
		}
	}
		
		
	// update waypoint
	if (distance(m_TargetPos, m_Pos) < 200 || m_TargetTimer++ > 7)// || frandom()*10 < 2)
	{
		m_TargetTimer = 0;
		
		if (GameServer()->Collision()->FindPath(m_Pos, m_TargetPos, m_Move))
		{
			if (GameServer()->Collision()->m_GotVision)
			{
				// we got waypoint to the target
				m_WaypointPos = GameServer()->Collision()->m_VisionPos;
				m_WaypointDir = m_WaypointPos - m_Pos;
			}
		}
		
		if (GameServer()->m_ShowWaypoints)
			GameServer()->CreatePlayerSpawn(m_WaypointPos);
	}
				

	MoveTowardsWaypoint(40);
	
	
	// jump if waypoint is above us
	if (abs(m_WaypointPos.x - m_Pos.x) < 100 && m_WaypointPos.y < m_Pos.y - 100 && frandom()*10 < 4)
	{
		m_Jump = 1;
	}
	
		
	// hook moving
	if (m_LastHook == 0)
	{
		vec2 HookDir = m_WaypointPos - m_Pos;
		float Angle = atan2(HookDir.x, HookDir.y);
			
		float MaxDist = 0;
		vec2 FinalHookPos = vec2(0, 0);
		
		for (int i = -3; i < 4; i++)
		{
			float a = Angle + i*0.025f;
			
			if (a < 60*RAD || a > 300*RAD)
				continue;
				
			vec2 HookPos = m_Pos + vec2(sin(a)*380, cos(a)*380);

			// hook if something in sight
			int C = GameServer()->Collision()->IntersectLine(m_Pos, HookPos, &HookPos, NULL);
			if (C&CCollision::COLFLAG_SOLID && !(C&CCollision::COLFLAG_NOHOOK) && m_LastHook == 0)
			{
				float Dist = distance(m_Pos, HookPos);
				if (Dist > 70 && Dist > MaxDist)
				{
					MaxDist = Dist;
					FinalHookPos = HookPos;
				}
			}
		}
			
		if (MaxDist > 0)
		{
			m_Hook = 1;
			m_Direction = FinalHookPos - m_Pos;
				
			if (Player()->GetCharacter()->IsGrounded())
				m_Jump = 1;
		}
		else
			m_Hook = 0;
	}
		
	// air jump
	if (Player()->GetCharacter()->GetVel().y > 0 && m_TargetPos.y + 80 < m_Pos.y )
	{
		if (!GameServer()->Collision()->FastIntersectLine(m_Pos, m_Pos+vec2(0, 100)) && frandom()*10 < 5)
			m_Jump = 1;
	}

	Unstuck();
	
	if (Player()->GetCharacter()->IsGrounded() && frandom()*10 < 3)
		m_Jump = 1;
	
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
	
	if (frandom()*10 < 3)
		m_Attack = 0;
		
	//if (LockMove != 0)
	//	m_Move = LockMove;
	
	// next reaction in
	m_ReactionTime = 4 + frandom()*12;
	
}