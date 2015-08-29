#ifndef GAME_SERVER_UPGRADELIST_H
#define GAME_SERVER_UPGRADELIST_H

#include <cstring>
#include <game/generated/protocol.h>


#define MAX_GRENADES 5


struct CWeapon
{
	char m_Name[64];
	char m_BuyCmd[64];
	int m_ParentWeapon;
	int m_ProjectileType;
	int m_Sound;
	int m_Cost;
	int m_Damage;
	bool m_FullAuto;
	int m_ShotSpread;
	int m_ClipSize;
	int m_PowerupSize;
	int m_MaxAmmo;
	float m_BulletLife;
	int m_BulletReloadTime;
	int m_ClipReloadTime;
	float m_Knockback;
	int m_Extra1;
	int m_Require;


	CWeapon(const char *Name,
			const char *BuyCmd,
			int ParentWeapon,
			int ProjectileType,
			int Sound,
			int Require,
			int Cost,
			int Damage,
			int Extra1,
			bool FullAuto,
			int ShotSpread,
			int ClipSize,
			int PowerupSize,
			int MaxAmmo,
			float BulletLife,
			int BulletReloadTime,
			int ClipReloadTime,
			float Knockback)
	{
	    str_copy(m_Name, Name, sizeof(m_Name));
	    str_copy(m_BuyCmd, BuyCmd, sizeof(m_BuyCmd));
		m_ParentWeapon = ParentWeapon;
		m_ProjectileType = ProjectileType;
		m_Sound = Sound;
		m_Require = Require;
		m_Cost = Cost;
		m_Damage = Damage;
		m_Extra1 = Extra1;
		m_FullAuto = FullAuto;
		m_ShotSpread = ShotSpread;
		m_ClipSize = ClipSize;
		m_PowerupSize = PowerupSize;
		m_MaxAmmo = MaxAmmo;
		m_BulletLife = BulletLife;
		m_BulletReloadTime = BulletReloadTime;
		m_ClipReloadTime = ClipReloadTime;
		m_Knockback = Knockback;
	}
};

enum CustomWeapons
{
	SWORD_KATANA,
	HAMMER_BASIC,
	SHOTGUN_DOUBLEBARREL,
	SHOTGUN_COMBAT,
	GUN_PISTOL,
	GUN_MAGNUM,
	RIFLE_ASSAULTRIFLE,
	RIFLE_HEAVYRIFLE,
	RIFLE_LIGHTNINGRIFLE,
	RIFLE_STORMRIFLE,
	RIFLE_LASERRIFLE,
	RIFLE_DOOMRAY,
	GRENADE_GRENADELAUNCHER,
	GRENADE_HEAVYLAUNCHER,
	GRENADE_DOOMLAUNCHER,
	GRENADE_GRENADE,
	NUM_CUSTOMWEAPONS
};


const int BotAttackRange[NUM_CUSTOMWEAPONS] =
{
	400, // SWORD_KATANA,
	120, // HAMMER_BASIC,
	400, // SHOTGUN_DOUBLEBARREL,
	400, // SHOTGUN_COMBAT,
	700, // GUN_PISTOL,
	700, // GUN_MAGNUM,
	700, // RIFLE_ASSAULTRIFLE,
	700, // RIFLE_HEAVYRIFLE,
	400, // RIFLE_LIGHTNINGRIFLE,
	600, // RIFLE_STORMRIFLE,
	700, // RIFLE_LASERRIFLE,
	700, // RIFLE_DOOMRAY,
	500, // GRENADE_GRENADELAUNCHER,
	500, // GRENADE_HEAVYLAUNCHER,
	500, // GRENADE_DOOMLAUNCHER,
	500, // GRENADE_GRENADE
};


enum ProjectileTypes
{
	PROJTYPE_NONE,
	PROJTYPE_HAMMER,
	PROJTYPE_FLYHAMMER,
	PROJTYPE_SWORD,
	PROJTYPE_BULLET,
	PROJTYPE_PELLET,
	PROJTYPE_LASER,
	PROJTYPE_LIGHTNING,
	PROJTYPE_GRENADE,
	PROJTYPE_MINE
};

enum WeaponExtraFeature
{
	NO_EXTRA1,
	MEGAROCKETS,
	DOOMROCKETS,
	BULLETSPREAD,
	BIGBULLETSPREAD,
	SLEEPEFFECT,
	ELECTRIC,
	SMOKE,
	NUM_EXTRA_FEATURES
};


const CWeapon aCustomWeapon[NUM_CUSTOMWEAPONS] =
{
	CWeapon(
		"Katana",
		"/buy katana",
		WEAPON_HAMMER,
		PROJTYPE_SWORD,
		SOUND_HAMMER_FIRE,
		-1, // require
		70, // cost
		17, // damage
		0, // extra1
		false, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0.75f, // bullet life - vanilla ninja life * bulletlife
		550, // bullet reload time
		0, // clip reload time
		0.0f // knockback
		),
	CWeapon(
		"Hammer",
		"/buy hammer",
		WEAPON_HAMMER,
		PROJTYPE_HAMMER,
		SOUND_HAMMER_FIRE,
		-1, // require
		0, // cost
		15, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		170, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Double barrel shotgun",
		"/buy double",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		-1, // require
		75, // cost
		6, // damage
		0, // extra1
		false, // autofire
		6, // bullet spread
		2, // clip size
		6, // powerup size
		20, // max ammo
		0.17f, // bullet life
		225, // bullet reload time
		750, // clip reload time
		7.0f // bullet knockback
		),
	CWeapon(
		"Combat shotgun",
		"/upg combat",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		SHOTGUN_DOUBLEBARREL, // require
		100, // cost
		7, // damage
		0, // extra1
		true, // autofire
		5, // bullet spread
		10, // clip size
		10, // powerup size
		100, // max ammo
		0.2f, // bullet life
		215, // bullet reload time
		1200, // clip reload time
		6.0f // bullet knockback
		),
	CWeapon(
		"Pistol",
		"/buy pistol",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		0, // cost
		11, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		7, // clip size
		14, // powerup size
		70, // max ammo
		0.3f, // bullet life
		160, // bullet reload time
		650, // clip reload time
		10.0f // bullet knockback
		),
	CWeapon(
		"Magnum .9000",
		"/upg magnum",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		GUN_PISTOL, // require
		50, // cost
		19, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		6, // clip size
		12, // powerup size
		60, // max ammo
		0.35f, // bullet life
		160, // bullet reload time
		700, // clip reload time
		15.0f // bullet knockback
		),
	CWeapon(
		"Assault rifle",
		"/buy rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		100, // cost
		13, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0.45f, // bullet life
		120, // bullet reload time
		1200, // clip reload time
		12.0f // bullet knockback
		),
	CWeapon(
		"Heavy assault rifle",
		"/upg rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		RIFLE_ASSAULTRIFLE, // require
		100, // cost
		22, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0.45f, // bullet life
		120, // bullet reload time
		1200, // clip reload time
		17.0f // bullet knockback
		),
	CWeapon(
		"Lightning rifle",
		"/upg lightning",
		WEAPON_RIFLE,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE,
		-1, // require
		100, // cost
		8, // damage
		BIGBULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0, // bullet life
		125, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Storm rifle",
		"/upg storm",
		WEAPON_RIFLE,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE,
		RIFLE_LIGHTNINGRIFLE, // require
		100, // cost
		7, // damage
		ELECTRIC, // extra1
		true, // autofire
		1, // bullet spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0, // bullet life
		125, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Laser rifle",
		"/buy laser",
		WEAPON_RIFLE,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		-1, // require
		100, // cost
		15, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		6, // clip size
		6, // powerup size
		60, // max ammo
		0, // bullet life
		250, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Doom ray",
		"/upg doomray",
		WEAPON_RIFLE,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		RIFLE_LASERRIFLE, // require
		150, // cost
		15, // damage
		DOOMROCKETS, // extra1
		false, // autofire
		1, // bullet spread
		4, // clip size
		4, // powerup size
		20, // max ammo
		0, // bullet life
		400, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Grenade launcher",
		"/buy grenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		-1, // require
		100, // cost
		10, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		6, // clip size
		12, // powerup size
		60, // max ammo
		1.4f, // bullet life
		300, // bullet reload time
		800, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Heavy launcher",
		"/upg heavy",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		GRENADE_GRENADELAUNCHER, // require
		100, // cost
		12, // damage
		MEGAROCKETS, // extra1
		true, // autofire
		1, // bullet spread
		4, // clip size
		8, // powerup size
		40, // max ammo
		1.5f, // bullet life
		330, // bullet reload time
		1000, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Doom launcher",
		"/upg doom",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		GRENADE_HEAVYLAUNCHER, // require
		100, // cost
		14, // damage
		DOOMROCKETS, // extra1
		true, // autofire
		1, // bullet spread
		3, // clip size
		6, // powerup size
		30, // max ammo
		1.5f, // bullet life
		360, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Smoke grenade",
		"/buy grenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		-1, // require
		0, // cost
		0, // damage
		SMOKE, // extra1
		false, // autofire
		0, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		1.2f, // bullet life
		0, // bullet reload time
		0, // clip reload time
		0.0f // bullet knockback
		)
};



#endif