#ifndef GAME_SERVER_GAMEMODES_CSTT_H
#define GAME_SERVER_GAMEMODES_CSTT_H
#include <game/server/gamecontroller.h>
#include <game/server/entity.h>

#define MAX_BOMBAREAS 10
#define MAX_LASERWALLS 200
#define MAX_PICKUPS 1024
#define MAX_DROPPABLES 30



enum GameState
{
	GAMESTATE_NEWGAME,
	GAMESTATE_STARTING,
	GAMESTATE_ROUND,
	GAMESTATE_ENDING,
};


class CGameControllerCSTT : public IGameController
{
private:
	bool m_DroppablesCreated;
	
	int m_GameState;
	int m_Round;
	int m_MaxRounds;
	int m_RoundTick;
	
	
	void Restart();
	void StartCountdown();
	void StartRound();
	
	void RoundWinLose();
	void RoundRewards(int WinningTeam);
	
	void SaveWeapons();
	
	int CheckLose();
	void AutoBalance();
	
	void EndTheShit();
	
	bool m_NewGame;
	
	bool m_aDefusing[MAX_CLIENTS];
	
	int m_BroadcastTimer;
	
	int m_BombSoundTimer;
	int m_BombActionTimer;
	
	
	int CountPlayers();
	int CountPlayersAlive();
		
	bool m_Timeout;
	
	// 
	class CBomb *m_pBomb; // red flag
	class CFlag *m_apBombArea[MAX_BOMBAREAS]; // blue flags
	int m_BombAreaCount;
	
	int m_BombDefuseTimer;
	bool m_BombDefused;
	
	class CFlag *GetClosestBombArea(vec2 Pos);
	
	int m_BombCarrierTurn;
	
	void GiveBombToPlayer();
	
	// store pickup pointers
	class CPickup *m_apPickup[MAX_PICKUPS];
	int m_PickupCount;
	
	// for item drops
	int m_PickupDropCount;
	
	void CreateDroppables();
	
	void RespawnPickups();
	void ClearPickups();
	
	bool ForceNogo(vec2 Pos);
	
	int m_LastReleaseSound;

	
	class CStaticlaser *m_apLaserwall[MAX_LASERWALLS];
	void AddLaserwall(vec2 Pos);
	int m_LaserwallCount;

	
	void ResetLaserwalls();
	void ReleaseLaserwalls(vec2 Pos);
	
public:
	CGameControllerCSTT(class CGameContext *pGameServer);
	
	virtual void DoWincheck();
	virtual bool CanBeMovedOnBalance(int ClientID);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
	
	virtual void DropPickup(vec2 Pos, int PickupType, int PickupSubtype = -1);
	void FlashPickups();
	
	
	virtual bool OnEntity(int Index, vec2 Pos);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	bool CanCharacterSpawn(int ClientID);
};
#endif
