/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

// this include should perhaps be removed
#include "entities/character.h"
#include "gamecontext.h"
#include "ai.h"

#include <game/server/upgradelist.h>
#include <game/server/classabilities.h>



enum TryBuyWeapon
{
	 BUY_OK = 0,
	 BUY_NOCHARACTER = -1,
	 BUY_NOPOINTS = -2,
	 BUY_HASALREADY = -3,
	 BUY_NOWEAPON = -4,
	 BUY_CLIPSFULL = -5,
	 BUY_INVALIDWEAPON = -6
};



// player object
class CPlayer
{
	MACRO_ALLOC_POOL_ID()

public:
	CPlayer(CGameContext *pGameServer, int ClientID, int Team);
	~CPlayer();
	
	void Init(int CID);

	void NewRound();
	
	void TryRespawn();
	void Respawn();
	void SetTeam(int Team, bool DoChatMsg=true);
	void SetWantedTeam(int Team, bool DoChatMsg=true);
	//int GetTeam() const { return m_Team; };
	
	int GetTeam()
	{
		if (m_Team != TEAM_SPECTATORS);
			return m_Team;
		return m_WantedTeam;
	};
	
	int GetCID() const { return m_ClientID; };

	void Tick();
	void PostTick();
	void Snap(int SnappingClient);

	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);
	void OnDisconnect(const char *pReason);

	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();

	void GiveCharacterAway()
	{
		m_pCharacter = NULL;
	}
	
	void GiveCharacter(CCharacter *Character)
	{
		m_pCharacter = Character;
	}
	
	// bomb planting & defusing
	int m_ActionTimer;
	
	int m_DeathTick;
	
	//---------------------------------------------------------
	// this is used for snapping so we know how we can clip the view for the player
	vec2 m_ViewPos;

	// states if the client is chatting, accessing a menu etc.
	int m_PlayerFlags;

	// used for snapping to just update latency if the scoreboard is active
	int m_aActLatency[MAX_CLIENTS];

	// used for spectator mode
	int m_SpectatorID;

	bool m_IsReady;
	
	bool m_BroadcastingCaptureStatus;

	//
	int m_Vote;
	int m_VotePos;
	//
	int m_LastVoteCall;
	int m_LastVoteTry;
	int m_LastChat;
	int m_LastSetTeam;
	int m_LastSetSpectatorMode;
	int m_LastChangeInfo;
	int m_LastEmote;
	int m_LastKill;

	// TODO: clean this up
	struct
	{
		char m_SkinName[64];
		int m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;
	} m_TeeInfos;
	
	// for shopping system
	int m_Money;
	bool m_CanShop;
	
	void DisableShopping();
	void EnableShopping();
	
	void BuyRandomWeapon();
	
	float m_InterestPoints;
	
	int m_RespawnTick;
	int m_DieTick;
	int m_Score;
	int m_ScoreStartTick;
	bool m_ForceBalanced;
	int m_LastActionTick;
	int m_TeamChangeTick;
	struct
	{
		int m_TargetX;
		int m_TargetY;
	} m_LatestActivity;

	// network latency calculations
	struct
	{
		int m_Accum;
		int m_AccumMin;
		int m_AccumMax;
		int m_Avg;
		int m_Min;
		int m_Max;
	} m_Latency;
	
	CAI *m_pAI;
	bool m_IsBot;
	
	void AITick();
	bool AIInputChanged();
	
	
	// custom
	bool BuyableWeapon(int i);
	void ListBuyableWeapons();
	//void VoteBuyableWeapons();
	bool BuyWeapon(int CustomWeapon);
	
	bool m_aSavedWeapon[NUM_CUSTOMWEAPONS];
	
	bool GotWeapon(int CustomWeapon);
	void SaveWeapons();
	void GiveSavedWeapons();
	void EraseWeapons();
	
	bool WeaponDisabled(int CustomWeapon)
	{
		if (!GetCharacter())
			return false;
		return m_pCharacter->WeaponDisabled(CustomWeapon);
	}
	
	void JoinTeam();
	
	bool m_ForceToSpectators;

	int m_WantedTeam;
	
	// settings
	int m_EnableWeaponInfo; // 0 = disabled, 1 = chat, 2 = broadcast
	bool m_EnableAutoSpectating;
	bool m_EnableEmoticonGrenades;
	
	// warm welcome
	bool m_Welcomed;
	
	void SetRandomSkin();
	
	
	
	int GetClass() { return m_Class; }
	int m_AbilityPoints;
	int GetAbilityPoints() { return m_AbilityPoints; }
	
	void ResetClass();
	bool SelectClass(int Class);
	bool SelectAbility(int Ability);
	
	bool SetClass(int WantedClass)
	{
		if (m_Class != -1)
			return false;
		
		m_Class = WantedClass;
		return true;
	}
	
	bool AbilityAvailable(int Ability)
	{
		if (Ability < 0 || Ability >= NUM_ABILITIES)
			return false;
		
		if (m_Class == -1)
			return false;
		
		if (m_aAbility[Ability] || (aAbilities[Ability].m_Class != -1 && aAbilities[Ability].m_Class != m_Class))
			return false;
	
		int Require = aAbilities[Ability].m_Require;
		if (Require != -1 && !m_aAbility[Require])
			return false;

		return true;
	}
	
	bool GotAbility(int Ability)
	{
		if (Ability < 0 || Ability >= NUM_ABILITIES)
			return false;
		
		return m_aAbility[Ability];
	}
	
	int MedkitSize()
	{
		int Size = 0;
		if (GotAbility(STORE_HEALTH))
			Size += 3;
		if (GotAbility(MEDKIT_EXPANSION))
			Size += 3;
		return Size;
	}
	
private:
	int m_Class;
	bool m_aAbility[NUM_ABILITIES];

	CCharacter *m_pCharacter;
	CGameContext *m_pGameServer;

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const;

	//
	void ForceToSpectators();
	
	//
	bool m_Spawning;
	int m_ClientID;
	int m_Team;
};

#endif
