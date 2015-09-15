#include <engine/shared/config.h>

#include "ai.h"
#include "entities/character.h"
#include "entities/staticlaser.h"
#include "entities/flag.h"
#include "entities/bomb.h"
#include <game/server/player.h>
#include "gamecontext.h"


CAI::CAI(CGameContext *pGameServer, CPlayer *pPlayer)
{
	m_pGameServer = pGameServer;
	m_pPlayer = pPlayer;
	Reset();
}


void CAI::Reset()
{
	m_Sleep = 0;
	m_Stun = 0;
	m_ReactionTime = 20;
	m_NextReaction = m_ReactionTime;
	m_InputChanged = true;
	m_Move = 0;
	m_LastMove = 0;
	m_Jump = 0;
	m_LastJump = 0;
	m_LastAttack = 0;
	m_Hook = 0;
	m_LastHook = 0;
	m_LastPos = vec2(-9000, -9000);
	m_Direction = vec2(-1, 0);
	
	m_UnstuckCount = 0;
	
	m_WayPointUpdateWait = 0;
	m_WayFound = false;
	
	m_TargetTimer = 0;
	m_AttackTimer = 0;
	m_HookTimer = 0;
	m_HookReleaseTimer = 0;
	
	m_PlayerPos = vec2(0, 0);
	m_TargetPos = vec2(0, 0);
	m_PlayerSpotTimer = 0;
	m_PlayerSpotCount = 0;
}


void CAI::OnCharacterSpawn(class CCharacter *pChr)
{
	pChr->SetCustomWeapon(GUN_PISTOL);
	m_WaypointPos = pChr->m_Pos;
}


void CAI::Zzz(int Time)
{
	if (!Player()->GetCharacter())
		return;
		
	if (m_Sleep < Time)
	{
		m_Sleep = Time;
		Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, Time*17, Time*17, true);
		GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_ZZZ);
	}
}


void CAI::Stun(int Time)
{
	if (!Player()->GetCharacter())
		return;

	
	if (m_Stun < Time)
	{
		m_Stun = Time;
		Player()->GetCharacter()->SetEmoteFor(EMOTE_SURPRISE, Time*17, Time*17, true);
	}
}


void CAI::StandStill(int Time)
{
	m_Sleep = Time;
}


void CAI::UpdateInput(int *Data)
{
	m_InputChanged = false;
	Data[0] = m_Move;
	Data[1] = m_Direction.x;
	Data[2] = m_Direction.y;
	Data[3] = m_Jump;
	Data[4] = m_Attack;
	Data[5] = m_Hook;
}




void CAI::AirJump()
{
	if (Player()->GetCharacter()->GetVel().y > 0 && m_TargetPos.y + 80 < m_Pos.y )
	{
		if (!GameServer()->Collision()->FastIntersectLine(m_Pos, m_Pos+vec2(0, 100)) && frandom()*10 < 5)
			m_Jump = 1;
	}
}



void CAI::UpdateWaypoint()
{
	if (distance(m_WaypointPos, m_Pos) < 100 || m_TargetTimer > 30)// && m_WayPointUpdateWait > 10)
	{
		m_TargetTimer = 0;
		
		if (GameServer()->Collision()->FindPath(m_Pos, m_TargetPos, m_Move))
		{
			if (GameServer()->Collision()->m_GotVision)
			{
				// we got a waypoint towards the target
				m_WaypointPos = GameServer()->Collision()->m_VisionPos;
				m_WaypointDir = m_WaypointPos - m_Pos;
				m_WayPointUpdateWait = 0;
				m_WayFound = true;
				
				//char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Max search distance: %d", GameServer()->Collision()->m_MaxDistance);
				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "path", aBuf);
			}
		}
		else
		{
			m_WayFound = false;
		}
		
		if (GameServer()->m_ShowWaypoints)
		{
			for (int i = 0; i < 98; i++)
				new CStaticlaser(&GameServer()->m_World, GameServer()->Collision()->m_aPath[i]+vec2(16, 16), GameServer()->Collision()->m_aPath[i+1]+vec2(16, 16), 10);
		}
		
		if (GameServer()->m_ShowWaypoints)
			new CStaticlaser(&GameServer()->m_World, m_Pos, m_WaypointPos, 10);
	}
	
	m_WayPointUpdateWait++;
	m_TargetTimer++;
}



void CAI::HookMove()
{
	// release hook if hooking an ally
	if (Player()->GetCharacter()->HookedPlayer() >= 0 && m_Hook == 1)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[Player()->GetCharacter()->HookedPlayer()];
		if(pPlayer && pPlayer->GetTeam() == Player()->GetTeam())
			m_Hook = 0;
	}
	
	// ...or not hooking at all
	if (!Player()->GetCharacter()->Hooking())
	{
		if (m_Hook == 1)
			m_Hook = 0;
	}
	
	
	// hook
	if (m_LastHook == 0 && m_HookTimer++ > 2)
	{
		vec2 HookDir = m_WaypointPos - m_Pos;
		float Angle = atan2(HookDir.x, HookDir.y);
			
		float MaxDist = 0;
		vec2 FinalHookPos = vec2(0, 0);
		
		//for (int i = -19; i <= 19; i++)
		for (int i = 0; i <= 36; i++)
		{
			//float a = Angle + i*0.02f;
			float a = i*10;
			
			// dont hook downwards
			if (a < 90 || a > 270)
				continue;
			
			// dont hook upwards if going down
			if (a > 70 && a < 290 && HookDir.y > 10)
				continue;
			
			// dont hook backwards
			if ((HookDir.x < 0 && a < 180) || (HookDir.x > 0 && a > 180))
				continue;
				
			vec2 HookPos = m_Pos + vec2(sin(a*RAD)*380, cos(a*RAD)*380);

			// hook if something in sight
			int C = GameServer()->Collision()->IntersectLine(m_Pos, HookPos, &HookPos, NULL);
			if (C&CCollision::COLFLAG_SOLID && !(C&CCollision::COLFLAG_NOHOOK) && m_LastHook == 0)
			{
				float Dist = distance(m_Pos, HookPos);
				if (Dist > 60 && Dist > MaxDist)
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
	
	// lock move direction
	if (m_Hook != 0 && Player()->GetCharacter()->Hooking())
	{
		vec2 CorePos = Player()->GetCharacter()->GetCore().m_Pos;
		vec2 HookPos = Player()->GetCharacter()->GetCore().m_HookPos;
		vec2 Vel = Player()->GetCharacter()->GetVel();
		
		if (distance(CorePos, HookPos) > 40)
		{
			if (Vel.x > 0 && HookPos.y < CorePos.y)
				m_Move = 1;
			if (Vel.x < 0 && HookPos.y < CorePos.y)
				m_Move = -1;
			if (HookPos.x > CorePos.x && HookPos.y > CorePos.y)
				m_Move = 1;
			if (HookPos.x < CorePos.x && HookPos.y > CorePos.y)
				m_Move = -1;
		}
	}
	
	// release hook
	if (m_Hook == 1)
	{
		if (m_HookReleaseTimer++ > 6 + frandom()*6)
		{
			m_Hook = 0;
			m_HookReleaseTimer = 0;
			m_HookTimer = 0;
		}
	}
	else
		m_HookReleaseTimer = 0;
}




void CAI::JumpIfPlayerIsAbove()
{
		if (abs(m_PlayerPos.x - m_Pos.x) < 100 && m_Pos.y > m_PlayerPos.y + 100)
		{
			if (frandom() * 10 < 4)
				m_Jump = 1;
		}
}

void CAI::HeadToMovingDirection()
{
	if (m_Move != 0)
		m_Direction = vec2(m_Move, 0);
}

void CAI::Unstuck()
{
	if (abs(m_Pos.x - m_StuckPos.x) < 10)
	{
		if (++m_UnstuckCount > 10)
		{
			if (frandom() * 10 < 5)
				m_Move = -1;
			else
				m_Move = 1;
			
			if (frandom() * 10 < 4)
				m_Jump = 1;
		}
	}
	else
	{
		m_UnstuckCount = 0;
		m_StuckPos = m_Pos;
	}
}



void CAI::SeekBombArea()
{
	CFlag *BombArea = GameServer()->m_pController->GetClosestBombArea(m_LastPos);
	
	if (BombArea)
		m_TargetPos = BombArea->m_Pos;
}
	
void CAI::SeekBomb()
{
	CBomb *Bomb = GameServer()->m_pController->GetBomb();
	
	if (Bomb)
		m_TargetPos = Bomb->m_Pos;
}


bool CAI::MoveTowardsPlayer(int Dist)
{
	if (abs(m_LastPos.x - m_PlayerPos.x) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_PlayerPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_PlayerPos.x)
		m_Move = -1;
		
	return false;
}


bool CAI::MoveTowardsTarget(int Dist)
{

	if (abs(m_LastPos.x - m_TargetPos.x) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_TargetPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_TargetPos.x)
		m_Move = -1;
		
	return false;
}


bool CAI::MoveTowardsWaypoint(int Dist)
{

	if (distance(m_LastPos, m_WaypointPos) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_WaypointPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_WaypointPos.x)
		m_Move = -1;
		
	return false;
}


void CAI::ReceiveDamage()
{
	
}



int CAI::WeaponShootRange()
{
	int Weapon = Player()->GetCharacter()->m_ActiveCustomWeapon;
	int Range = 40;
		
	if (Weapon >= 0 && Weapon < NUM_CUSTOMWEAPONS)
		Range = BotAttackRange[Weapon];
	
	return Range;
}



void CAI::ShootAtClosestEnemy()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 800 && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
			//!GameServer()->Collision()->IntersectLine(pCharacter->m_Pos, m_LastPos, NULL, NULL))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (pClosestCharacter && ClosestDistance < WeaponShootRange()*1.2f)
	{
		if (ClosestDistance < WeaponShootRange())
			m_Attack = 1;
		
		m_Direction = vec2(m_PlayerDirection.x+ClosestDistance*(frandom()*0.3f-frandom()*0.3f), m_PlayerDirection.y+ClosestDistance*(frandom()*0.3f-frandom()*0.3f));
	}
	
	// ammo check
	if (Player()->GetCharacter()->m_ActiveCustomWeapon != HAMMER_BASIC && Player()->GetCharacter()->m_ActiveCustomWeapon != SWORD_KATANA &&
		!Player()->GetCharacter()->HasAmmo())
		Player()->GetCharacter()->SetCustomWeapon(HAMMER_BASIC);
}


void CAI::RandomlyStopShooting()
{
	if (frandom()*20 < 4 && m_Attack == 1)
	{
		m_Attack = 0;
		
		m_AttackTimer = g_Config.m_SvBotReactTime*0.5f;
	}
}




bool CAI::SeekClosestEnemy()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (!pClosestCharacter || Distance < ClosestDistance)
		{
			pClosestCharacter = pCharacter;
			ClosestDistance = Distance;
			m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
			m_PlayerPos = pCharacter->m_Pos;
		}
	}
	
	if (pClosestCharacter)
	{
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	return false;
}


bool CAI::SeekClosestEnemyInSight()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 800 && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
			//!GameServer()->Collision()->IntersectLine(pCharacter->m_Pos, m_LastPos, NULL, NULL))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (pClosestCharacter)
	{
		m_PlayerSpotCount++;
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	m_PlayerSpotCount = 0;
	return false;
}





void CAI::Tick()
{
	m_NextReaction--;
	
	// character check & position update
	if (m_pPlayer->GetCharacter())
		m_Pos = m_pPlayer->GetCharacter()->m_Pos;
	else
		return;
		
	
	// skip if sleeping or stunned
	if (m_Sleep > 0 || m_Stun > 0)
	{
		if (m_Sleep > 0)
			m_Sleep--;
			
		if (m_Stun > 0)
			m_Stun--;
		
		m_Move = 0;
		m_Jump = 0;
		m_Hook = 0;
		m_Attack = 0;
		m_InputChanged = true;
		
		return;
	}
	
	// stupid AI can't even react to events every frame
	if (m_NextReaction <= 0)
	{
		m_NextReaction = m_ReactionTime;
	
		DoBehavior();
		
		if (m_pPlayer->GetCharacter())
			m_LastPos = m_pPlayer->GetCharacter()->m_Pos;
		m_LastMove = m_Move;
		m_LastJump = m_Jump;
		m_LastAttack = m_Attack;
		m_LastHook = m_Hook;
			
		m_InputChanged = true;
	}
	else
	{
		m_Attack = 0;
	}
}