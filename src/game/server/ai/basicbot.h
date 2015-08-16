#ifndef GAME_SERVER_AI_BASICBOT_H
#define GAME_SERVER_AI_BASICBOT_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIBasicbot : public CAI
{
public:
	CAIBasicbot(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
};

#endif