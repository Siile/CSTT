#ifndef GAME_SERVER_AI_CSBB_AI_H
#define GAME_SERVER_AI_CSBB_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIcsbb : public CAI
{
public:
	CAIcsbb(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);
	
private:
	int m_SkipMoveUpdate;
	
};

#endif