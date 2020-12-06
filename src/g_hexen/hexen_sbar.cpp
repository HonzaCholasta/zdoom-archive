#include "doomtype.h"
#include "doomstat.h"
#include "v_font.h"
#include "sbar.h"
#include "r_defs.h"
#include "w_wad.h"
#include "m_random.h"
#include "d_player.h"
#include "st_stuff.h"
#include "v_video.h"
#include "r_draw.h"
#include "templates.h"
#include "a_hexenglobal.h"

class FManaBar : public FTexture
{
public:
	FManaBar ();

	const BYTE *GetColumn (unsigned int column, const Span **spans_out);
	const BYTE *GetPixels ();
	void Unload ();
	bool CheckModified ();

	void SetVial (FTexture *pic, int level);

protected:
	BYTE Pixels[5*24];
	static const Span DummySpan[2];

	FTexture *VialPic;
	int VialLevel;
	bool NeedRefresh;

	void MakeTexture ();
};

const FTexture::Span FManaBar::DummySpan[2] = { { 0, 24 }, { 0, 0 } };

FManaBar::FManaBar ()
: VialPic(0), VialLevel(0), NeedRefresh(false)
{
	Width = 5;
	Height = 24;
	WidthBits = 2;
	HeightBits = 5;
	WidthMask = 3;
}

void FManaBar::Unload ()
{
	if (VialPic != 0)
	{
		VialPic->Unload ();
	}
}

bool FManaBar::CheckModified ()
{
	return NeedRefresh;
}

const BYTE *FManaBar::GetColumn (unsigned int column, const Span **spans_out)
{
	if (NeedRefresh)
	{
		MakeTexture ();
	}
	if (column > 4)
	{
		column = 4;
	}
	if (spans_out != NULL)
	{
		*spans_out = DummySpan;
	}
	return Pixels + column*24;
}

const BYTE *FManaBar::GetPixels ()
{
	if (NeedRefresh)
	{
		MakeTexture ();
	}
	return Pixels;
}

void FManaBar::SetVial (FTexture *pic, int level)
{
	if (level > 22)
	{
		level = 22;
	}
	if (VialPic != pic || VialLevel != level)
	{
		VialPic = pic;
		VialLevel = level;
		NeedRefresh = true;
	}
}

void FManaBar::MakeTexture ()
{
	int run = 22 - VialLevel;
	BYTE color0 = GPalette.Remap[0];

	NeedRefresh = false;
	VialPic->CopyToBlock (Pixels, 5, 24, 0, 0);
	memset (Pixels + 25, color0, run);
	memset (Pixels + 25+24, color0, run);
	memset (Pixels + 25+24+24, color0, run);
}

class FHexenStatusBar : public FBaseStatusBar
{
public:
	FHexenStatusBar () : FBaseStatusBar (38)
	{
		static const char *hexenLumpNames[NUM_HEXENSB_IMAGES] =
		{
			"H2BAR",	"H2TOP",	"INVBAR",	"LFEDGE",	"RTEDGE",
			"STATBAR",	"KEYBAR",	"SELECTBO",	"ARTICLS",	"ARMCLS",
			"MANACLS",	"MANAVL1",	"MANAVL2",	"MANAVL1D",	"MANAVL2D",
			"MANADIM1",	"MANADIM2",	"MANABRT1",	"MANABRT2",	"INVGEML1",
			"INVGEML2",	"INVGEMR1",	"INVGEMR2",	"KILLS",	"USEARTIA",
			"USEARTIB",	"USEARTIC",	"USEARTID",	"USEARTIE",	"KEYSLOT1",
			"KEYSLOT2",	"KEYSLOT3",	"KEYSLOT4",	"KEYSLOT5",	"KEYSLOT6",
			"KEYSLOT7",	"KEYSLOT8",	"KEYSLOT9",	"KEYSLOTA",	"KEYSLOTB",
			"ARMSLOT1",	"ARMSLOT2",	"ARMSLOT3",	"ARMSLOT4",	"ARTIBOX"
		};
		static const char *classLumpNames[3][NUM_HEXENCLASSSB_IMAGES] =
		{
			{
				"WPSLOT0",	"WPFULL0",	"WPIECEF1",	"WPIECEF2",
				"WPIECEF3",	"CHAIN",	"LIFEGMF2"
			},
			{
				"WPSLOT1",	"WPFULL1",	"WPIECEC1",	"WPIECEC2",
				"WPIECEC3",	"CHAIN2",	"LIFEGMC2"
			},
			{
				"WPSLOT2",	"WPFULL2",	"WPIECEM1",	"WPIECEM2",
				"WPIECEM3",	"CHAIN3",	"LIFEGMM2"
			}
		};
		static const char *sharedLumpNames[] =
		{
			"LAME",		"NEGNUM",	"IN0",		"IN1",		"IN2",
			"IN3",		"IN4",		"IN5",		"IN6",		"IN7",
			"IN8",		"IN9",		"FONTB13",	"FONTB16",	"FONTB17",
			"FONTB18",	"FONTB19",	"FONTB20",	"FONTB21",	"FONTB22",
			"FONTB23",	"FONTB24",	"FONTB25",	"SMALLIN0",	"SMALLIN1",
			"SMALLIN2",	"SMALLIN3",	"SMALLIN4",	"SMALLIN5",	"SMALLIN6",
			"SMALLIN7",	"SMALLIN8",	"SMALLIN9",

			"INRED0",	"INRED1",	"INRED2",	"INRED3",	"INRED4",
			"INRED5",	"INRED6",	"INRED7",	"INRED8",	"INRED9"
		};

		FBaseStatusBar::Images.Init (sharedLumpNames, NUM_BASESB_IMAGES + 10);
		Images.Init (hexenLumpNames, NUM_HEXENSB_IMAGES);
		ClassImages[0].Init (classLumpNames[0], NUM_HEXENCLASSSB_IMAGES);
		ClassImages[1].Init (classLumpNames[1], NUM_HEXENCLASSSB_IMAGES);
		ClassImages[2].Init (classLumpNames[2], NUM_HEXENCLASSSB_IMAGES);

		oldarti = 0;
		oldartiCount = 0;
		oldfrags = -9999;
		oldmana1 = -1;
		oldmana2 = -1;
		oldarmor = -1;
		oldhealth = -1;
		oldlife = -1;
		oldpieces = -1;
		oldweapon = -1;
		oldkeys = -1;

		HealthMarker = 0;
		ArtifactFlash = 0;

		ArtiRefresh = 0;
		FragHealthRefresh = 0;
		KeysRefresh = 0;
		ArmorRefresh = 0;
		HealthRefresh = 0;
		Mana1Refresh = 0;
		Mana2Refresh = 0;
	}

	~FHexenStatusBar ()
	{
	}

	void Tick ()
	{
		int curHealth;

		FBaseStatusBar::Tick ();
		if (CPlayer->mo == NULL)
		{
			curHealth = 0;
		}
		else
		{
			curHealth = CPlayer->mo->health;
		}
		if (curHealth < 0)
		{
			curHealth = 0;
		}
		if (curHealth < HealthMarker)
		{
			HealthMarker -= clamp ((HealthMarker - curHealth) >> 2, 1, 6);
		}
		else if (curHealth > HealthMarker)
		{
			HealthMarker += clamp ((curHealth - HealthMarker) >> 2, 1, 6);
		}

		if (ArtifactFlash > 0)
		{
			ArtifactFlash--;
		}
	}

	void Draw (EHudState state)
	{
		FBaseStatusBar::Draw (state);

		if (state == HUD_Fullscreen)
		{
			DrawFullScreenStuff ();
			SB_state = screen->GetPageCount ();
		}
		else if (state == HUD_StatusBar)
		{
			if (SB_state > 0)
			{
				DrawImage (Images[imgH2BAR], 0, -27);
				oldhealth = -1;
			}
			DrawCommonBar ();
			if (CPlayer->inventorytics == 0)
			{
				if (SB_state < 0)
				{
					SB_state = screen->GetPageCount ();
				}
				if (SB_state != 0)
				{
					// Main interface
					SB_state--;
					DrawImage (Images[!automapactive ? imgSTATBAR : imgKEYBAR], 38, 0);
					oldarti = 0;
					oldmana1 = -1;
					oldmana2 = -1;
					oldarmor = -1;
					oldpieces = -1;
					oldfrags = -9999; //can't use -1, 'cuz of negative frags
					oldlife = -1;
					oldweapon = -1;
					oldkeys = -1;
					//oldhealth = -1;
				}
				if (!automapactive)
				{
					DrawMainBar ();
				}
				else
				{
					DrawKeyBar ();
				}
			}
			else
			{
				if (SB_state > -1)
				{
					SB_state = -screen->GetPageCount () - 1;
				}
				if (SB_state < -1)
				{
					SB_state++;
				}
				DrawInventoryBar ();
			}
		}

		DrawAnimatedIcons ();
	}

	void AttachToPlayer (player_s *player)
	{
		FBaseStatusBar::AttachToPlayer (player);
		if (player->mo != NULL)
		{
			if (player->mo->IsKindOf (RUNTIME_CLASS(AMagePlayer)))
			{
				FourthWeaponShift = 6;
				FourthWeaponClass = 2;
				LifeBarClass = 2;
			}
			else if (player->mo->IsKindOf (RUNTIME_CLASS(AClericPlayer)))
			{
				FourthWeaponShift = 3;
				FourthWeaponClass = 1;
				LifeBarClass = 1;
			}
			else
			{
				FourthWeaponShift = 0;
				FourthWeaponClass = 0;
				LifeBarClass = 0;
			}
		}
	}

	void SetInteger (int pname, int param)
	{
		if (pname == 0)
		{
			FourthWeaponShift = param;
			FourthWeaponClass = param / 3;
		}
	}

private:
	void FHexenStatusBar::DrawAnimatedIcons ()
	{
		int frame;
		static bool hitCenterFrame;
		int picnum;

		// Flight icons
		if (CPlayer->powers[pw_flight])
		{
			if (CPlayer->powers[pw_flight] > BLINKTHRESHOLD
				|| !(CPlayer->powers[pw_flight] & 16))
			{
				picnum = TexMan.GetTexture ("SPFLY0", FTexture::TEX_MiscPatch);
				frame = (level.time/3) & 15;

				if (CPlayer->mo->flags2 & MF2_FLY)
				{
					if (hitCenterFrame && (frame != 15 && frame != 0))
					{
						DrawOuterTexture (TexMan[picnum+15], 20, 19);
					}
					else
					{
						DrawOuterTexture (TexMan[picnum+frame], 20, 19);
						hitCenterFrame = false;
					}
				}
				else
				{
					if (!hitCenterFrame && (frame != 15 && frame != 0))
					{
						DrawOuterTexture (TexMan[picnum+frame], 20, 19);
						hitCenterFrame = false;
					}
					else
					{
						DrawOuterTexture (TexMan[picnum+15], 20, 19);
						hitCenterFrame = true;
					}
				}
			}
			BorderTopRefresh = screen->GetPageCount ();
		}

		// Speed Boots
		if (CPlayer->powers[pw_speed])
		{
			if (CPlayer->powers[pw_speed] > BLINKTHRESHOLD
				|| !(CPlayer->powers[pw_speed] & 16))
			{
				picnum = TexMan.GetTexture ("SPBOOT0", FTexture::TEX_MiscPatch);
				frame = (level.time/3)&15;
				DrawOuterTexture (TexMan[picnum+frame], 60, 19);
			}
			BorderTopRefresh = screen->GetPageCount ();
		}

		// Defensive power
		if (CPlayer->powers[pw_invulnerability])
		{
			if (CPlayer->powers[pw_invulnerability] > BLINKTHRESHOLD
				|| !(CPlayer->powers[pw_invulnerability] & 16))
			{
				frame = (level.time/3)&15;
				picnum = TexMan.GetTexture ("SPSHLD0", FTexture::TEX_MiscPatch);
				DrawOuterTexture (TexMan[picnum+frame], -60, 19);
			}
			BorderTopRefresh = screen->GetPageCount ();
		}

		// Minotaur Active
		if (CPlayer->powers[pw_minotaur])
		{
			if (CPlayer->powers[pw_minotaur] > BLINKTHRESHOLD
				|| !(CPlayer->powers[pw_minotaur]&16))
			{
				frame = (level.time/3)&15;
				picnum = TexMan.GetTexture ("SPMINO0", FTexture::TEX_MiscPatch);
				DrawOuterTexture (TexMan[picnum+frame], -20, 19);
			}
			BorderTopRefresh = screen->GetPageCount ();
		}
	}

//---------------------------------------------------------------------------
//
// PROC DrawCommonBar
//
//---------------------------------------------------------------------------

	void DrawCommonBar ()
	{
		int healthPos;

		DrawImage (Images[imgH2TOP], 0, -27);

		if (oldhealth != HealthMarker)
		{
			oldhealth = HealthMarker;
			HealthRefresh = screen->GetPageCount ();
		}
		if (HealthRefresh)
		{
			HealthRefresh--;
			healthPos = clamp (HealthMarker, 0, 100);
			DrawImage (ClassImages[LifeBarClass][imgCHAIN], 35+((healthPos*196/100)%9), 31);
			DrawImage (ClassImages[LifeBarClass][imgLIFEGEM], 7+(healthPos*11/5), 31, multiplayer ?
				translationtables[TRANSLATION_PlayersExtra] + (CPlayer-players)*256 : NULL);
			DrawImage (Images[imgLFEDGE], 0, 31);
			DrawImage (Images[imgRTEDGE], 277, 31);
		}
	}

//---------------------------------------------------------------------------
//
// PROC DrawMainBar
//
//---------------------------------------------------------------------------

	void DrawMainBar ()
	{
		int temp;

		// Ready artifact
		if (ArtifactFlash)
		{
			DrawImage (Images[imgARTICLEAR], 144, -1);
			DrawImage (Images[imgUSEARTIA + ArtifactFlash], 148, 2);
			oldarti = -1; // so that the correct artifact fills in after the flash
		}
		else if (oldarti != CPlayer->readyArtifact
			|| oldartiCount != CPlayer->inventory[oldarti])
		{
			oldarti = CPlayer->readyArtifact;
			oldartiCount = CPlayer->inventory[oldarti];
			ArtiRefresh = screen->GetPageCount ();
		}
		if (ArtiRefresh)
		{
			ArtiRefresh--;
			DrawImage (Images[imgARTICLEAR], 144, -1);
			if (CPlayer->inventory[oldarti] > 0)
			{
				DrawImage (ArtiImages[oldarti], 143, 2);
				if (oldartiCount != 1)
				{
					DrSmallNumber (oldartiCount, 162, 23);
				}
			}
		}

		// Frags
		if (deathmatch)
		{
			temp = CPlayer->fragcount;
			if (temp != oldfrags)
			{
				oldfrags = temp;
				FragHealthRefresh = screen->GetPageCount ();
			}
			if (FragHealthRefresh)
			{
				FragHealthRefresh--;
				DrawImage (Images[imgKILLS], 38, 1);
				DrINumber (temp, 40, 15);
			}
		}
		else
		{
			temp = clamp (HealthMarker, 0, 100);
			if (oldlife != temp)
			{
				oldlife = temp;
				FragHealthRefresh = screen->GetPageCount ();
			}
			if (FragHealthRefresh)
			{
				FragHealthRefresh--;
				DrawImage (Images[imgARMCLEAR], 41, 16);
				DrINumber (temp, 40, 14, temp >= 25 ? imgINumbers : NUM_BASESB_IMAGES);
			}
		}

		// Mana
		int manaPatch1, manaPatch2;
		int manaVialPatch1, manaVialPatch2;

		manaPatch1 = 0;
		manaPatch2 = 0;
		manaVialPatch1 = 0;
		manaVialPatch2 = 0;

		temp = CPlayer->ammo[MANA_1];
		if (oldmana1 != temp)
		{
			oldmana1 = temp;
			Mana1Refresh = screen->GetPageCount ();
		}
		if (Mana1Refresh)
		{
			Mana1Refresh--;
			DrawImage (Images[imgMANACLEAR], 77, 16);
			DrSmallNumber (temp, 79, 19);
			if (temp == 0)
			{ // Draw Dim Mana icon
				manaPatch1 = imgMANADIM1;
			}
			else if (oldmana1 == 0)
			{
				manaPatch1 = imgMANABRIGHT1;
			}		
			manaVialPatch1 = 1; // force a vial update
		}

		temp = CPlayer->ammo[MANA_2];
		if (oldmana2 != temp)
		{
			oldmana2 = temp;
			Mana2Refresh = screen->GetPageCount ();
		}
		if (Mana2Refresh)
		{
			Mana2Refresh--;
			DrawImage (Images[imgMANACLEAR], 109, 16);
			DrSmallNumber (temp, 111, 19);
			if (temp == 0)
			{ // Draw Dim Mana icon
				manaPatch2 = imgMANADIM2;
			}		
			else if (oldmana2 == 0)
			{
				manaPatch2 = imgMANABRIGHT2;
			}		
			manaVialPatch1 = 1; // force a vial update
		}

		if (oldweapon != CPlayer->readyweapon || manaPatch1 || manaPatch2
			|| manaVialPatch1)
		{ // Update mana graphics based upon mana count/weapon type
			FWeaponInfo **wpinfo;
			int ammo;

			if (CPlayer->powers[pw_weaponlevel2] && !deathmatch)
			{
				wpinfo = wpnlev2info;
			}
			else
			{
				wpinfo = wpnlev1info;
			}
			ammo = wpinfo[CPlayer->readyweapon]->ammo;

			// Pick graphics for mana 1 vial
			if (ammo == MANA_1 || ammo == MANA_BOTH)
			{
				if (!manaPatch1)
				{
					manaPatch1 = imgMANABRIGHT1;
				}
				manaVialPatch1 = imgMANAVIAL1;
			}
			else
			{
				manaPatch1 = imgMANADIM1;
				manaVialPatch1 = imgMANAVIALDIM1;
			}

			// Pick graphics for mana 2 vial
			if (ammo == MANA_2 || ammo == MANA_BOTH)
			{
				if (!manaPatch2)
				{
					manaPatch2 = imgMANABRIGHT2;
				}
				manaVialPatch2 = imgMANAVIAL2;
			}
			else
			{
				manaPatch2 = imgMANADIM2;
				manaVialPatch2 = imgMANAVIALDIM2;
			}

			DrawImage (Images[manaPatch1], 77, 2);
			DrawImage (Images[manaPatch2], 110, 2);

			ManaVial1Pic.SetVial (Images[manaVialPatch1], 22*CPlayer->ammo[MANA_1]/MAX_MANA);
			ManaVial2Pic.SetVial (Images[manaVialPatch2], 22*CPlayer->ammo[MANA_2]/MAX_MANA);
			DrawImage (&ManaVial1Pic, 94, 2);
			DrawImage (&ManaVial2Pic, 102, 2);
			oldweapon = CPlayer->readyweapon;
		}

		// Armor
		temp = CPlayer->mo->GetAutoArmorSave()
			+CPlayer->armorpoints[ARMOR_ARMOR]+CPlayer->armorpoints[ARMOR_SHIELD]
			+CPlayer->armorpoints[ARMOR_HELMET]+CPlayer->armorpoints[ARMOR_AMULET];
		if (oldarmor != temp)
		{
			oldarmor = temp;
			ArmorRefresh = screen->GetPageCount ();
		}
		if (ArmorRefresh)
		{
			ArmorRefresh--;
			DrawImage (Images[imgARMCLEAR], 255, 16);
			DrINumber (temp / (5*FRACUNIT), 250, 14);
		}

		// Weapon Pieces
		if (oldpieces != CPlayer->pieces)
		{
			DrawWeaponPieces();
			oldpieces = CPlayer->pieces;
		}
	}

//---------------------------------------------------------------------------
//
// PROC DrawInventoryBar
//
//---------------------------------------------------------------------------

	void DrawInventoryBar ()
	{
		int i;
		int x;
		bool left, right;

		DrawImage (Images[imgINVBAR], 38, 0);
		FindInventoryPos (x, left, right);
		if (x > 0)
		{
			for (i = 0; i < 7 && x < NUMINVENTORYSLOTS; x++)
			{
				if (CPlayer->inventory[x])
				{
					DrawImage (ArtiImages[x], 50+i*31, 1);
					if (CPlayer->inventory[x] != 1)
					{
						DrSmallNumber (CPlayer->inventory[x], 68+i*31, 23);
					}
					if (x == CPlayer->readyArtifact)
					{
						DrawImage (Images[imgSELECTBOX], 51+i*31, 1);
					}
					i++;
				}
			}
			if (left)
			{
				DrawImage (Images[!(level.time & 4) ?
					imgINVLFGEM1 : imgINVLFGEM2], 42, 1);
			}
			if (right)
			{
				DrawImage (Images[!(level.time & 4) ?
					imgINVRTGEM1 : imgINVRTGEM2], 269, 1);
			}
		}
	}

//==========================================================================
//
// DrawKeyBar
//
//==========================================================================

	void DrawKeyBar ()
	{
		int playerkeys;
		int i;
		int xPosition;
		int temp;

		playerkeys = 0;
		
		for (i = 0; i < NUMKEYS; ++i)
		{
			playerkeys <<= 1;
			if (CPlayer->keys[i])
			{
				playerkeys |= 1;
			}
		}

		if (oldkeys != playerkeys)
		{
			oldkeys = playerkeys;
			KeysRefresh = screen->GetPageCount ();
		}
		if (KeysRefresh)
		{
			KeysRefresh--;
			xPosition = 46;
			for (i = 0; i < NUMKEYS && xPosition <= 126; i++)
			{
				if (CPlayer->keys[i])
				{
					DrawImage (Images[imgKEYSLOT1+i], xPosition, 2);
					xPosition += 20;
				}
			}
		}
		temp = CPlayer->mo->GetAutoArmorSave()
			+CPlayer->armorpoints[ARMOR_ARMOR]+CPlayer->armorpoints[ARMOR_SHIELD]
			+CPlayer->armorpoints[ARMOR_HELMET]+CPlayer->armorpoints[ARMOR_AMULET];
		if (oldarmor != temp)
		{
			for (i = 0; i < NUMARMOR; i++)
			{
				if (CPlayer->armorpoints[i] > 0)
				{
					DrawFadedImage (Images[imgARMSLOT1+i], 150+31*i, 2,
						MIN<fixed_t> (OPAQUE, Scale (CPlayer->armorpoints[i], OPAQUE,
											CPlayer->mo->GetArmorIncrement (i))));
				}
			}
			oldarmor = temp;
		}
	}

//==========================================================================
//
// DrawWeaponPieces
//
//==========================================================================

	void DrawWeaponPieces ()
	{
		static int PieceX[3][3] = 
		{
			{ 190, 225, 234 },
			{ 190, 212, 225 },
			{ 190, 205, 224 },
		};
		int pieces = (CPlayer->pieces >> FourthWeaponShift) & 7;

		if (pieces == 7)
		{
			DrawImage (ClassImages[FourthWeaponClass][imgWEAPONFULL], 190, 0);
			return;
		}
		DrawImage (ClassImages[FourthWeaponClass][imgWEAPONSLOT], 190, 0);
		if (pieces & WPIECE1)
		{
			DrawImage (ClassImages[FourthWeaponClass][imgPIECE1], PieceX[FourthWeaponClass][0], 0);
		}
		if (pieces & WPIECE2)
		{
			DrawImage (ClassImages[FourthWeaponClass][imgPIECE2], PieceX[FourthWeaponClass][1], 0);
		}
		if (pieces & WPIECE3)
		{
			DrawImage (ClassImages[FourthWeaponClass][imgPIECE3], PieceX[FourthWeaponClass][2], 0);
		}
	}

//---------------------------------------------------------------------------
//
// PROC DrawFullScreenStuff
//
//---------------------------------------------------------------------------

	void DrawFullScreenStuff ()
	{
		int i;
		int x;

		DrBNumberOuter (MAX (0, CPlayer->mo->health), 5, -20);
		if (deathmatch)
		{
			DrINumberOuter (CPlayer->fragcount, 45, -15);
		}
		if (CPlayer->inventorytics == 0)
		{
			if (ArtifactFlash)
			{
				DrawOuterFadedImage (Images[imgARTIBOX], -80, -30, HX_SHADOW);
				DrawOuterImage (Images[imgUSEARTIA + ArtifactFlash], -76, -26);
			}
			else if (CPlayer->inventory[CPlayer->readyArtifact] > 0)
			{
				DrawOuterFadedImage (Images[imgARTIBOX], -80, -30, HX_SHADOW);
				DrawOuterImage (ArtiImages[CPlayer->readyArtifact], -82, -31);
				if (CPlayer->inventory[CPlayer->readyArtifact] != 1)
				{
					DrSmallNumberOuter (CPlayer->inventory[CPlayer->readyArtifact],
						-64, -8);
				}
			}
		}
		else
		{
			SetHorizCentering (true);
			bool left, right;

			FindInventoryPos (x, left, right);
			for (i = 0; i < 7 && x < NUMINVENTORYSLOTS; x++)
			{
				if (CPlayer->inventory[x])
				{
					DrawOuterFadedImage (Images[imgARTIBOX], -106+i*31, -32, HX_SHADOW);
					DrawOuterImage (ArtiImages[x], -108+i*31, -33);
					if (CPlayer->inventory[x] != 1)
					{
						DrSmallNumberOuter (CPlayer->inventory[x], -90+i*31, -12);
					}
					if (x == CPlayer->readyArtifact)
					{
						DrawOuterImage (Images[imgSELECTBOX], -107+i*31, -33);
					}
					i++;
				}
			}
			for (; i < 7; i++)
			{
				DrawOuterFadedImage (Images[imgARTIBOX], -106+i*31, -32, HX_SHADOW);
			}
			if (left)
			{
				DrawOuterImage (Images[!(level.time & 4) ?
					imgINVLFGEM1 : imgINVLFGEM2], -118, -33);
			}
			if (right)
			{
				DrawOuterImage (Images[!(level.time & 4) ?
					imgINVRTGEM1 : imgINVRTGEM2], 113, -33);
			}
			SetHorizCentering (false);
		}

		int manaImage;
		FWeaponInfo **wpinfo;
		int ammo;

		if (CPlayer->powers[pw_weaponlevel2] && !deathmatch)
		{
			wpinfo = wpnlev2info;
		}
		else
		{
			wpinfo = wpnlev1info;
		}
		ammo = wpinfo[CPlayer->readyweapon]->ammo;

		i = CPlayer->ammo[MANA_1];
		manaImage = ((ammo == MANA_1 || ammo == MANA_BOTH) && i > 0)
			? imgMANABRIGHT1 : imgMANADIM1;
		DrawOuterImage (Images[manaImage], -17, -30);
		DrINumberOuter (i, -47, -30);

		i = CPlayer->ammo[MANA_2];
		manaImage = ((ammo == MANA_2 || ammo == MANA_BOTH) && i > 0)
			? imgMANABRIGHT2 : imgMANADIM2;
		DrawOuterImage (Images[manaImage], -17, -15);
		DrINumberOuter (i, -47, -15);
	}

//---------------------------------------------------------------------------
//
// PROC FlashArtifact
//
//---------------------------------------------------------------------------

	void FlashArtifact (int arti)
	{
		ArtifactFlash = 4;
	}

	static const char patcharti[][10];
	static const char ammopic[][10];

	int oldarti;
	int oldartiCount;
	int oldfrags;
	int oldmana1;
	int oldmana2;
	int oldarmor;
	int oldhealth;
	int oldlife;
	int oldpieces;
	int oldweapon;
	int oldkeys;

	enum
	{
		imgH2BAR,
		imgH2TOP,
		imgINVBAR,
		imgLFEDGE,
		imgRTEDGE,
		imgSTATBAR,
		imgKEYBAR,
		imgSELECTBOX,
		imgARTICLEAR,
		imgARMCLEAR,
		imgMANACLEAR,
		imgMANAVIAL1,
		imgMANAVIAL2,
		imgMANAVIALDIM1,
		imgMANAVIALDIM2,
		imgMANADIM1,
		imgMANADIM2,
		imgMANABRIGHT1,
		imgMANABRIGHT2,
		imgINVLFGEM1,
		imgINVLFGEM2,
		imgINVRTGEM1,
		imgINVRTGEM2,
		imgKILLS,
		imgUSEARTIA,
		imgUSEARTIB,
		imgUSEARTIC,
		imgUSEARTID,
		imgUSEARTIE,
		imgKEYSLOT1,
		imgKEYSLOT2,
		imgKEYSLOT3,
		imgKEYSLOT4,
		imgKEYSLOT5,
		imgKEYSLOT6,
		imgKEYSLOT7,
		imgKEYSLOT8,
		imgKEYSLOT9,
		imgKEYSLOTA,
		imgKEYSLOTB,
		imgARMSLOT1,
		imgARMSLOT2,
		imgARMSLOT3,
		imgARMSLOT4,
		imgARTIBOX,

		NUM_HEXENSB_IMAGES
	};

	enum
	{
		imgWEAPONSLOT,
		imgWEAPONFULL,
		imgPIECE1,
		imgPIECE2,
		imgPIECE3,
		imgCHAIN,
		imgLIFEGEM,

		NUM_HEXENCLASSSB_IMAGES
	};

	FImageCollection Images;
	FImageCollection ClassImages[3];

	int HealthMarker;
	char ArtifactFlash;

	char FourthWeaponShift;
	char FourthWeaponClass;
	char LifeBarClass;

	char ArtiRefresh;
	char FragHealthRefresh;
	char KeysRefresh;
	char ArmorRefresh;
	char HealthRefresh;
	char Mana1Refresh;
	char Mana2Refresh;

	FManaBar ManaVial1Pic;
	FManaBar ManaVial2Pic;
};

FBaseStatusBar *CreateHexenStatusBar ()
{
	return new FHexenStatusBar;
}
