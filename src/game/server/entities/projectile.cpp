/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "projectile.h"
#include "electro.h"
#include "superexplosion.h"
#include "smokescreen.h"

#include <game/server/classabilities.h>

CProjectile::CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon, int ExtraInfo)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Type = Type;
	m_Pos = Pos;
	m_Direction = Dir;
	m_LifeSpan = Span;
	m_Owner = Owner;
	m_Force = Force;
	m_Damage = Damage;
	m_SoundImpact = SoundImpact;
	m_Weapon = Weapon;
	m_StartTick = Server()->Tick();
	m_Explosive = Explosive;
	m_ExtraInfo = ExtraInfo;

	m_ElectroTimer = 0;
	
	GameWorld()->InsertEntity(this);
}

void CProjectile::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CProjectile::GetPos(float Time)
{
	float Curvature = 0;
	float Speed = 0;

	switch(m_Type)
	{
		case WEAPON_GRENADE:
			Curvature = GameServer()->Tuning()->m_GrenadeCurvature;
			Speed = GameServer()->Tuning()->m_GrenadeSpeed;
			break;

		case WEAPON_SHOTGUN:
			Curvature = GameServer()->Tuning()->m_ShotgunCurvature;
			Speed = GameServer()->Tuning()->m_ShotgunSpeed;
			break;

		case WEAPON_GUN:
			Curvature = GameServer()->Tuning()->m_GunCurvature;
			Speed = GameServer()->Tuning()->m_GunSpeed;
			break;
	}

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


void CProjectile::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0);
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, CurPos, OwnerChar);

	m_LifeSpan--;
	
	//if (m_ExtraInfo == SMOKE)
	//	GameServer()->CreateDeath(CurPos, -1);


	// grenade electricity
	if(m_Explosive && OwnerChar && GameServer()->GotAbility(m_Owner, ELECTRO_GRENADES) && m_ElectroTimer++ > 2)
	{
		m_ElectroTimer = 0;
		float Reach = 110.0f;
		if (GameServer()->GotAbility(m_Owner, ELECTRO_REACH1))
			Reach *= 1.33f;
		vec2 Dir = normalize(vec2(frandom()-frandom(), frandom()-frandom()));
		
		for (int i = -1; i <= 1; i += 2)
		{
			vec2 To = CurPos + Dir * Reach * i;
			vec2 Start = CurPos;
			GameServer()->Collision()->IntersectLine(Start, To, 0x0, &To);
					
			// character collision
			vec2 At;
			CCharacter *Owner = NULL;
			
			if (GameServer()->m_apPlayers[m_Owner])
				Owner = GameServer()->m_apPlayers[m_Owner]->GetCharacter();
			
			CCharacter *pHit = GameServer()->m_World.IntersectCharacter(CurPos, To, 60.0f, At, Owner);
			if(pHit)
			{
				To = pHit->m_Pos;
				pHit->ElectroShock();
					
				int Damage = 4;
				if (GameServer()->GotAbility(m_Owner, ELECTRO_DAMAGE1))
					Damage++;
					
				pHit->TakeDamage(vec2(0, 0), Damage, m_Owner, WEAPON_GRENADE);
			}
				
			int A = distance(CurPos, To) / 100;
						
			if (A > 4)
				A = 4;
						
			if (A < 2)
				A = 2;
				
			new CElectro(GameWorld(), CurPos, To, vec2(0, 0), A);
		}
	}


	if(TargetChr || Collide || m_LifeSpan < 0 || GameLayerClipped(CurPos))
	{
		if(m_LifeSpan >= 0 || m_Weapon == WEAPON_GRENADE)
			GameServer()->CreateSound(CurPos, m_SoundImpact);

		if (m_ExtraInfo == SMOKE)
		{
			CSmokescreen *S = new CSmokescreen(&GameServer()->m_World, CurPos, Server()->TickSpeed()*6);
			GameServer()->m_World.InsertEntity(S);
		}
		
		if(m_Explosive)
		{
			
			if (m_ExtraInfo == MEGAROCKETS)
			{
				/*GameServer()->CreateExplosion(CurPos+vec2(-32, -32), m_Owner, m_Weapon, false);
				GameServer()->CreateExplosion(CurPos+vec2(+32, -32), m_Owner, m_Weapon, false);
				GameServer()->CreateExplosion(CurPos+vec2(+32, +32), m_Owner, m_Weapon, false);
				GameServer()->CreateExplosion(CurPos+vec2(-32, +32), m_Owner, m_Weapon, false);*/
				
				CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, CurPos, m_Owner, m_Weapon, 1);
				GameServer()->m_World.InsertEntity(S);
			}
			else if (m_ExtraInfo == DOOMROCKETS)
			{
				CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, CurPos, m_Owner, m_Weapon, 2);
				GameServer()->m_World.InsertEntity(S);
			}
			else
				GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, false);
		}

		else if(TargetChr)
		{
			// apply sleep effect
			if (TargetChr->GetPlayer()->m_pAI && OwnerChar)
			{
				//if (OwnerChar->GetPlayer()->HasWeaponUpgrade(m_Weapon, UPG_SLEEPEFFECT))
				//	TargetChr->GetPlayer()->m_pAI->Zzz(40);
			}
			
			TargetChr->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, m_Weapon);
		}
			
		GameServer()->m_World.DestroyEntity(this);
	}
}

void CProjectile::TickPaused()
{
	++m_StartTick;
}

void CProjectile::FillInfo(CNetObj_Projectile *pProj)
{
	pProj->m_X = (int)m_Pos.x;
	pProj->m_Y = (int)m_Pos.y;
	pProj->m_VelX = (int)(m_Direction.x*100.0f);
	pProj->m_VelY = (int)(m_Direction.y*100.0f);
	pProj->m_StartTick = m_StartTick;
	pProj->m_Type = m_Type;
}

void CProjectile::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();

	if(NetworkClipped(SnappingClient, GetPos(Ct)))
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(pProj)
		FillInfo(pProj);
}
