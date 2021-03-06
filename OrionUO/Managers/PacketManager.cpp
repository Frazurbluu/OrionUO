/***********************************************************************************
**
** PacketManager.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "PacketManager.h"
#include "ConnectionManager.h"
#include "../OrionUO.h"
#include "../OrionWindow.h"
#include "../CharacterList.h"
#include "../ServerList.h"
#include "../CityList.h"
#include "../Screen stages/ConnectionScreen.h"
#include "../Screen stages/MainScreen.h"
#include "../Screen stages/CharacterListScreen.h"
#include "../Screen stages/ServerScreen.h"
#include "../Network/Packets.h"
#include "../Game objects/GameWorld.h"
#include "../Game objects/GamePlayer.h"
#include "../Walker/Walker.h"
#include "../Game objects/ObjectOnCursor.h"
#include "../ClickObject.h"
#include "../Weather.h"
#include "../Managers/MapManager.h"
#include "../Target.h"
#include "../Managers/ConfigManager.h"
#include "../Managers/SoundManager.h"
#include "../Container.h"
#include "../Party.h"
#include "../Managers/ClilocManager.h"
#include "../Managers/FontsManager.h"
#include "../Managers/GumpManager.h"
#include "../Gumps/GumpPaperdoll.h"
#include "../Gumps/GumpContainer.h"
#include "../Gumps/GumpPopupMenu.h"
#include "../Managers/EffectManager.h"
#include "../Game objects/GameEffectMoving.h"
#include "../Game objects/GameEffectDrag.h"
#include "../QuestArrow.h"
#include "../MultiMap.h"
#include "../Gumps/GumpBuff.h"
#include "../Gumps/GumpSecureTrading.h"
#include "../Gumps/GumpTextEntryDialog.h"
#include "../Gumps/GumpGrayMenu.h"
#include "../Gumps/GumpMenu.h"
#include "../Gumps/GumpDye.h"
#include "../Gumps/GumpGeneric.h"
#include "../Gumps/GumpMap.h"
#include "../Gumps/GumpTip.h"
#include "../Gumps/GumpProfile.h"
#include "../Gumps/GumpBulletinBoard.h"
#include "../Gumps/GumpBulletinBoardItem.h"
#include "../Gumps/GumpBook.h"
#include "../Gumps/GumpShop.h"
#include "../Gumps/GumpSkills.h"
#include "../zlib.h"

#pragma comment(lib, "zdll.lib")
//----------------------------------------------------------------------------------
CPacketManager g_PacketManager;
//----------------------------------------------------------------------------------
//����� ������� �� ��� �������
#define UMSG(size) { "?", size, DIR_BOTH, 0 }
// A message type sent to the server
#define SMSG(name, size) { name, size, DIR_SEND, 0 }
// A message type received from the server
#define RMSG(name, size) { name, size, DIR_RECV, 0 }
// A message type transmitted in both directions
#define BMSG(name, size) { name, size, DIR_BOTH, 0 }
// Message types that have handler methods
#define RMSGH(name, size, rmethod) \
{ name, size, DIR_RECV, &CPacketManager::Handle ##rmethod }
#define BMSGH(name, size, rmethod) \
{ name, size, DIR_BOTH, &CPacketManager::Handle ##rmethod }
//----------------------------------------------------------------------------------
CPacketInfo CPacketManager::m_Packets[0x100] =
{
	/*0x00*/ SMSG("Create Character", 0x68),
	/*0x01*/ SMSG("Disconnect", 0x05),
	/*0x02*/ SMSG("Walk Request", 0x07),
	/*0x03*/ BMSGH("Client Talk", PACKET_VARIABLE_SIZE, ClientTalk),
	/*0x04*/ SMSG("Request God mode (God client)", 0x02),
	/*0x05*/ SMSG("Attack", 0x05),
	/*0x06*/ SMSG("Double Click", 0x05),
	/*0x07*/ SMSG("Pick Up Item", 0x07),
	/*0x08*/ SMSG("Drop Item", 0x0e),
	/*0x09*/ SMSG("Single Click", 0x05),
	/*0x0A*/ SMSG("Edit (God client)", 0x0b),
	/*0x0B*/ RMSGH("Damage Visualization", 0x07, Damage),
	/*0x0C*/ BMSG("Edit tiledata (God client)", PACKET_VARIABLE_SIZE),
	/*0x0D*/ UMSG(0x03),
	/*0x0E*/ UMSG(0x01),
	/*0x0F*/ UMSG(0x3d),
	/*0x10*/ UMSG(0xd7),
	/*0x11*/ RMSGH("Character Status", PACKET_VARIABLE_SIZE, CharacterStatus),
	/*0x12*/ SMSG("Perform Action", PACKET_VARIABLE_SIZE),
	/*0x13*/ SMSG("Client Equip Item", 0x0a),
	/*0x14*/ SMSG("Send elevation (God client)", 0x06),
	/*0x15*/ BMSG("Follow", 0x09),
	/*0x16*/ UMSG(0x01),
	/*0x17*/ RMSG("Health status bar update (KR)", PACKET_VARIABLE_SIZE),
	/*0x18*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x19*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x1A*/ RMSGH("Update Item", PACKET_VARIABLE_SIZE, UpdateItem),
	/*0x1B*/ RMSGH("Enter World", 0x25, EnterWorld),
	/*0x1C*/ RMSGH("Server Talk", PACKET_VARIABLE_SIZE, Talk),
	/*0x1D*/ RMSGH("Delete Object", 0x05, DeleteObject),
	/*0x1E*/ UMSG(0x04),
	/*0x1F*/ UMSG(0x08),
	/*0x20*/ RMSGH("Update Player", 0x13, UpdatePlayer),
	/*0x21*/ RMSGH("Deny Walk", 0x08, DenyWalk),
	/*0x22*/ BMSGH("Confirm Walk", 0x03, ConfirmWalk),
	/*0x23*/ RMSGH("Drag Animation", 0x1a, DragAnimation),
	/*0x24*/ RMSGH("Open Container", 0x07, OpenContainer),
	/*0x25*/ RMSGH("Update Contained Item", 0x14, UpdateContainedItem),
	/*0x26*/ UMSG(0x05),
	/*0x27*/ RMSGH("Deny Move Item", 0x02, DenyMoveItem),
	/*0x28*/ UMSG(0x05),
	/*0x29*/ RMSG("Drop Item Approved", 0x01),
	/*0x2A*/ UMSG(0x05),
	/*0x2B*/ UMSG(0x02),
	/*0x2C*/ BMSGH("Death Screen", 0x02, DeathScreen),
	/*0x2D*/ RMSG("Mob Attributes", 0x11),
	/*0x2E*/ RMSGH("Server Equip Item", 0x0f, EquipItem),
	/*0x2F*/ RMSG("Combat Notification", 0x0a),
	/*0x30*/ RMSG("Attack ok", 0x05),
	/*0x31*/ RMSG("Attack end", 0x01),
	/*0x32*/ UMSG(0x02),
	/*0x33*/ RMSGH("Pause Control", 0x02, PauseControl),
	/*0x34*/ SMSG("Status Request", 0x0a),
	/*0x35*/ UMSG(0x28d),
	/*0x36*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x37*/ UMSG(0x08),
	/*0x38*/ BMSG("Pathfinding in Client", 0x07),
	/*0x39*/ RMSG("Remove (Group)", 0x09),
	/*0x3A*/ BMSGH("Update Skills", PACKET_VARIABLE_SIZE, UpdateSkills),
	/*0x3B*/ BMSGH("Vendor Buy Reply", PACKET_VARIABLE_SIZE, BuyReply),
	/*0x3C*/ RMSGH("Update Contained Items", PACKET_VARIABLE_SIZE, UpdateContainedItems),
	/*0x3D*/ UMSG(0x02),
	/*0x3E*/ UMSG(0x25),
	/*0x3F*/ RMSG("Update Statics (God Client)", PACKET_VARIABLE_SIZE),
	/*0x40*/ UMSG(0xc9),
	/*0x41*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x42*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x43*/ UMSG(0x229),
	/*0x44*/ UMSG(0x2c9),
	/*0x45*/ BMSG("Version OK", 0x05),
	/*0x46*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x47*/ UMSG(0x0b),
	/*0x48*/ UMSG(0x49),
	/*0x49*/ UMSG(0x5d),
	/*0x4A*/ UMSG(0x05),
	/*0x4B*/ UMSG(0x09),
	/*0x4C*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x4D*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x4E*/ RMSGH("Personal Light Level", 0x06, PersonalLightLevel),
	/*0x4F*/ RMSGH("Global Light Level", 0x02, LightLevel),
	/*0x50*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x51*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x52*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x53*/ RMSGH("Error Code", 0x02, ErrorCode),
	/*0x54*/ RMSGH("Sound Effect", 0x0c, PlaySoundEffect),
	/*0x55*/ RMSGH("Login Complete", 0x01, LoginComplete),
	/*0x56*/ BMSGH("Map Data", 0x0b, MapData),
	/*0x57*/ BMSG("Update Regions", 0x6e),
	/*0x58*/ UMSG(0x6a),
	/*0x59*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x5A*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x5B*/ RMSGH("Set Time", 0x04, SetTime),
	/*0x5C*/ BMSG("Restart Version", 0x02),
	/*0x5D*/ SMSG("Select Character", 0x49),
	/*0x5E*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x5F*/ UMSG(0x31),
	/*0x60*/ UMSG(0x05),
	/*0x61*/ UMSG(0x09),
	/*0x62*/ UMSG(0x0f),
	/*0x63*/ UMSG(0x0d),
	/*0x64*/ UMSG(0x01),
	/*0x65*/ RMSGH("Set Weather", 0x04, SetWeather),
	/*0x66*/ BMSGH("Book Page Data", PACKET_VARIABLE_SIZE, BookData),
	/*0x67*/ UMSG(0x15),
	/*0x68*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x69*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x6A*/ UMSG(0x03),
	/*0x6B*/ UMSG(0x09),
	/*0x6C*/ BMSGH("Target Data", 0x13, Target),
	/*0x6D*/ RMSGH("Play Music", 0x03, PlayMusic),
	/*0x6E*/ RMSGH("Character Animation", 0x0e, CharacterAnimation),
	/*0x6F*/ BMSGH("Secure Trading", PACKET_VARIABLE_SIZE, SecureTrading),
	/*0x70*/ RMSGH("Graphic Effect", 0x1c, GraphicEffect),
	/*0x71*/ BMSGH("Bulletin Board Data", PACKET_VARIABLE_SIZE, BulletinBoardData),
	/*0x72*/ BMSGH("War Mode", 0x05, Warmode),
	/*0x73*/ BMSGH("Ping", 0x02, Ping),
	/*0x74*/ RMSGH("Vendor Buy List", PACKET_VARIABLE_SIZE, BuyList),
	/*0x75*/ SMSG("Rename Character", 0x23),
	/*0x76*/ RMSG("New Subserver", 0x10),
	/*0x77*/ RMSGH("Update Character", 0x11, UpdateCharacter),
	/*0x78*/ RMSGH("Update Object", PACKET_VARIABLE_SIZE, UpdateObject),
	/*0x79*/ UMSG(0x09),
	/*0x7A*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x7B*/ UMSG(0x02),
	/*0x7C*/ RMSGH("Open Menu Gump", PACKET_VARIABLE_SIZE, OpenMenuGump),
	/*0x7D*/ SMSG("Menu Choice", 0x0d),
	/*0x7E*/ UMSG(0x02),
	/*0x7F*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x80*/ SMSG("First Login", 0x3e),
	/*0x81*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x82*/ RMSGH("Login Error", 0x02, LoginError),
	/*0x83*/ SMSG("Delete Character", 0x27),
	/*0x84*/ UMSG(0x45),
	/*0x85*/ RMSGH("Character List Notification", 0x02, CharacterListNotification),
	/*0x86*/ RMSGH("Resend Character List", PACKET_VARIABLE_SIZE, ResendCharacterList),
	/*0x87*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x88*/ RMSGH("Open Paperdoll", 0x42, OpenPaperdoll),
	/*0x89*/ RMSGH("Corpse Equipment", PACKET_VARIABLE_SIZE, CorpseEquipment),
	/*0x8A*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x8B*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x8C*/ RMSGH("Relay Server", 0x0b, RelayServer),
	/*0x8D*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x8E*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x8F*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x90*/ RMSGH("Display Map", 0x13, DisplayMap),
	/*0x91*/ SMSG("Second Login", 0x41),
	/*0x92*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x93*/ RMSGH("Open Book", 0x63, OpenBook),
	/*0x94*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x95*/ BMSGH("Dye Data", 0x09, DyeData),
	/*0x96*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0x97*/ RMSG("Move Player", 0x02),
	/*0x98*/ BMSG("All Names (3D Client Only)", PACKET_VARIABLE_SIZE),
	/*0x99*/ BMSGH("Multi Placement", 0x1a, MultiPlacement),
	/*0x9A*/ BMSGH("ASCII Prompt", PACKET_VARIABLE_SIZE, ASCIIPrompt),
	/*0x9B*/ SMSG("Help Request", 0x102),
	/*0x9C*/ UMSG(0x135),
	/*0x9D*/ UMSG(0x33),
	/*0x9E*/ RMSGH("Vendor Sell List", PACKET_VARIABLE_SIZE, SellList),
	/*0x9F*/ SMSG("Vendor Sell Reply", PACKET_VARIABLE_SIZE),
	/*0xA0*/ SMSG("Select Server", 0x03),
	/*0xA1*/ RMSGH("Update Hitpoints", 0x09, UpdateHitpoints),
	/*0xA2*/ RMSGH("Update Mana", 0x09, UpdateMana),
	/*0xA3*/ RMSGH("Update Stamina", 0x09, UpdateStamina),
	/*0xA4*/ SMSG("System Information", 0x95),
	/*0xA5*/ RMSGH("Open URL", PACKET_VARIABLE_SIZE, OpenUrl),
	/*0xA6*/ RMSGH("Tip Window", PACKET_VARIABLE_SIZE, TipWindow),
	/*0xA7*/ SMSG("Request Tip", 0x04),
	/*0xA8*/ RMSGH("Server List", PACKET_VARIABLE_SIZE, ServerList),
	/*0xA9*/ RMSGH("Character List", PACKET_VARIABLE_SIZE, CharacterList),
	/*0xAA*/ RMSGH("Attack Reply", 0x05, AttackCharacter),
	/*0xAB*/ RMSGH("Text Entry Dialog", PACKET_VARIABLE_SIZE, TextEntryDialog),
	/*0xAC*/ SMSG("Text Entry Dialog Reply", PACKET_VARIABLE_SIZE),
	/*0xAD*/ SMSG("Unicode Client Talk", PACKET_VARIABLE_SIZE),
	/*0xAE*/ RMSGH("Unicode Server Talk", PACKET_VARIABLE_SIZE, UnicodeTalk),
	/*0xAF*/ RMSGH("Display Death", 0x0d, DisplayDeath),
	/*0xB0*/ RMSGH("Open Dialog Gump", PACKET_VARIABLE_SIZE, OpenGump),
	/*0xB1*/ SMSG("Dialog Choice", PACKET_VARIABLE_SIZE),
	/*0xB2*/ BMSG("Chat Data", PACKET_VARIABLE_SIZE),
	/*0xB3*/ RMSG("Chat Text ?", PACKET_VARIABLE_SIZE),
	/*0xB4*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xB5*/ BMSGH("Open Chat Window", 0x40, OpenChat),
	/*0xB6*/ SMSG("Popup Help Request", 0x09),
	/*0xB7*/ RMSG("Popup Help Data", PACKET_VARIABLE_SIZE),
	/*0xB8*/ BMSGH("Character Profile", PACKET_VARIABLE_SIZE, CharacterProfile),
	/*0xB9*/ RMSGH("Enable locked client features", 0x03, EnableLockedFeatures),
	/*0xBA*/ RMSGH("Display Quest Arrow", 0x06, DisplayQuestArrow),
	/*0xBB*/ SMSG("Account ID ?", 0x09),
	/*0xBC*/ RMSGH("Season", 0x03, Season),
	/*0xBD*/ BMSGH("Client Version", PACKET_VARIABLE_SIZE, ClientVersion),
	/*0xBE*/ BMSGH("Assist Version", PACKET_VARIABLE_SIZE, AssistVersion),
	/*0xBF*/ BMSGH("Extended Command", PACKET_VARIABLE_SIZE, ExtendedCommand),
	/*0xC0*/ RMSGH("Graphical Effect", 0x24, GraphicEffect),
	/*0xC1*/ RMSGH("Display cliloc String", PACKET_VARIABLE_SIZE, DisplayClilocString),
	/*0xC2*/ BMSGH("Unicode prompt", PACKET_VARIABLE_SIZE, UnicodePrompt),
	/*0xC3*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xC4*/ UMSG(0x06),
	/*0xC5*/ UMSG(0xcb),
	/*0xC6*/ UMSG(0x01),
	/*0xC7*/ UMSG(0x31),
	/*0xC8*/ BMSGH("Client View Range", 0x02, ClientViewRange),
	/*0xC9*/ UMSG(0x06),
	/*0xCA*/ UMSG(0x06),
	/*0xCB*/ UMSG(0x07),
	/*0xCC*/ RMSGH("Localized Text Plus String", PACKET_VARIABLE_SIZE, DisplayClilocString),
	/*0xCD*/ UMSG(0x01),
	/*0xCE*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xCF*/ UMSG(0x4e),
	/*0xD0*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xD1*/ UMSG(0x02),
	/*0xD2*/ RMSGH("Update Player (New)", 0x19, UpdatePlayer),
	/*0xD3*/ RMSGH("Update Object (New)", PACKET_VARIABLE_SIZE, UpdateObject),
	/*0xD4*/ BMSGH("Open Book (New)", PACKET_VARIABLE_SIZE, OpenBookNew),
	/*0xD5*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xD6*/ BMSGH("Mega cliloc", PACKET_VARIABLE_SIZE, MegaCliloc),
	/*0xD7*/ BMSG("+AoS command", PACKET_VARIABLE_SIZE),
	/*0xD8*/ RMSG("+Custom house", PACKET_VARIABLE_SIZE),
	/*0xD9*/ SMSG("+Metrics", 0x10c),
	/*0xDA*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xDB*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xDC*/ RMSG("OPL Info Packet", 9),
	/*0xDD*/ RMSGH("Compressed Gump", PACKET_VARIABLE_SIZE, OpenCompressedGump),
	/*0xDE*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xDF*/ RMSGH("Buff/Debuff", PACKET_VARIABLE_SIZE, BuffDebuff),
	/*0xE0*/ SMSG("Bug Report KR", PACKET_VARIABLE_SIZE),
	/*0xE1*/ SMSG("Client Type KR/SA", 0x09),
	/*0xE2*/ RMSG("New Character Animation", 0xa),
	/*0xE3*/ RMSG("KR Encryption Responce", 0x4d),
	/*0xE4*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xE5*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xE6*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xE7*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xE8*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xE9*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xEA*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xEB*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xEC*/ SMSG("Equip Macro", PACKET_VARIABLE_SIZE),
	/*0xED*/ SMSG("Unequip item macro", PACKET_VARIABLE_SIZE),
	/*0xEE*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xEF*/ SMSG("KR/2D Client Login/Seed", 0x15),
	/*0xF0*/ BMSGH("Krrios client special", PACKET_VARIABLE_SIZE, KrriosClientSpecial),
	/*0xF1*/ BMSG("Freeshard List", PACKET_VARIABLE_SIZE),
	/*0xF2*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xF3*/ RMSGH("Update Item (SA)", 0x18, UpdateItemSA),
	/*0xF4*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xF5*/ RMSGH("Display New Map", 0x15, DisplayMap),
	/*0xF6*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xF7*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xF8*/ SMSG("Character Creation (7.0.16.0)", 0x6a),
	/*0xF9*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xFA*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xFB*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xFC*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xFE*/ UMSG(PACKET_VARIABLE_SIZE),
	/*0xFD*/ RMSG("Razor Handshake", 0x8),
	/*0xFF*/ UMSG(PACKET_VARIABLE_SIZE)
};
//----------------------------------------------------------------------------------
CPacketManager::CPacketManager()
: WISP_NETWORK::CPacketReader(), m_ClientVersion(CV_OLD), m_AutoLoginNames("")
{
}
//----------------------------------------------------------------------------------
CPacketManager::~CPacketManager()
{
}
//---------------------------------------------------------------------------
bool CPacketManager::AutoLoginNameExists(const string &name)
{
	if (!m_AutoLoginNames.length())
		return false;

	string search = string("|") + name + "|";
	
	return (m_AutoLoginNames.find(search) != string::npos);
}
//----------------------------------------------------------------------------------
#define CV_PRINT 0

#if CV_PRINT!=0
	#define CVPRINT(s) LOG(s)
#else //CV_PRINT==0
	#define CVPRINT(s)
#endif //CV_PRINT!=0
//----------------------------------------------------------------------------------
void CPacketManager::OnClientVersionChange(const CLIENT_VERSION &newClientVersion)
{
	if (newClientVersion >= CV_500A)
	{
		CVPRINT("Set new length for packet 0x0B (>= 5.0.0a)\n");
		m_Packets[0x0B].Size = 0x07;
		CVPRINT("Set new length for packet 0x16 (>= 5.0.0a)\n");
		m_Packets[0x16].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set new length for packet 0x31 (>= 5.0.0a)\n");
		m_Packets[0x31].Size = PACKET_VARIABLE_SIZE;
	}
	else
	{
		CVPRINT("Set standart length for packet 0x0B (< 5.0.0a)\n");
		m_Packets[0x0B].Size = 0x10A;
		CVPRINT("Set standart length for packet 0x16 (< 5.0.0a)\n");
		m_Packets[0x16].Size = 0x01;
		CVPRINT("Set standart length for packet 0x31 (< 5.0.0a)\n");
		m_Packets[0x31].Size = 0x01;
	}

	if (newClientVersion >= CV_5090)
	{
		CVPRINT("Set new length for packet 0xE1 (>= 5.0.9.0)\n");
		m_Packets[0xE1].Size = PACKET_VARIABLE_SIZE;
	}
	else
	{
		CVPRINT("Set standart length for packet 0xE1 (<= 5.0.9.0)\n");
		m_Packets[0xE1].Size = 0x09;
	}

	if (newClientVersion >= CV_6013)
	{
		CVPRINT("Set new length for packet 0xE3 (>= 6.0.1.3)\n");
		m_Packets[0xE3].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set new length for packet 0xE6 (>= 6.0.1.3)\n");
		m_Packets[0xE6].Size = 0x05;
		CVPRINT("Set new length for packet 0xE7 (>= 6.0.1.3)\n");
		m_Packets[0xE7].Size = 0x0C;
		CVPRINT("Set new length for packet 0xE8 (>= 6.0.1.3)\n");
		m_Packets[0xE8].Size = 0x0D;
		CVPRINT("Set new length for packet 0xE9 (>= 6.0.1.3)\n");
		m_Packets[0xE9].Size = 0x4B;
		CVPRINT("Set new length for packet 0xEA (>= 6.0.1.3)\n");
		m_Packets[0xEA].Size = 0x03;
	}
	else
	{
		CVPRINT("Set standart length for packet 0xE3 (<= 6.0.1.3)\n");
		m_Packets[0xE3].Size = 0x4D;
		CVPRINT("Set standart length for packet 0xE6 (<= 6.0.1.3)\n");
		m_Packets[0xE6].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xE7 (<= 6.0.1.3)\n");
		m_Packets[0xE7].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xE8 (<= 6.0.1.3)\n");
		m_Packets[0xE8].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xE9 (<= 6.0.1.3)\n");
		m_Packets[0xE9].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xEA (<= 6.0.1.3)\n");
		m_Packets[0xEA].Size = PACKET_VARIABLE_SIZE;
	}

	if (newClientVersion >= CV_6017)
	{
		CVPRINT("Set new length for packet 0x08 (>= 6.0.1.7)\n");
		m_Packets[0x08].Size = 0x0F;
		CVPRINT("Set new length for packet 0x25 (>= 6.0.1.7)\n");
		m_Packets[0x25].Size = 0x15;
	}
	else
	{
		CVPRINT("Set standart length for packet 0x08 (<= 6.0.1.7)\n");
		m_Packets[0x08].Size = 0x0E;
		CVPRINT("Set standart length for packet 0x25 (<= 6.0.1.7)\n");
		m_Packets[0x25].Size = 0x14;
	}

	if (newClientVersion == CV_6060)
	{
		CVPRINT("Set new length for packet 0xEE (>= 6.0.6.0)\n");
		m_Packets[0xEE].Size = 0x2000;
		CVPRINT("Set new length for packet 0xEF (>= 6.0.6.0)\n");
		m_Packets[0xEF].Size = 0x2000;
		CVPRINT("Set new length for packet 0xF1 (>= 6.0.6.0)\n");
		m_Packets[0xF1].Size = 0x09;
	}
	else
	{
		CVPRINT("Set standart length for packet 0xEE (<= 6.0.6.0)\n");
		m_Packets[0xEE].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xEF (<= 6.0.6.0)\n");
		m_Packets[0xEF].Size = 0x15;
		CVPRINT("Set standart length for packet 0xF1 (<= 6.0.6.0)\n");
		m_Packets[0xF1].Size = PACKET_VARIABLE_SIZE;
	}

	if (newClientVersion >= CV_60142)
	{
		CVPRINT("Set new length for packet 0xB9 (>= 6.0.14.2)\n");
		m_Packets[0xB9].Size = 0x05;
	}
	else
	{
		CVPRINT("Set standart length for packet 0xB9 (<= 6.0.14.2)\n");
		m_Packets[0xB9].Size = 0x03;
	}

	if (newClientVersion >= CV_7000)
	{
		CVPRINT("Set new length for packet 0xEE (>= 7.0.0.0)\n");
		m_Packets[0xEE].Size = 0x2000;
		CVPRINT("Set new length for packet 0xEF (>= 7.0.0.0)\n");
		m_Packets[0xEF].Size = 0x2000;
		/*CVPRINT("Set new length for packet 0xF0 (>= 7.0.0.0)\n");
		m_Packets[0xF0].size = 0x2000;
		CVPRINT("Set new length for packet 0xF1 (>= 7.0.0.0)\n");
		m_Packets[0xF1].size = 0x2000;
		CVPRINT("Set new length for packet 0xF2 (>= 7.0.0.0)\n");
		m_Packets[0xF2].size = 0x2000;*/
	}
	else
	{
		CVPRINT("Set standart length for packet 0xEE (<= 7.0.0.0)\n");
		m_Packets[0xEE].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xEF (<= 7.0.0.0)\n");
		m_Packets[0xEF].Size = 0x15;
		/*CVPRINT("Set standart length for packet 0xF0 (<= 7.0.0.0)\n");
		m_Packets[0xF0].size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xF1 (<= 7.0.0.0)\n");
		m_Packets[0xF1].size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xF2 (<= 7.0.0.0)\n");
		m_Packets[0xF2].size = PACKET_VARIABLE_SIZE;*/
	}

	if (newClientVersion >= CV_7090)
	{
		CVPRINT("Set new length for packet 0x24 (>= 7.0.9.0)\n");
		m_Packets[0x24].Size = 0x09;
		CVPRINT("Set new length for packet 0x99 (>= 7.0.9.0)\n");
		m_Packets[0x99].Size = 0x1E;
		CVPRINT("Set new length for packet 0xBA (>= 7.0.9.0)\n");
		m_Packets[0xBA].Size = 0x0A;
		CVPRINT("Set new length for packet 0xF3 (>= 7.0.9.0)\n");
		m_Packets[0xF3].Size = 0x1A;

		//� ������� 7.0.8.2 ��� ��������
		CVPRINT("Set new length for packet 0xF1 (>= 7.0.9.0)\n");
		m_Packets[0xF1].Size = 0x09;
		CVPRINT("Set new length for packet 0xF2 (>= 7.0.9.0)\n");
		m_Packets[0xF2].Size = 0x19;
	}
	else
	{
		CVPRINT("Set standart length for packet 0x24 (<= 7.0.9.0)\n");
		m_Packets[0x24].Size = 0x07;
		CVPRINT("Set standart length for packet 0x99 (<= 7.0.9.0)\n");
		m_Packets[0x99].Size = 0x1A;
		CVPRINT("Set standart length for packet 0xBA (<= 7.0.9.0)\n");
		m_Packets[0xBA].Size = 0x06;
		CVPRINT("Set standart length for packet 0xF3 (<= 7.0.9.0)\n");
		m_Packets[0xF3].Size = 0x18;

		//� ������� 7.0.8.2 ��� ��������
		CVPRINT("Set standart length for packet 0xF1 (<= 7.0.9.0)\n");
		m_Packets[0xF1].Size = PACKET_VARIABLE_SIZE;
		CVPRINT("Set standart length for packet 0xF2 (<= 7.0.9.0)\n");
		m_Packets[0xF2].Size = PACKET_VARIABLE_SIZE;
	}

	if (newClientVersion >= CV_70180)
	{
		CVPRINT("Set new length for packet 0x00 (>= 7.0.18.0)\n");
		m_Packets[0x00].Size = 0x6A;
	}
	else
	{
		CVPRINT("Set standart length for packet 0x24 (<= 7.0.18.0)\n");
		m_Packets[0x00].Size = 0x68;
	}
}
//----------------------------------------------------------------------------------
int CPacketManager::GetPacketSize(const UCHAR_LIST &packet, int &offsetToSize)
{
	if (packet.size())
		return m_Packets[packet[0]].Size;

	return 0;
}
//---------------------------------------------------------------------------
void CPacketManager::SendMegaClilocRequests()
{
	if (m_MegaClilocRequests.size())
	{
		CPacketMegaClilocRequest(m_MegaClilocRequests).Send();

		m_MegaClilocRequests.clear();
	}
}
//----------------------------------------------------------------------------------
void CPacketManager::OnPacket()
{
	uint ticks = g_Ticks;
	g_TotalRecvSize += m_Size;

	CPacketInfo &info = m_Packets[*m_Start];

	LOG("--- ^(%d) r(+%d => %d) Server:: %s\n", ticks - g_LastPacketTime, m_Size, g_TotalRecvSize, info.Name);
	LOG_DUMP(m_Start, m_Size);

	g_LastPacketTime = ticks;

	if (info.Direction != DIR_RECV && info.Direction != DIR_BOTH)
		LOG("message direction invalid: 0x%02X\n", *m_Start);
	else if (info.Handler != 0)
	{
		//if (PluginManager->PacketRecv(buf, size))
		{
			m_Ptr = m_Start + 1;

			if (!info.Size)
				m_Ptr += 2;

			(this->*(info.Handler))();
		}
	}
}
//----------------------------------------------------------------------------------
#define PACKET_HANDLER(name) void CPacketManager::Handle ##name ()
//----------------------------------------------------------------------------------
PACKET_HANDLER(LoginError)
{
	if (g_GameState == GS_MAIN_CONNECT || g_GameState == GS_SERVER_CONNECT)
	{
		g_ConnectionScreen.ConnectionFailed = true;
		g_ConnectionScreen.ErrorCode = ReadUInt8();
		g_ConnectionManager.Disconnect();
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ServerList)
{
	g_ServerList.Clear();
	Move(1);

	int numServers = ReadUInt16BE();

	if (numServers == 0)
		LOG("Warning!!! Empty server list\n");

	// Calculate expected message size
	int exSize = 6 + numServers * 40;
	if (m_Size != exSize)
		LOG("Warning!!! Server list message size should be %d\n", exSize);

	IFOR(i, 0, numServers)
	{
		ushort id = ReadUInt16BE();
		string name = ReadString(32);
		uchar fullPercent = ReadUInt8();
		uchar timezone = ReadUInt8();
		uchar ip = ReadUInt32LE(); //little-endian!!!

		g_ServerList.AddServer(new CServer(id, name, fullPercent, timezone, ip, i == 0));
	}

	if (numServers && g_MainScreen.m_AutoLogin->Checked)
		g_Orion.ServerSelection(0);
	else
		g_Orion.InitScreen(GS_SERVER);

	g_ServerScreen.UpdateContent();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(RelayServer)
{
	memset(&g_SelectedCharName[0], 0, sizeof(g_SelectedCharName));
	in_addr addr;
	puint paddr = (puint)m_Ptr;
	Move(4);
	addr.S_un.S_addr = *paddr;
	char relayIP[30] = { 0 };
	memcpy(&relayIP[0], inet_ntoa(addr), 29);
	int relayPort = ReadUInt16BE();
	g_Orion.RelayServer(relayIP, relayPort, m_Ptr);
	g_PacketLoginComplete = false;
	g_CurrentMap = 0;
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(CharacterList)
{
	HandleResendCharacterList();

	uchar locCount = ReadUInt8();

	g_CityList.Clear();

	if (m_ClientVersion >= CV_70130)
	{
		IFOR(i, 0, locCount)
		{
			CCityItemNew city;

			city.LocationIndex = ReadUInt8();

			city.SetName(ReadString(32));
			city.SetArea(ReadString(32));

			city.X = ReadUInt32BE();
			city.Y = ReadUInt32BE();
			city.Z = ReadUInt32BE();
			city.MapIndex = ReadUInt32BE();
			city.Cliloc = ReadUInt32BE();

			Move(4);

			g_CityList.AddCity(city);
		}
	}
	else
	{
		IFOR(i, 0, locCount)
		{
			CCityItem city;

			city.LocationIndex = ReadUInt8();

			city.SetName(ReadString(31));
			city.SetArea(ReadString(31));

			city.InitCity();

			g_CityList.AddCity(city);
		}
	}

	g_ClientFlag = ReadUInt32BE();

	g_CharacterList.OnePerson = (bool)(g_ClientFlag & LFF_TD);
	g_SendLogoutNotification = (bool)(g_ClientFlag & LFF_RE);
	g_NPCPopupEnabled = (bool)(g_ClientFlag & LFF_LBR);
	g_ChatEnabled = (bool)(g_ClientFlag & LFF_T2A);

	g_CharacterListScreen.UpdateContent();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ResendCharacterList)
{
	g_Orion.InitScreen(GS_CHARACTER);

	int numSlots = ReadInt8();
	if (*m_Start == 0x86)
		LOG("/======Resend chars===\n");
	else
		LOG("/======Chars===\n");

	g_CharacterList.Clear();
	g_CharacterList.Count = numSlots;

	int AutoPos = -1;
	bool autoLogin = g_MainScreen.m_AutoLogin->Checked;

	if (numSlots == 0)
		LOG("Warning!!! No slots in character list\n");
	else
	{
		IFOR(i, 0, numSlots)
		{
			g_CharacterList.SetName(i, (char*)Ptr);

			if (autoLogin && AutoPos == -1 && AutoLoginNameExists((char*)Ptr))
				AutoPos = i;

			LOG("%d: %s\n", i, (char*)Ptr);

			Move(60);
		}
	}

	if (autoLogin && numSlots)
	{
		if (AutoPos == -1)
			AutoPos = 0;

		g_CharacterList.Selected = AutoPos;

		if (g_CharacterList.GetName(AutoPos).length())
			g_Orion.CharacterSelection(AutoPos);
	}

	if (*m_Start == 0x86)
		g_CharacterListScreen.UpdateContent();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(LoginComplete)
{
	g_PacketLoginComplete = true;

	g_Orion.LoginComplete();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(SetTime)
{
	g_ServerTimeHour = ReadUInt8();
	g_ServerTimeMinute = ReadUInt8();
	g_ServerTimeSecond = ReadUInt8();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(EnterWorld)
{
	uint serial = ReadUInt32BE();

	if (g_World != NULL)
		LOG("Error!!! Duplicate enter world message\n");
	else
	{
		RELEASE_POINTER(g_World);
		RELEASE_POINTER(g_Walker);
		RELEASE_POINTER(g_ObjectInHand);

		g_World = new CGameWorld(serial);
		g_Walker = new CWalker();
		g_PendingDelayTime = 0;

		g_WalkRequestCount = 0;
		g_PingCount = 0;
		g_PingSequence = 0;
		g_ClickObject.Clear();
		g_Weather.Reset();
		g_SkillsTotal = 0.0f;
		g_ConsolePrompt = PT_NONE;
		//g_MacroPointer = NULL;
		g_Season = ST_SPRING;
		g_GlobalScale = 1.0;
	}

	Move(4);

	if (strlen(g_SelectedCharName))
		g_Player->Name = g_SelectedCharName;

	g_Player->Graphic = ReadUInt16BE();
	g_Player->OnGraphicChange();

	g_Player->X = ReadUInt16BE();
	g_Player->Y = ReadUInt16BE();
	Move(1);
	g_Player->Z = ReadUInt8();
	uchar dir = ReadUInt8();
	g_Player->Direction = dir;
	g_Player->Flags = m_Start[28];

	g_MapManager->Init();
	g_MapManager->AddRender(g_Player);

	g_Walker->SetSequence(0, dir);
	g_Player->OffsetX = 0;
	g_Player->OffsetY = 0;
	g_Player->OffsetZ = 0;

	if (m_ClientVersion >= CV_500A && !g_Player->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(g_Player->Serial);

	LOG("Player 0x%08lX entered the world.\n", serial);

	g_Orion.LoadStartupConfig();

	g_LastSpellIndex = 0;
	g_LastSkillIndex = 1;

	g_Orion.Click(g_PlayerSerial);
	g_Orion.StatusReq(g_PlayerSerial);

	if (m_ClientVersion >= CV_200)
		CPacketGameWindowSize().Send();

	/*BYTE wbuf[4] = {0x65, 0x01, 0x46, 0};
	Ptr = wbuf + 1;
	HandleSetWeather(wbuf, 4);*/
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateHitpoints)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameCharacter *obj = g_World->FindWorldCharacter(serial);
	if (obj == NULL)
		return;

	obj->MaxHits = ReadInt16BE();
	obj->Hits = ReadInt16BE();

	g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
	g_GumpManager.UpdateContent(serial, 0, GT_TARGET_SYSTEM);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateMana)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameCharacter *obj = g_World->FindWorldCharacter(serial);
	if (obj == NULL)
		return;

	obj->MaxMana = ReadInt16BE();
	obj->Mana = ReadInt16BE();

	g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateStamina)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameCharacter *obj = g_World->FindWorldCharacter(serial);
	if (obj == NULL)
		return;

	obj->MaxStam = ReadInt16BE();
	obj->Stam = ReadInt16BE();

	g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdatePlayer)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	if (serial != g_PlayerSerial)
	{
		LOG("Warning!!! Current player changed from 0x%08lX to 0x%08lX\n", g_PlayerSerial, serial);
		g_World->SetPlayer(serial);
	}

	WORD oldGraphic = g_Player->Graphic;
	g_Player->Graphic = ReadUInt16BE();
	g_Player->OnGraphicChange();

	if (oldGraphic && oldGraphic != g_Player->Graphic)
	{
		if (g_Player->Dead())
		{
			g_Weather.Reset();
			g_Target.Reset();

			if (g_ConfigManager.Music)
			{
				g_SoundManager.StopMusic();
				g_SoundManager.StopWarMusic();
				g_Orion.PlayMusic(42, true);
			}

			g_DeathScreenTimer = g_Ticks + DEATH_SCREEN_DELAY;
		}
	}

	Move(1);
	g_Player->Color = ReadUInt16BE();
	g_Player->Flags = ReadUInt8();
	g_Player->X = ReadUInt16BE();
	g_Player->Y = ReadUInt16BE();
	Move(2);

	g_Player->m_WalkStack.Clear();

	uchar dir = ReadUInt8();
	g_Walker->SetSequence(0, dir);
	g_WalkRequestCount = 0;
	g_Player->OffsetX = 0;
	g_Player->OffsetY = 0;
	g_Player->OffsetZ = 0;

	if (g_Player->Direction != dir)
	{
		CGameItem *bank = g_Player->FindLayer(OL_BANK);

		if (bank != NULL)
		{
			CGump *bankContainer = g_GumpManager.GetGump(bank->Serial, 0, GT_CONTAINER);

			if (bankContainer != NULL)
				g_GumpManager.RemoveGump(bankContainer);
		}
	}

	g_Player->Direction = dir;
	g_Player->Z = ReadUInt8();

	g_World->MoveToTop(g_Player);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(CharacterStatus)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameCharacter *obj = g_World->FindWorldCharacter(serial);
	if (obj == NULL)
		return;

	string name = ReadString(30);
	obj->Name = name;

	obj->Hits = ReadInt16BE();
	obj->MaxHits = ReadInt16BE();

	obj->CanChangeName = (ReadUInt8() != 0);

	BYTE flag = ReadUInt8();

	if (flag > 0)
	{
		obj->Sex = ReadUInt8(); //buf[43];

		if (serial == g_PlayerSerial)
		{
			short newStr = ReadInt16BE();
			short newDex = ReadInt16BE();
			short newInt = ReadInt16BE();

			if (g_ConfigManager.StatReport && g_Player->Str)
			{
				short currentStr = g_Player->Str;
				short currentDex = g_Player->Dex;
				short currentInt = g_Player->Int;

				short deltaStr = newStr - currentStr;
				short deltaDex = newDex - currentDex;
				short deltaInt = newInt - currentInt;

				char str[64] = { 0 };
				if (deltaStr)
				{
					sprintf(str, "Your strength has changed by %d.  It is now %d.", deltaStr, newStr);
					g_Orion.CreateTextMessage(TT_SYSTEM, 0, 3, 0x0170, str);
				}

				if (deltaDex)
				{
					sprintf(str, "Your dexterity has changed by %d.  It is now %d.", deltaDex, newDex);
					g_Orion.CreateTextMessage(TT_SYSTEM, 0, 3, 0x0170, str);
				}

				if (deltaInt)
				{
					sprintf(str, "Your intelligence has changed by %d.  It is now %d.", deltaInt, newInt);
					g_Orion.CreateTextMessage(TT_SYSTEM, 0, 3, 0x0170, str);
				}
			}

			g_Player->Str = newStr;
			g_Player->Dex = newDex;
			g_Player->Int = newInt;

			g_Player->Stam = ReadInt16BE();
			g_Player->MaxStam = ReadInt16BE();
			g_Player->Mana = ReadInt16BE();
			g_Player->MaxMana = ReadInt16BE();
			g_Player->Gold = ReadUInt32BE();
			g_Player->Armor = ReadInt16BE();
			g_Player->Weight = ReadInt16BE(); //+64

			if (flag >= 5)
			{
				g_Player->MaxWeight = ReadInt16BE(); //unpack16(buf + 66);
				g_Player->Race = (CHARACTER_RACE_TYPE)ReadUInt8();
			}
			else
				g_Player->MaxWeight = (g_Player->Str * 4) + 25;

			if (flag >= 3)
			{
				g_Player->StatsCap = ReadUInt16BE();
				g_Player->Followers = ReadUInt8();
				g_Player->MaxFollowers = ReadUInt8();
			}

			if (flag >= 4)
			{
				g_Player->FireResistance = ReadInt16BE();
				g_Player->ColdResistance = ReadInt16BE();
				g_Player->PoisonResistance = ReadInt16BE();
				g_Player->EnergyResistance = ReadInt16BE();
				g_Player->Luck = ReadInt16BE();
				g_Player->MinDamage = ReadInt16BE();
				g_Player->MaxDamage = ReadInt16BE();
				g_Player->TithingPoints = ReadUInt32BE();
			}

			if (!g_ConnectionScreen.Completed && g_PacketLoginComplete)
				g_Orion.LoginComplete();
		}
	}

	g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
	g_GumpManager.UpdateContent(serial, 0, GT_TARGET_SYSTEM);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateItem)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	ushort graphic = ReadUInt16BE();

#if UO_ABYSS_SHARD == 1
	if ((graphic & 0x7FFF) == 0x0E5C)
		return;
#endif

	ushort count = 0;

	if (serial & 0x80000000)
	{
		serial &= 0x7FFFFFFF;
		count = ReadUInt16BE();
	}

	CGameItem *obj = g_World->GetWorldItem(serial);
	if (obj == NULL)
	{
		LOG("no memory??");
		return;
	}

	if (g_ObjectInHand != NULL && g_ObjectInHand->Serial == obj->Serial && g_ObjectInHand->Dropped)
	{
		delete g_ObjectInHand;
		g_ObjectInHand = NULL;
	}

	if (obj->Dragged)
		g_GumpManager.CloseGump(serial, 0, GT_DRAG);

	obj->Dragged = false;
	obj->MapIndex = g_CurrentMap;

	if (graphic & 0x8000)
	{
		graphic &= 0x7FFF;
		graphic += ReadUInt8();
	}

	if (!obj->Graphic)
		LOG("created ");
	else
		LOG("updated ");

	obj->Graphic = graphic;
	obj->Count = count;
	ushort x = ReadUInt16BE();
	ushort y = ReadUInt16BE();
	uchar dir = 0;

	if (x & 0x8000)
	{
		x &= 0x7FFF;

		//obj->Direction = *Ptr; //����������� ��������?
		dir = ReadUInt8();
	}

	obj->X = x;
	obj->Z = ReadUInt8();

	if (y & 0x8000)
	{
		y &= 0x7FFF;

		obj->Color = ReadUInt16BE();
	}
	else
		obj->Color = 0;

	if (y & 0x4000)
	{
		y &= 0x3FFF;
		obj->Flags = ReadUInt8();
	}

	obj->Y = y;

	obj->OnGraphicChange(dir);

	g_World->MoveToTop(obj);

	if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(obj->Serial);

	LOG("0x%08lX:0x%04X*%d %d:%d:%d\n", serial, graphic, obj->Count, obj->X, obj->Y, obj->Z);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateItemSA)
{
	if (g_World == NULL)
		return;

	Move(2);
	char type = ReadUInt8(); //buf[3];
	uint serial = ReadUInt32BE();
	ushort graphic = ReadUInt16BE();
	uchar dir = ReadUInt8();
	WORD count = ReadUInt16BE();
	Move(2);
	ushort x = ReadUInt16BE();
	ushort y = ReadUInt16BE();
	char z = ReadUInt8();
	Move(1);
	ushort color = ReadUInt16BE();
	char flags = ReadUInt8();

	CGameItem *obj = g_World->GetWorldItem(serial);

	if (obj == NULL)
	{
		LOG("no memory??");
		return;
	}

	if (g_ObjectInHand != NULL && g_ObjectInHand->Serial == obj->Serial)
	{
		delete g_ObjectInHand;
		g_ObjectInHand = NULL;
	}

	if (obj->Dragged)
		g_GumpManager.CloseGump(serial, 0, GT_DRAG);

	obj->Dragged = false;
	obj->MapIndex = g_CurrentMap;

	if (!obj->Graphic)
		LOG("created ");
	else
		LOG("updated ");

	obj->Graphic = graphic;
	obj->Count = count;

	obj->X = x;
	obj->Y = y;
	obj->Z = z;

	obj->Color = color;

	obj->OnGraphicChange(dir);

	obj->Flags = flags;

	LOG("0x%08lX:0x%04X*%d %d:%d:%d\n", serial, obj->Graphic, obj->Count, obj->X, obj->Y, obj->Z);

	if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(obj->Serial);

	g_World->MoveToTop(obj);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateObject)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameItem *item = NULL;
	CGameCharacter *character = NULL;
	CGameObject *obj = NULL;

	if (serial & 0x80000000)
	{
		item = g_World->GetWorldItem(serial & 0x7FFFFFFF);
		obj = item;
	}
	else
	{
		character = g_World->GetWorldCharacter(serial & 0x7FFFFFFF);
		obj = character;
	}

	if (obj == NULL)
	{
		LOG("no memory??");
		return;
	}

	if (g_ObjectInHand != NULL && g_ObjectInHand->Serial == obj->Serial)
	{
		delete g_ObjectInHand;
		g_ObjectInHand = NULL;
	}

	obj->MapIndex = g_CurrentMap;

	ushort graphic = ReadUInt16BE();

	if (serial & 0x80000000)
	{
		obj->Count = ReadUInt16BE();
		obj->NPC = false;
	}
	else
		obj->NPC = true;

	if (graphic & 0x8000)
	{
		graphic &= 0x7FFF;

		graphic += ReadUInt16BE();
		//obj->GraphicIncrement = unpack16(ptr);
		//Move(2);
	}

	obj->Graphic = graphic;

	ushort x = ReadUInt16BE();

	if (x & 0x8000)
	{
		x &= 0x7FFF;

		Move(1); //direction2 ?????
	}

	ushort newX = x;
	ushort newY = ReadUInt16BE();
	char newZ = ReadUInt8();

	uchar dir = ReadUInt8();

	if (character != NULL)
		obj->OnGraphicChange(1000);
	else
		obj->OnGraphicChange(dir);

	obj->Color = ReadUInt16BE();

	bool hidden = obj->Hidden();
	obj->Flags = ReadUInt8();
	bool updateCoords = (hidden == obj->Hidden());

	uchar noto = ReadUInt8();

	if (updateCoords)
	{
		if (character != NULL && !character->m_WalkStack.Empty())
		{
			if (newX != obj->X || newX != obj->X)
			{
				obj->X = character->m_WalkStack.m_Items->X;
				obj->Y = character->m_WalkStack.m_Items->Y;
				obj->Z = character->m_WalkStack.m_Items->Z;
				character->m_WalkStack.Clear();
				updateCoords = false;
			}
		}

		if (updateCoords)
		{
			obj->X = newX;
			obj->Y = newY;
			obj->Z = newZ;

			if (character != NULL)
				character->Direction = dir;
		}
	}

	if (character != NULL)
	{
		character->Notoriety = noto;
		LOG("0x%08X 0x%04X NPC %d,%d,%d C%04X F%02X D%d N%d\n", serial, obj->Graphic, obj->X, obj->Y, obj->Z, obj->Color, obj->Flags, character->Direction, character->Notoriety);
	}
	else
		LOG("0x%08X 0x%04X %d,%d,%d C%04X F%02X\n", serial, obj->Graphic, obj->X, obj->Y, obj->Z, obj->Color, obj->Flags);

	if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(obj->Serial);

	serial = ReadUInt32BE();

	g_World->MoveToTop(obj);

	puchar end = m_Start + m_Size;

	if (*m_Start != 0x78)
		end -= 6;

	UINT_LIST megaClilocRequestList;

	while (serial != 0)
	{
		if (m_Ptr >= end)
			break;

		CGameItem *obj2 = g_World->GetWorldItem(serial);

		obj2->MapIndex = g_CurrentMap;

		graphic = ReadUInt16BE();

		BYTE layer = ReadUInt8();

		if (graphic & 0x8000)
		{
			graphic &= 0x7FFF;

			obj2->Color = ReadUInt16BE();
		}
		//else if (m_ClientVersion >= CV_7090)
		//	obj2->Color = ReadUInt16BE();

		obj2->Graphic = graphic;

		g_World->PutEquipment(obj2, obj, layer);
		obj2->OnGraphicChange();

		LOG("\t0x%08X:%04X [%d] %04X\n", obj2->Serial, obj2->Graphic, layer, obj2->Color);

		if (m_ClientVersion >= CV_500A && !obj2->GetClilocMessage().length())
			megaClilocRequestList.push_back(obj2->Serial);

		g_World->MoveToTop(obj2);

		serial = ReadUInt32BE();
	}

	if (megaClilocRequestList.size())
	{
		CPacketMegaClilocRequest(megaClilocRequestList).Send();

		megaClilocRequestList.clear();
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(EquipItem)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	if (g_ObjectInHand != NULL && g_ObjectInHand->Serial == serial)
	{
		delete g_ObjectInHand;
		g_ObjectInHand = NULL;
	}

	CGameItem *obj = g_World->GetWorldItem(serial);
	obj->MapIndex = g_CurrentMap;
	obj->Graphic = ReadUInt16BE();
	Move(1);
	int layer = ReadUInt8();
	uint cserial = ReadUInt32BE();
	obj->Color = ReadUInt16BE();

	g_World->PutEquipment(obj, cserial, layer);
	obj->OnGraphicChange();

	if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(obj->Serial);

	if (layer < OL_MOUNT)
		g_GumpManager.UpdateContent(cserial, 0, GT_PAPERDOLL);

	if (layer >= OL_BUY_RESTOCK && layer <= OL_SELL)
		obj->Clear();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateContainedItem)
{
	if (g_World == NULL)
		return;

	CGameItem *obj = g_World->GetWorldItem(ReadUInt32BE());
	if (obj == NULL)
		return;

	if (g_ObjectInHand != NULL && g_ObjectInHand->Serial == obj->Serial)
	{
		if (g_ObjectInHand->Separated)
			g_ObjectInHand->Separated = false;
		else
		{
			delete g_ObjectInHand;
			g_ObjectInHand = NULL;
		}
	}

	if (obj->Dragged)
		g_GumpManager.CloseGump(obj->Serial, 0, GT_DRAG);

	obj->Dragged = false;
	obj->MapIndex = g_CurrentMap;

	obj->Graphic = ReadUInt16BE();
	obj->OnGraphicChange();
	Move(1);
	obj->Count = ReadUInt16BE();
	obj->X = ReadUInt16BE();
	obj->Y = ReadUInt16BE();
	obj->Z = 0;

	if (m_ClientVersion >= CV_6017)
		Move(1);

	uint cserial = ReadUInt32BE();
	bool canPut = true;

	if (obj->Layer != OL_NONE)
	{
		CGameObject *container = g_World->FindWorldObject(cserial);
		if (container != NULL && container->IsCorpse())
		{
			canPut = false;
			g_World->PutEquipment(obj, container, obj->Layer);
		}
	}

	if (canPut)
	{
		obj->Layer = OL_NONE;
		g_World->PutContainer(obj, cserial);
	}

	obj->Color = ReadUInt16BE();

	if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(obj->Serial);

	if (obj->Graphic == 0x0EB0) //Message board item
	{
		CPacketBulletinBoardRequestMessageSummary(cserial, obj->Serial).Send();

		g_GumpManager.UpdateGump(cserial, 0, GT_BULLETIN_BOARD);
	}

	g_World->MoveToTop(obj);

	CGameItem *container = g_World->FindWorldItem(cserial);
	if (container != NULL)
	{
		CGump *gump = g_GumpManager.UpdateContent(cserial, 0, GT_SPELLBOOK);
		if (gump == NULL)
			gump = g_GumpManager.UpdateContent(cserial, 0, GT_CONTAINER);

		if (gump != NULL)
			container->Opened = true;

		CGameObject *top = container->GetTopObject();

		if (top != NULL)
			g_GumpManager.UpdateContent(top->Serial, 0, GT_TRADE);
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateContainedItems)
{
	if (g_World == NULL)
		return;

	ushort count = ReadUInt16BE();
	uint cupd = 0;
	CGameItem *contobj = NULL;
	bool isContGameBoard = false;
	bool bbUpdated = false;
	vector<CORPSE_EQUIPMENT_DATA> vced;
	bool containerIsCorpse = false;
	UINT_LIST megaClilocRequestList;

	IFOR(i, 0, count)
	{
		uint serial = ReadUInt32BE();
		ushort graphic = ReadUInt16BE();
		Move(1);
		ushort count = ReadUInt16BE();
		ushort x = ReadUInt16BE();
		ushort y = ReadUInt16BE();

		if (m_ClientVersion >= CV_6017)
			Move(1);

		uint cserial = ReadUInt32BE();

		if (cupd && cserial != cupd)
		{
			puchar oldPtr = m_Ptr;
			m_Ptr -= 5;

			cserial = ReadUInt32BE();

			if (cserial != cupd)
				continue;
		}

		ushort color = ReadUInt16BE();

		contobj = g_World->GetWorldItem(cserial);

		contobj->MapIndex = g_CurrentMap;
		ushort contgraphic = contobj->Graphic;

		if (!cupd)
		{
			cupd = cserial;
			CGameObject *objA = g_World->FindWorldObject(cupd);
			LOG("Making %08X empty...\n", cupd);

			if (objA != NULL)
			{
				if (objA->IsCorpse())
				{
					containerIsCorpse = true;

					QFOR(citem, objA->m_Items, CGameItem*)
					{
						int lay = citem->Layer;

						if (lay > 0 && lay < OL_MOUNT)
						{
							CORPSE_EQUIPMENT_DATA ced = { citem->Serial, lay };
							vced.push_back(ced);
						}
					}
				}

				objA->Clear();

				if (contobj->Opened)
				{
					CGump *gameGump = g_GumpManager.GetGump(contobj->Serial, 0, GT_CONTAINER);

					if (gameGump != NULL)
						isContGameBoard = ((CGumpContainer*)gameGump)->IsGameBoard;
				}
			}
		}

		CGameItem *obj = g_World->GetWorldItem(serial);

		obj->MapIndex = g_CurrentMap;
		obj->Layer = 0;

		if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
			megaClilocRequestList.push_back(obj->Serial);

		g_World->PutContainer(obj, cserial);

		obj->Graphic = graphic;
		obj->OnGraphicChange();
		obj->Count = count;
		obj->X = x;
		obj->Y = y;
		obj->Color = color;

		if (obj->Graphic == 0x0EB0) //Message board item
		{
			CPacketBulletinBoardRequestMessageSummary(cserial, serial).Send();

			if (!bbUpdated)
			{
				bbUpdated = true;

				g_GumpManager.UpdateGump(cserial, 0, GT_BULLETIN_BOARD);
			}
		}

		LOG("\t|0x%08X<0x%08X:%04X*%d (%d,%d) %04X\n", obj->Container, obj->Serial, obj->Graphic, obj->Count, obj->X, obj->Y, obj->Color);
	}

	if (megaClilocRequestList.size())
	{
		CPacketMegaClilocRequest(megaClilocRequestList).Send();

		megaClilocRequestList.clear();
	}

	if (containerIsCorpse)
	{
		IFOR(i, 0, (int)vced.size())
		{
			CGameItem *gi = g_World->FindWorldItem(vced[i].Serial);

			if (gi != NULL)
				g_World->PutEquipment(gi, cupd, vced[i].Layer);
		}

		vced.clear();
	}

	if (contobj != NULL)
	{
		CGump *gump = g_GumpManager.UpdateContent(contobj->Serial, 0, GT_SPELLBOOK);

		if (gump == NULL)
			gump = g_GumpManager.UpdateContent(contobj->Serial, 0, GT_CONTAINER);

		if (gump != NULL)
			contobj->Opened = true;

		CGameObject *top = contobj->GetTopObject();

		if (top != NULL)
			g_GumpManager.UpdateContent(top->Serial, 0, GT_TRADE);
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DenyMoveItem)
{
	if (g_World == NULL)
		return;

	if (g_ObjectInHand != NULL)
	{
		CGameItem *obj = g_World->FindWorldItem(g_ObjectInHand->Serial);

		if (obj == NULL)
		{
			obj = g_World->GetWorldItem(g_ObjectInHand->Serial);

			if (obj != NULL)
			{
				obj->Paste(g_ObjectInHand);

				if (m_ClientVersion >= CV_500A)
					m_MegaClilocRequests.push_back(obj->Serial);

				g_World->PutContainer(obj, g_ObjectInHand->Container);

				g_World->MoveToTop(obj);
			}
		}

		delete g_ObjectInHand;
		g_ObjectInHand = NULL;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DeleteObject)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	CGameObject *obj = g_World->FindWorldObject(serial);

	if (g_ObjectInHand != NULL && g_ObjectInHand->Serial == serial)
	{
		bool sep = g_ObjectInHand->Separated;

		if (sep)
			g_ObjectInHand->Separated = false;
		else if (g_ObjectInHand->Dropped/*Deleted*/)
		{
			delete g_ObjectInHand;
			g_ObjectInHand = NULL;
		}

		if (g_ObjectInHand != NULL)
		{
			if (obj != NULL && sep)
			{
				if (obj->Count != g_ObjectInHand->Count)
					g_ObjectInHand->Deleted = true;
				else
					g_ObjectInHand->Separated = true;
			}
			else
				g_ObjectInHand->Deleted = true;
		}
	}

	if (obj != NULL && obj->Serial != g_PlayerSerial)
	{
		uint cont = obj->Container;

		if (cont != 0xFFFFFFFF)
		{
			CGameObject *top = obj->GetTopObject();

			if (top != NULL)
				g_GumpManager.UpdateContent(top->Serial, 0, GT_TRADE);

			if (!obj->NPC && ((CGameItem*)obj)->Layer != OL_NONE)
				g_GumpManager.UpdateContent(cont, 0, GT_PAPERDOLL);

			CGump *gump = g_GumpManager.UpdateContent(cont, 0, GT_CONTAINER);

			if (obj->Graphic == 0x0EB0)
			{
				g_GumpManager.CloseGump(serial, cont, GT_BULLETIN_BOARD_ITEM);

				CGumpBulletinBoard *bbGump = (CGumpBulletinBoard*)g_GumpManager.UpdateGump(cont, 0, GT_BULLETIN_BOARD);

				if (bbGump != NULL && bbGump->m_HTMLGump != NULL)
				{
					QFOR(go, bbGump->m_HTMLGump->m_Items, CBaseGUI*)
					{
						if (go->Serial == serial)
						{
							bbGump->m_HTMLGump->Delete(go);

							int posY = 0;

							QFOR(go1, bbGump->m_HTMLGump->m_Items, CBaseGUI*)
							{
								if (go1->Type == GOT_BB_OBJECT)
								{
									go1->Y = posY;
									posY += 18;
								}
							}

							bbGump->m_HTMLGump->CalculateDataSize();

							break;
						}
					}
				}
			}
		}

		if (obj->NPC && g_Party.Contains(obj->Serial))
			obj->RemoveRender();
		else
			g_World->RemoveObject(obj);
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateCharacter)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	CGameCharacter *obj = g_World->GetWorldCharacter(serial);

	if (obj == NULL)
		return;

	if (!obj->Graphic)
	{
		if (g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR) != NULL)
			g_Orion.StatusReq(serial);

		if (g_GumpManager.UpdateContent(serial, 0, GT_TARGET_SYSTEM) != NULL)
			g_Orion.StatusReq(serial);
	}

	obj->MapIndex = g_CurrentMap;
	obj->Graphic = ReadUInt16BE();
	obj->OnGraphicChange();

	if (m_ClientVersion >= CV_500A && !obj->GetClilocMessage().length())
		m_MegaClilocRequests.push_back(obj->Serial);

	short x = ReadInt16BE();
	short y = ReadInt16BE();
	char z = ReadInt8();
	uchar dir = ReadUInt8();

	if (!obj->IsTeleportAction(x, y, dir))
	{
		if (serial != g_PlayerSerial)
		{
			CWalkData *wd = new CWalkData();
			wd->X = x;
			wd->Y = y;
			wd->Z = z;
			wd->Direction = dir;

			if (obj->m_WalkStack.Empty())
				obj->LastStepTime = g_Ticks;

			obj->m_WalkStack.Push(wd);
		}
		else
		{
			obj->X = x;
			obj->Y = y;
			obj->Z = z;
			obj->Direction = dir;
		}
	}

	obj->Color = ReadUInt16BE();
	obj->Flags = ReadUInt8();

	g_World->MoveToTop(obj);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(Warmode)
{
	if (g_World == NULL)
		return;

	g_Player->Warmode = ReadUInt8();

	g_GumpManager.UpdateContent(g_PlayerSerial, 0, GT_STATUSBAR);

	CGumpPaperdoll *gump = (CGumpPaperdoll*)g_GumpManager.GetGump(g_PlayerSerial, 0, GT_PAPERDOLL);

	if (gump != NULL && gump->m_ButtonWarmode != NULL)
	{
		ushort graphic = 0x07E5;

		if (g_Player->Warmode)
			graphic += 3;

		gump->m_ButtonWarmode->Graphic = graphic;
		gump->m_ButtonWarmode->GraphicSelected = graphic + 2;
		gump->m_ButtonWarmode->GraphicPressed = graphic + 1;

		gump->WantRedraw = true;
	}

	g_World->MoveToTop(g_Player);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(PauseControl)
{
	/*g_ClientPaused = ReadUInt8();

	if (!g_ClientPaused)
	UO->ResumeClient();*/
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenPaperdoll)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameCharacter *obj = g_World->FindWorldCharacter(serial);

	string text = ReadString(0);

	if (obj != NULL)
		obj->PaperdollText = text;

	CGumpPaperdoll *gump = (CGumpPaperdoll*)g_GumpManager.UpdateGump(serial, 0, GT_PAPERDOLL);

	if (gump == NULL)
	{
		gump = new CGumpPaperdoll(serial, 0, 0, false);
		g_GumpManager.AddGump(gump);
	}

	gump->UpdateDescription(text);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ClientVersion)
{
	CPacketClientVersion(g_Orion.ClientVersionText).Send();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(Ping)
{
	g_PingSequence = ReadUInt8();

	if (g_PingCount)
		g_PingCount--;
	else
		g_Orion.Send(m_Start, 2);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(SetWeather)
{
	g_Weather.Reset();
	uchar type = ReadUInt8();
	g_Weather.Type = type;
	g_Weather.Count = ReadUInt8();

	if (g_Weather.Count > 70)
		g_Weather.Count = 70;

	g_Weather.Temperature = ReadUInt8();
	g_Weather.Timer = g_Ticks + WEATHER_TIMER;
	g_Weather.Generate();

	switch (type)
	{
		case 0:
		{
			g_Orion.CreateTextMessage(TT_SYSTEM, 0xFFFFFFFF, 3, 0, "It begins to rain.");
			break;
		}
		case 1:
		{
			g_Orion.CreateTextMessage(TT_SYSTEM, 0xFFFFFFFF, 3, 0, "A fierce storm approaches.");
			break;
		}
		case 2:
		{
			g_Orion.CreateTextMessage(TT_SYSTEM, 0xFFFFFFFF, 3, 0, "It begins to snow.");
			break;
		}
		case 3:
		{
			g_Orion.CreateTextMessage(TT_SYSTEM, 0xFFFFFFFF, 3, 0, "A storm is brewing.");
			break;
		}
		case 0xFE:
		case 0xFF:
		{
			g_Weather.Timer = 0;
			break;
		}
		default:
			break;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(PersonalLightLevel)
{
	uint serial = ReadUInt32BE();

	if (serial == g_PlayerSerial)
	{
		uchar level = ReadUInt8();

		if (level > 0x1F)
			level = 0x1F;

		g_PersonalLightLevel = level;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(LightLevel)
{
	uchar level = ReadUInt8();

	if (level > 0x1F)
		level = 0x1F;

	g_LightLevel = level;
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(EnableLockedFeatures)
{
	if (m_ClientVersion >= CV_60142)
		g_LockedClientFeatures = ReadUInt32BE();
	else
		g_LockedClientFeatures = ReadUInt16BE();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenContainer)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	ushort gumpid = ReadUInt16BE();

	CGump *gump = NULL;
	bool addGump = true;

	if (gumpid == 0xFFFF) //Spellbook
	{
		int gameWindowCenterX = (g_ConfigManager.GameWindowX - 4) + g_ConfigManager.GameWindowWidth / 2;
		int gameWindowCenterY = (g_ConfigManager.GameWindowY - 4) + g_ConfigManager.GameWindowHeight / 2;

		int x = gameWindowCenterX - 200;
		int y = gameWindowCenterY - 100;

		if (x < 0)
			x = 0;

		if (y < 0)
			y = 0;

		//gump = new CGumpSpellbook(serial, x, y);
		g_Orion.PlaySoundEffect(0x0055);
	}
	else if (gumpid == 0x0030) //Buylist
	{
		CGumpShop *buyGump = (CGumpShop*)g_GumpManager.GetGump(serial, 0, GT_SHOP);

		if (buyGump != NULL && (buyGump->Serial != serial || !buyGump->IsBuyGump))
		{
			g_GumpManager.RemoveGump(buyGump);
			buyGump = NULL;
		}

		if (buyGump == NULL)
			buyGump = new CGumpShop(serial, true, 150, 5);
		else
			addGump = false;

		gump = buyGump;

		buyGump->Visible = true;
	}
	else //Container
	{
		ushort graphic = 0xFFFF;

		IFOR(i, 0, CONTAINERS_COUNT)
		{
			if (gumpid == g_ContainerOffset[i].Gump)
			{
				graphic = i;
				break;
			}
		}

		if (graphic == 0xFFFF)
			return;

		g_ContainerRect.Calculate(gumpid);

		gump = new CGumpContainer(serial, g_ContainerRect.X, g_ContainerRect.Y);
		gump->Graphic = graphic;
		((CGumpContainer*)gump)->m_BodyGump->Graphic = gumpid;
		((CGumpContainer*)gump)->IsGameBoard = (gumpid == 0x091A || gumpid == 0x092E);
		g_Orion.ExecuteGump(gumpid);
	}

	if (gump == NULL)
		return;

	gump->ID = gumpid;

	if (gumpid != 0x0030)
	{
		if (g_ContainerStack.size())
		{
			for (deque<CContainerStackItem>::iterator cont = g_ContainerStack.begin(); cont != g_ContainerStack.end(); cont++)
			{
				if (cont->Serial == serial)
				{
					gump->X = cont->X;
					gump->Y = cont->Y;
					gump->Minimized = cont->Minimized;
					gump->MinimizedX = cont->MinimizedX;
					gump->MinimizedY = cont->MinimizedY;
					gump->LockMoving = cont->LockMoving;

					if (cont->Minimized)
						gump->Page = 1;
					else
						gump->Page = 2;

					g_ContainerStack.erase(cont);

					break;
				}
			}

			if (g_CheckContainerStackTimer < g_Ticks)
				g_ContainerStack.clear();
		}

		CGameItem *obj = g_World->FindWorldItem(serial);

		if (obj != NULL)
		{
			/*if (gumpid != 0xFFFF)*/ obj->Opened = true;
			if (!obj->IsCorpse())
				g_World->ClearContainer(obj);
		}
	}

	if (addGump)
		g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UpdateSkills)
{
	if (g_World == NULL)
		return;

	int type = ReadUInt8();
	bool HaveCap = (type == 0x02 || type == 0xDF);
	bool IsSingleUpdate = (type == 0xFF || type == 0xDF);
	LOG("Skill update type %i (Cap=%d)\n", type, HaveCap);

	CGumpSkills *gump = (CGumpSkills*)g_GumpManager.UpdateGump(g_PlayerSerial, 0, GT_SKILLS);

	while (m_Ptr < m_End)
	{
		ushort id = ReadUInt16BE();

		if (!id && !type)
			break;
		else if (!type || type == 0x02)
			id--;

		ushort BaseVal = ReadUInt16BE();
		ushort RealVal = ReadUInt16BE();
		uchar lock = ReadUInt8();
		ushort Cap = 0;

		if (HaveCap)
			Cap = ReadUInt16BE();

		if (id < g_SkillsCount)
		{
			if (IsSingleUpdate)
			{
				float change = (float)(BaseVal / 10.0f) - g_Player->GetSkillBaseValue(id);

				if (change)
				{
					char str[128] = { 0 };
					sprintf(str, "Your skill in %s has %s by %.1f%%.  It is now %.1f%%.", g_Skills[id].Name.c_str(), ((change < 0) ? "decreased" : "increased"), change, g_Player->GetSkillBaseValue(id) + change);
					//else if (change > 0) sprintf(str, "Your skill in %s has increased by %.1f%%.  It is now %.1f%%.", UO->m_Skills[id].m_Name.c_str(), change, obj->GetSkillBaseValue(id) + change);
					g_Orion.CreateTextMessage(TT_SYSTEM, 0, 3, 0x58, str);
				}
			}

			g_Player->SetSkillBaseValue(id, (float)(BaseVal / 10.0f));
			g_Player->SetSkillValue(id, (float)(RealVal / 10.0f));
			g_Player->SetSkillCap(id, (float)(Cap / 10.0f));
			g_Player->SetSkillStatus(id, lock);

			if (gump != NULL)
				gump->UpdateSkillValue(id);

			if (HaveCap)
				LOG("Skill %i is %i|%i|%i\n", id, BaseVal, RealVal, Cap);
			else
				LOG("Skill %i is %i|%i\n", id, BaseVal, RealVal);
		}
		else
			LOG("Unknown skill update %d\n", id);
	}

	g_SkillsTotal = 0.0f;

	IFOR(i, 0, g_SkillsCount)
		g_SkillsTotal += g_Player->GetSkillValue(i);

	if (gump != NULL)
		gump->UpdateSkillsSum();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ExtendedCommand)
{
	ushort cmd = ReadUInt16BE();

	switch (cmd)
	{
		case 0:
			break;
		case 1: //Initialize Fast Walk Prevention
		{
			IFOR(i, 0, 6)
				g_Walker->m_FastWalkStack.Push(ReadUInt32BE());

			break;
		}
		case 2: //Add key to Fast Walk Stack
		{
			g_Walker->m_FastWalkStack.Push(ReadUInt32BE());

			break;
		}
		case 4: //Close generic gump
		{
			uint id = ReadUInt32BE();
			uint button = ReadUInt32BE();

			QFOR(gump, g_GumpManager.m_Items, CGump*)
			{
				if (gump->GumpType == GT_GENERIC && gump->ID == id)
				{
					((CGumpGeneric*)gump)->SendGumpResponse(button);

					break;
				}
			}

			break;
		}
		case 5: //Screen size
		{
			//g_GameWindowSizeX = unpack16(buf + 5);
			//g_GameWindowSizeY = unpack16(buf + 9);
			break;
		}
		case 6: //Party commands
		{
			g_Party.ParsePacketData(*this);

			break;
		}
		case 8: //Set cursor / map
		{
			g_Orion.ChangeMap(ReadUInt8());

			break;
		}
		case 0xC: //Close statusbar gump
		{
			uint serial = ReadUInt32BE();
			g_GumpManager.CloseGump(serial, 0, GT_STATUSBAR);

			break;
		}
		case 0x14: //Display Popup/context menu (2D and KR)
		{
			Move(1);
			uchar mode = ReadUInt8();
			uint serial = ReadUInt32BE();
			uchar count = ReadUInt8();

			CGumpPopupMenu *menu = new CGumpPopupMenu(serial, g_MouseManager.Position.X, g_MouseManager.Position.Y);
			int width = 0;
			int height = 20;

			menu->Add(new CGUIAlphaBlending(true, 0.5f));
			CGUIResizepic *resizepic = (CGUIResizepic*)menu->Add(new CGUIResizepic(0, 0x0A3C, 0, 0, 0, 0));
			menu->Add(new CGUIAlphaBlending(false, 0.5f));

			int offsetY = 10;

			IFOR(i, 0, count)
			{
				/*uint cliloc = ReadUInt32BE();
				ushort index = ReadUInt16BE() + 1;*/
				ushort index = ReadUInt16BE() + 1;
				uint cliloc = ReadUInt16BE() + 3000000;
				ushort flags = ReadUInt16BE();
				ushort color = 0xFFFF;

				if (flags == 0x20)
					color = ReadUInt16BE();

				wstring str = g_ClilocManager.Cliloc(g_Language)->GetW(cliloc);

				CGUIText *item = (CGUIText*)menu->Add(new CGUIText(color, 10, offsetY));
				item->CreateTextureW(0, str);
				item->DrawOnly = true;

				menu->Add(new CGUIHitBox(index, 10, offsetY, item->m_Texture.Width, item->m_Texture.Height, true));

				height += item->m_Texture.Height;
				offsetY += item->m_Texture.Height;

				if (width < item->m_Texture.Width)
					width = item->m_Texture.Width;
			}

			width += 20;

			if (height <= 20 || width <= 20)
				delete menu;
			else
			{
				resizepic->Width = width;
				resizepic->Height = height;

				QFOR(item, menu->m_Items, CBaseGUI*)
				{
					if (item->Type == GOT_HITBOX)
						((CGUIHitBox*)item)->Width = width - 20;
				}

				g_GumpManager.AddGump(menu);
			}

			break;
		}
		case 0x16: //Close User Interface Windows
		{
			//ID:
			//0x01: Paperdoll
			//0x02: Status
			//0x08: Character Profile
			//0x0C: Container
			uint ID = ReadUInt32BE();
			uint Serial = ReadUInt32BE();

			switch (ID)
			{
				case 1: //Paperdoll
				{
					g_GumpManager.CloseGump(Serial, 0, GT_PAPERDOLL);
					break;
				}
				case 2: //Statusbar
				{
					g_GumpManager.CloseGump(Serial, 0, GT_STATUSBAR);
					break;
				}
				case 8: //Character Profile
				{
					//UO->CloseGump(Serial, 0, GT_PROFILE);
					break;
				}
				case 0xC: //Container
				{
					g_GumpManager.CloseGump(Serial, 0, GT_CONTAINER);
					break;
				}
				default:
					break;
			}

			break;
		}
		case 0x19: //Extended stats
		{
			Move(1);

			if (ReadUInt32BE() == g_PlayerSerial)
			{
				Move(1);
				uchar state = ReadUInt8();

				g_Player->LockStr = (state >> 4) & 3;
				g_Player->LockDex = (state >> 2) & 3;
				g_Player->LockInt = state & 3;

				CGump *statusbar = g_GumpManager.GetGump(g_PlayerSerial, 0, GT_STATUSBAR);

				if (statusbar != NULL && !statusbar->Minimized)
					statusbar->WantUpdateContent = true;
			}

			break;
		}
		case 0x1B: //New spellbook content
		{
			Move(2);
			uint serial = ReadUInt32BE();

			CGameItem *spellbook = g_World->FindWorldItem(serial);

			if (spellbook == NULL)
			{
				LOG("Where is a spellbook?!?\n");
				return;
			}

			g_World->ClearContainer(spellbook);

			ushort graphic = ReadUInt16BE();
			SPELLBOOK_TYPE bookType = (SPELLBOOK_TYPE)ReadUInt16BE();

			uint spells[2] = { 0 };

			IFOR(j, 0, 2)
			{
				IFOR(i, 0, 4)
					spells[j] |= (ReadUInt8() << (i * 8));
			}

			switch (bookType)
			{
				case ST_MAGE:
				{
					IFOR(j, 0, 2)
					{
						IFOR(i, 0, 32)
						{
							if (spells[j] & (1 << i))
							{
								CGameItem *spellItem = new CGameItem();
								spellItem->Graphic = 0x1F2E;
								spellItem->Count = (j * 32) + i + 1;

								spellbook->AddItem(spellItem);
							}
						}
					}

					break;
				}
				default:
					break;
			}
		}
		case 0x26:
		{
			uchar val = ReadUInt8();

			if (val > CST_FAST_UNMOUNT_AND_CANT_RUN)
				val = 0;

			g_SpeedMode = (CHARACTER_SPEED_TYPE)val;

			break;
		}
		default:
			break;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DenyWalk)
{
	g_WalkRequestCount = 0;

	if (g_Player == NULL)
		return;

	Move(1);
	g_Player->X = ReadUInt16BE();
	g_Player->Y = ReadUInt16BE();
	uchar dir = ReadUInt8();
	g_Player->Direction = dir;
	g_Player->Z = ReadUInt8();

	g_Walker->SetSequence(0, dir);
	g_Player->OffsetX = 0;
	g_Player->OffsetY = 0;
	g_Player->OffsetZ = 0;

	g_Player->m_WalkStack.Clear();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ConfirmWalk)
{
	if (g_WalkRequestCount)
		g_WalkRequestCount--;

	if (g_Player == NULL)
		return;

	uchar Seq = ReadUInt8();
	//player->SetDirection(newdir);

	uchar newnoto = ReadUInt8();

	if (newnoto >= 0x40)
		newnoto ^= 0x40;

	if (!newnoto || newnoto >= 7)
		newnoto = 0x01;

	g_Player->Notoriety = newnoto;

	g_World->MoveToTop(g_Player);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenUrl)
{
	g_Orion.GoToWebLink(ReadString(0));
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(Target)
{
	g_Target.SetData(*this);

	if (g_PartyHelperTimer > g_Ticks && g_PartyHelperTarget)
	{
		g_Target.SendTargetObject(g_PartyHelperTarget);
		g_PartyHelperTimer = 0;
		g_PartyHelperTarget = 0;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(Talk)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	ushort graphic = ReadUInt16BE();
	SPEECH_TYPE type = (SPEECH_TYPE)ReadUInt8();
	ushort textColor = ReadUInt16BE();
	ushort font = ReadUInt16BE();

	if (!serial && font == 0xFFFF && textColor == 0xFFFF)
	{
		uchar sbuffer[0x28] =
		{
			0x03, 0x00, 0x28, 0x20, 0x00, 0x34, 0x00, 0x03, 0xdb, 0x13, 0x14, 0x3f, 0x45, 0x2c, 0x58, 0x0f,
			0x5d, 0x44, 0x2e, 0x50, 0x11, 0xdf, 0x75, 0x5c, 0xe0, 0x3e, 0x71, 0x4f, 0x31, 0x34, 0x05, 0x4e,
			0x18, 0x1e, 0x72, 0x0f, 0x59, 0xad, 0xf5, 0x00
		};

		g_Orion.Send(sbuffer, 0x28);

		return;
	}

	string name(ReadString(0));
	string str = "";

	if (m_Size > 44)
	{
		m_Ptr = m_Start + 44;
		str = ReadString(0);
	}

	if (type == ST_BROADCAST || /*type == ST_SYSTEM ||*/ serial == 0xFFFFFFFF || !serial || name == string("System"))
		g_Orion.CreateTextMessage(TT_SYSTEM, serial, (uchar)font, textColor, str);
	else
	{
		if (type == ST_EMOTE)
		{
			textColor = g_ConfigManager.EmoteColor;
			str = "*" + str + "*";
		}

		CGameObject *obj = g_World->FindWorldObject(serial);

		if (obj != NULL)
		{
			obj->YouSeeJournalPrefix = (type == ST_SYSTEM);

			if (!obj->Name.length())
			{
				obj->Name = name;

				if (obj->NPC)
					g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
			}
		}

		g_Orion.CreateTextMessage(TT_OBJECT, serial, (uchar)font, textColor, str);

		if (obj != NULL)
			obj->YouSeeJournalPrefix = false;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UnicodeTalk)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	ushort graphic = ReadUInt16BE();
	SPEECH_TYPE type = (SPEECH_TYPE)ReadUInt8();
	ushort textColor = ReadUInt16BE();
	ushort font = ReadUInt16BE();
	uint language = ReadUInt32BE();

	if (!serial && font == 0xFFFF && textColor == 0xFFFF)
	{
		uchar sbuffer[0x28] =
		{
			0x03, 0x00, 0x28, 0x20, 0x00, 0x34, 0x00, 0x03, 0xdb, 0x13, 0x14, 0x3f, 0x45, 0x2c, 0x58, 0x0f,
			0x5d, 0x44, 0x2e, 0x50, 0x11, 0xdf, 0x75, 0x5c, 0xe0, 0x3e, 0x71, 0x4f, 0x31, 0x34, 0x05, 0x4e,
			0x18, 0x1e, 0x72, 0x0f, 0x59, 0xad, 0xf5, 0x00
		};

		g_Orion.Send(sbuffer, 0x28);

		return;
	}

	wstring name((wchar_t*)m_Ptr);
	//wstring name = ReadUnicodeStringLE(0);
	wstring str = L"";

	if (m_Size > 48)
	{
		m_Ptr = m_Start + 48;
		str = ReadWString((m_Size - 48) / 2);
	}

	if (type == ST_BROADCAST /*|| type == ST_SYSTEM*/ || serial == 0xFFFFFFFF || !serial || name == wstring(L"System"))
		g_Orion.CreateUnicodeTextMessage(TT_SYSTEM, serial, (uchar)g_ConfigManager.SpeechFont, textColor, str);
	else
	{
		if (type == ST_EMOTE)
		{
			textColor = g_ConfigManager.EmoteColor;
			str = L"*" + str + L"*";
		}

		CGameObject *obj = g_World->FindWorldObject(serial);

		if (obj != NULL)
		{
			obj->YouSeeJournalPrefix = (type == ST_SYSTEM);

			if (!obj->Name.length())
			{
				obj->Name = ToString(name);

				if (obj->NPC)
					g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
			}
		}

		g_Orion.CreateUnicodeTextMessage(TT_OBJECT, serial, (uchar)g_ConfigManager.SpeechFont, textColor, str);

		if (obj != NULL)
			obj->YouSeeJournalPrefix = false;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ClientTalk)
{
	if (!g_AbyssPacket03First)
	{
		bool parse = true;

		if (m_Start[4] == 0x78)
		{
			m_Size = ReadInt16BE();
			m_Start += 4;
		}
		else if (m_Start[4] == 0x3C)
		{
			m_Size = ReadInt16BE();
			m_Start += 4;
		}
		else if (m_Start[4] == 0x25)
		{
			m_Size = 0x14;
			m_Start += 4;
		}
		else if (m_Start[4] == 0x2E)
		{
			m_Size = 0x0F;
			m_Start += 4;
		}
		else
			parse = false;

		if (parse)
			OnPacket();
	}

	g_AbyssPacket03First = false;
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(MultiPlacement)
{
	if (g_World == NULL)
		return;

	//uint serial = unpack32(buf + 2);
	//ushort graphic = unpack16(buf + 18);

	g_Target.SetMultiData(*this);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(GraphicEffect)
{
	if (g_World == NULL)
		return;

	uchar type = ReadUInt8();

	if (type > 3) //error
		return;

	uint sourceSerial = ReadUInt32BE();
	uint destSerial = ReadUInt32BE();
	ushort graphic = ReadUInt16BE();
	short sourceX = ReadInt16BE();
	short sourceY = ReadInt16BE();
	char sourceZ = ReadInt8();
	short destX = ReadInt16BE();
	short destY = ReadInt16BE();
	char destZ = ReadInt8();
	uchar speed = ReadUInt8();
	short duration = (short)ReadUInt8() * 50;
	//what is in 24-25 bytes?
	Move(2);
	uchar fixedDirection = ReadUInt8();
	uchar explode = ReadUInt8();

	uint color = 0;
	uint renderMode = 0;

	if (*m_Start == 0xC0)
	{
		color = ReadUInt32BE();
		renderMode = ReadUInt32BE() % 7;
	}

	CGameEffect *effect = NULL;
	if (!type) //Moving
		effect = new CGameEffectMoving();
	else
		effect = new CGameEffect();

	effect->EffectType = (EFFECT_TYPE)type;
	effect->Serial = sourceSerial;
	effect->DestSerial = destSerial;
	effect->Graphic = graphic;
	effect->X = sourceX;
	effect->Y = sourceY;
	effect->Z = sourceZ;
	effect->DestX = destX;
	effect->DestY = destY;
	effect->DestZ = destZ;

	/*DWORD addressAnimData = (DWORD)FileManager.AnimdataMul.Address;

	if (addressAnimData)
	{
		PANIM_DATA pad = (PANIM_DATA)(addressAnimData + ((graphic * 68) + 4 * ((graphic / 8) + 1)));

		effect->Speed = (pad->FrameInterval - effect->Speed) * 50;
	}
	else*/
	effect->Speed = speed + 6;

	effect->Duration = g_Ticks + duration;
	effect->FixedDirection = (fixedDirection != 0);
	effect->Explode = (explode != 0);

	effect->Color = (ushort)color;
	effect->RenderMode = renderMode;

	g_EffectManager.AddEffect(effect);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DeathScreen)
{
	g_Weather.Reset();
	g_Target.Reset();
	g_DeathScreenTimer = g_Ticks + DEATH_SCREEN_DELAY;
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(PlaySoundEffect)
{
	Move(1);
	ushort index = ReadUInt16BE();
	ushort volume = ReadUInt16BE();
	ushort xCoord = ReadUInt16BE();
	ushort yCoord = ReadUInt16BE();

	int distance = GetDistance(g_Player, WISP_GEOMETRY::CPoint2Di(xCoord, yCoord));
	//LOG("Play sound 0x%04X\n", index);

	g_Orion.PlaySoundEffect(index, g_SoundManager.GetVolumeValue(distance));

}
//----------------------------------------------------------------------------------
PACKET_HANDLER(PlayMusic)
{
	ushort index = ReadUInt16BE();

	//LOG("Play midi music 0x%04X\n", index);
	if (!g_ConfigManager.Music || GetForegroundWindow() != g_OrionWindow.Handle || g_ConfigManager.MusicVolume < 1)
		return;

	g_Orion.PlayMusic(index);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DragAnimation)
{
	if (g_World == NULL)
		return;

	ushort graphic = ReadUInt16BE();

	if (graphic == 0x0EED)
		graphic = 0x0EEF;
	else if (graphic == 0x0EEA)
		graphic = 0x0EEC;
	else if (graphic == 0x0EF0)
		graphic = 0x0EF2;

	Move(3);
	ushort count = ReadUInt16BE();

	uint sourceSerial = ReadUInt32BE();
	short sourceX = ReadInt16BE();
	short sourceY = ReadInt16BE();
	char sourceZ = ReadInt8();
	uint destSerial = ReadUInt32BE();
	short destX = ReadInt16BE();
	short destY = ReadInt16BE();
	char destZ = ReadInt8();

	CGameEffect *effect = NULL;

	if (sourceSerial < 0x40000000) //�����/��� ������ ������� � ���������
	{
		effect = new CGameEffectMoving();
		effect->FixedDirection = true;
	}
	else //������� ����� �� ����������
	{
		effect = new CGameEffectDrag();
	}

	effect->EffectType = EF_DRAG;
	effect->Serial = sourceSerial;
	effect->DestSerial = destSerial;
	effect->X = sourceX;
	effect->Y = sourceY;
	effect->Z = sourceZ;
	effect->DestX = destX;
	effect->DestY = destY;
	effect->DestZ = destZ;
	effect->Speed = 5;
	effect->Duration = g_Ticks + 5000;

	effect->Graphic = graphic;

	g_EffectManager.AddEffect(effect);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(CorpseEquipment)
{
	if (g_World == NULL)
		return;

	uint cserial = ReadUInt32BE();

	puchar end = m_Start + m_Size;

	int layer = ReadUInt8();

	while (layer && m_Ptr < end)
	{
		uint serial = ReadUInt32BE();

		CGameItem *obj = g_World->FindWorldItem(serial);

		if (obj != NULL)
			g_World->PutEquipment(obj, cserial, layer);

		layer = ReadUInt8();
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ASCIIPrompt)
{
	if (g_World == NULL)
		return;

	if (g_ConsolePrompt != PT_NONE)
	{
		g_Orion.ConsolePromptCancel();
		g_Orion.CreateTextMessage(TT_SYSTEM, 0xFFFFFFFF, 3, 0, "Previous prompt cancelled.");
	}

	g_ConsolePrompt = PT_ASCII;
	memcpy(&g_LastASCIIPrompt[0], &m_Start[0], 11);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(UnicodePrompt)
{
	if (g_World == NULL)
		return;

	if (g_ConsolePrompt != PT_NONE)
	{
		g_Orion.ConsolePromptCancel();
		g_Orion.CreateTextMessage(TT_SYSTEM, 0xFFFFFFFF, 3, 0, "Previous prompt cancelled.");
	}

	g_ConsolePrompt = PT_UNICODE;
	memcpy(&g_LastUnicodePrompt[0], &m_Start[0], 11);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(CharacterAnimation)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	CGameCharacter *obj = g_World->FindWorldCharacter(serial);

	if (obj != NULL)
	{
		ushort action = ReadUInt16BE();
		Move(1);
		uchar frameCount = ReadUInt8();
		frameCount = 0;
		ushort repeatMode = ReadUInt16BE();
		bool frameDirection = (ReadUInt8() == 0); //true - forward, false - backward
		bool repeat = (ReadUInt8() != 0);
		uchar delay = ReadUInt8();

		obj->SetAnimation((uchar)action, delay, frameCount, (uchar)repeatMode, repeat, frameDirection);
		obj->AnimationFromServer = true;

		LOG("Anim is set\n");
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DisplayQuestArrow)
{
	g_QuestArrow.Timer = g_Ticks + 1000;
	g_QuestArrow.Enabled = (ReadUInt8() != 0);
	g_QuestArrow.X = ReadUInt16BE();
	g_QuestArrow.Y = ReadUInt16BE();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ClientViewRange)
{
	g_ConfigManager.UpdateRange = ReadUInt8();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(KrriosClientSpecial)
{
	CPacketRazorAnswer().Send();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(AssistVersion)
{
	uint version = ReadUInt32BE();

	CPacketAssistVersion(version, g_Orion.ClientVersionText).Send();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(CharacterListNotification)
{
	g_Orion.InitScreen(GS_DELETE);
	g_ConnectionScreen.Type = CST_CHARACTER_LIST;
	g_ConnectionScreen.ConnectionFailed = true;
	g_ConnectionScreen.ErrorCode = ReadUInt8();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(ErrorCode)
{
	uchar code = ReadUInt8();

	g_Orion.InitScreen(GS_DELETE);
	g_ConnectionScreen.Type = CST_GAME_LOGIN;
	g_ConnectionScreen.ErrorCode = code;

	if (code > 7)
		g_ConnectionScreen.ErrorCode = 3;
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(AttackCharacter)
{
	g_LastAttackObject = ReadUInt32BE();

	if (g_LastAttackObject != 0 && g_World != NULL)
	{
		CGameCharacter *obj = g_World->FindWorldCharacter(g_LastAttackObject);

		if (obj != NULL && !obj->MaxHits)
			CPacketStatusRequest(g_LastAttackObject).Send();
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(Season)
{
	uchar season = ReadUInt8();

	if (season >= ST_DESOLATION)
		season = 0; //season % (ST_DESOLATION + 1)

	g_Season = (SEASON_TYPE)season;

	int music = ReadUInt8();

	if (music) //Play sound
	{
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DisplayDeath)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	uint corpseSerial = ReadUInt32BE();

	CGameItem *obj = g_World->FindWorldItem(corpseSerial);

	if (obj != NULL)
		obj->AnimIndex = 0;
	else
		g_CorpseSerialList.push_back(pair<uint, uint>(corpseSerial, g_Ticks + 1000));
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenChat)
{
	uchar newbuf[4] = { 0xf0, 0x00, 0x04, 0xff };
	g_Orion.Send(newbuf, 4);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DisplayClilocString)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	ushort graphic = ReadUInt16BE();
	uchar type = ReadUInt8();
	ushort color = ReadUInt16BE();
	ushort font = g_FontManager.UnicodeFontExists((uchar)ReadUInt16BE());
	uint cliloc = ReadUInt32BE();

	uchar flags = 0;

	if (*m_Start == 0xCC)
		flags = ReadUInt8();

	string name = ReadString(30);

	string affix = "";
	if (*m_Start == 0xCC)
		affix = ReadString(0);

	wstring args((wchar_t*)Ptr);
	//wstring args = ReadUnicodeStringLE(0);
	wstring message = g_ClilocManager.ParseArgumentsToClilocString(cliloc, args);
	//wstring message = ClilocManager->Cliloc(g_Language)->GetW(cliloc);

	if (/*type == ST_BROADCAST || type == ST_SYSTEM ||*/ serial == 0xFFFFFFFF || !serial || name == string("System"))
		g_Orion.CreateUnicodeTextMessage(TT_SYSTEM, serial, (uchar)font, color, message);
	else
	{
		/*if (type == ST_EMOTE)
		{
			color = ConfigManager.EmoteColor;
			str = L"*" + str + L"*";
		}*/

		//if (serial >= 0x40000000) //������ ��� ���������
		{
			CGameObject *obj = g_World->FindWorldObject(serial);

			if (obj != NULL && !obj->Name.length())
			{
				obj->Name = name;

				if (obj->NPC)
					g_GumpManager.UpdateContent(serial, 0, GT_STATUSBAR);
			}
		}

		g_Orion.CreateUnicodeTextMessage(TT_OBJECT, serial, (uchar)font, color, message);
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(MegaCliloc)
{
	if (g_World == NULL)
		return;

	ushort wat = ReadUInt16BE();
	uint serial = ReadUInt32BE();

	CGameObject *obj = g_World->FindWorldObject(serial);

	if (obj == NULL)
		return;

	ushort wat2 = ReadUInt16BE();
	uint testedSerial = ReadUInt32BE();
	wstring message(L"");

	if (!obj->NPC)
		message = L"<basefont color=\"yellow\">";

	puchar end = m_Start + m_Size;
	bool first = true;

	while (m_Ptr < end)
	{
		uint cliloc = ReadUInt32BE();

		if (!cliloc)
			break;

		short len = ReadInt16BE();

		wstring argument = L"";

		if (len > 0)
		{
			argument = wstring((wchar_t*)Ptr, len / 2);
			Ptr += len;
			//wstring argument = ReadUnicodeStringLE(len / 2);
		}

		wstring str = g_ClilocManager.ParseArgumentsToClilocString(cliloc, argument);
		//LOG("Cliloc: argstr=%s\n", ToString(str).c_str());

		if (message.length() && !first)
			message += L"\n";

		message += str;

		if (first)
		{
			if (!obj->NPC)
				message += L"<basefont color=\"#FFFFFFFF\">";

			first = false;
		}

		//LOG("Cliloc: 0x%08X len=%i arg=%s\n", cliloc, len, ToString(argument).c_str());
	}

	//LOG_DUMP((PBYTE)message.c_str(), message.length() * 2);
	obj->ClilocMessage = message;
	//LOG("message=%s\n", ToString(message).c_str());
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(Damage)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	CGameCharacter *character = g_World->FindWorldCharacter(serial);

	if (character != NULL)
	{
		int damage = ReadUInt16BE();

		CTextData *text = new CTextData();
		text->Unicode = false;
		text->Font = 3;
		text->Serial = serial;
		text->Color = 0x0035;
		text->Type = TT_OBJECT;
		text->SetText(std::to_string(damage));
		text->GenerateTexture(0);
		text->X = text->m_Texture.Width / 2;
		int height = text->m_Texture.Height;

		CTextData *head = (CTextData*)character->m_DamageTextControl->Last();

		if (head != NULL)
		{
			height += head->Y;

			if (height > 0)
			{
				if (height > 100)
					height = 0;

				text->Y = height;
			}
		}

		character->m_DamageTextControl->Add(text);
		text->Timer = g_Ticks + DAMAGE_TEXT_NORMAL_DELAY;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(BuffDebuff)
{
	if (g_World == NULL)
		return;

	/*
	df
	00 2e
	00 00 2b b5
	04 04
	00 01
	00 00 00 00 04 04 00 01 00 00 00 00
	01 ca
	00 00 00
	00 10 6a 6b
	00 10 6a 6c
	00 00 00 00
	00 00
	00 00
	00 00



	df
	00 46
	00 00 2b b5
	04 05
	00 01
	00 00 00 00 04 05 00 01 00 00 00 00
	00 85
	00 00 00
	00 10 6a 66
	00 10 56 82
	00 00 00 00
	00 01
	00 00
	09 00 2b 00 20 00 39 00 20 00 41 00 72 00 6d 00 6f 00 72 00 00 00
	00 01
	00 00




	Buffs And Attributes Packet.
	from server
	byte    ID (DF)
	word    Packet Size
	dword    Player Serial
	word    Buff Type
	(
	BonusStr = 0x01, BonusDex = 0x02, BonusInt = 0x03, BonusHits = 0x07, BonusStamina = 0x08, BonusMana = 0x09,
	RegenHits = 0x0A, RegenStam = 0x0B, RegenMana = 0x0C, NightSight = 0x0D, Luck = 0x0E, ReflectPhysical = 0x10,
	EnhancePotions = 0x11, AttackChance = 0x12, DefendChance = 0x13, SpellDamage = 0x14, CastRecovery = 0x15,
	CastSpeed = 0x16, ManaCost = 0x17, ReagentCost = 0x18, WeaponSpeed = 0x19, WeaponDamage = 0x1A,
	PhysicalResistance = 0x1B, FireResistance = 0x1C, ColdResistance = 0x1D, PoisonResistance = 0x1E,
	EnergyResistance = 0x1F, MaxPhysicalResistance = 0x20, MaxFireResistance = 0x21, MaxColdResistance = 0x22,
	MaxPoisonResistance = 0x23, MaxEnergyResistance = 0x24, AmmoCost = 0x26, KarmaLoss = 0x28, 0x3EA+ = buff icons
	)

	word    Buffs Count

	loop    Buffs >>>
	word    Source Type
	(
	0 = Character, 50 = two-handed weapon, 53 = one-handed weapon or spellbook, 54 = shield or ranged weapon,
	55 = shoes, 56 = pants or legs, 58 = helm or hat, 59 = gloves, 60 = ring, 61 = talisman, 62 = necklace or gorget,
	64 = waist, 65 = inner torso, 66 = bracelet, 69 = middle torso, 70 = earring, 71 = arms, 72 = cloak or quiver,
	74 = outer torso, 1000 = spells
	)

	word    0x00
	word    Buff Icon ID (0 for attributes)
	word    Buff Queue Index (Delta Value for attributes)
	dword    0x00
	word    Buff Duration in seconds (0 for attributes)
	byte[3]    0x00
	dword    Buff Title Cliloc
	dword    Buff Secondary Cliloc (0 for attributes)
	dword    Buff Third Cliloc (0 for attributes)
	word    Primary Cliloc Arguments Length (0 for attributes)
	uchar[*]    Primary Cliloc Arguments
	word    Secondary Cliloc Arguments Length (0 for attributes)
	uchar[*]    Secondary Cliloc Arguments
	word    Third Cliloc Arguments Length (0 for attributes)
	uchar[*]    Third Cliloc Arguments
	endloop    Buffs <<<<
	*/



	const int tableCount = 126;

	static const ushort table[tableCount] =
	{
		0x754C, 0x754A, 0x0000, 0x0000, 0x755E, 0x7549, 0x7551, 0x7556, 0x753A, 0x754D,
		0x754E, 0x7565, 0x753B, 0x7543, 0x7544, 0x7546, 0x755C, 0x755F, 0x7566, 0x7554,
		0x7540, 0x7568, 0x754F, 0x7550, 0x7553, 0x753E, 0x755D, 0x7563, 0x7562, 0x753F,
		0x7559, 0x7557, 0x754B, 0x753D, 0x7561, 0x7558, 0x755B, 0x7560, 0x7541, 0x7545,
		0x7552, 0x7569, 0x7548, 0x755A, 0x753C, 0x7547, 0x7567, 0x7542, 0x758A, 0x758B,
		0x758C, 0x758D, 0x0000, 0x758E, 0x094B, 0x094C, 0x094D, 0x094E, 0x094F, 0x0950,
		0x753E, 0x5011, 0x7590, 0x7591, 0x7592, 0x7593, 0x7594, 0x7595, 0x7596, 0x7598,
		0x7599, 0x759B, 0x759C, 0x759E, 0x759F, 0x75A0, 0x75A1, 0x75A3, 0x75A4, 0x75A5,
		0x75A6, 0x75A7, 0x75C0, 0x75C1, 0x75C2, 0x75C3, 0x75C4, 0x75F2, 0x75F3, 0x75F4,
		0x75F5, 0x75F6, 0x75F7, 0x75F8, 0x75F9, 0x75FA, 0x75FB, 0x75FC, 0x75FD, 0x75FE,
		0x75FF, 0x7600, 0x7601, 0x7602, 0x7603, 0x7604, 0x7605, 0x7606, 0x7607, 0x7608,
		0x7609, 0x760A, 0x760B, 0x760C, 0x760D, 0x760E, 0x760F, 0x7610, 0x7611, 0x7612,
		0x7613, 0x7614, 0x7615, 0x75C5, 0x75F6, 0x761B
	};

	const ushort buffIconStart = 0x03E9; //0x03EA ???

	uint serial = ReadUInt32BE();

	ushort iconID = ReadUInt16BE() - buffIconStart;

	if (iconID < tableCount) //buff
	{
		CGumpBuff *gump = (CGumpBuff*)g_GumpManager.UpdateGump(serial, 0, GT_BUFF);

		if (gump != NULL)
		{
			ushort mode = ReadUInt16BE();

			if (mode)
			{
				Move(12);

				ushort timer = ReadUInt16BE();

				Move(3);

				uint titleCliloc = ReadUInt32BE();
				uint descriptionCliloc = ReadUInt32BE();
				uint wtfCliloc = ReadUInt32BE();

				Move(4);

				wstring title = g_ClilocManager.Cliloc(g_Language)->GetW(titleCliloc).c_str();
				wstring description = L"";
				wstring wtf = L"";

				if (descriptionCliloc)
				{
					//wstring arguments((wchar_t*)Ptr);
					wstring arguments = ReadWString(0, false);

					//LOG("Buff arguments: %s\n", ToString(arguments).c_str());
					//LOG("Buff arguments: %s\n", ToString(ClilocManager->ParseArgumentsToClilocString(descriptionCliloc, arguments)).c_str());

					description = L'\n' + g_ClilocManager.ParseArgumentsToClilocString(descriptionCliloc, arguments);

					if (description.length() < 2)
						description = L"";
				}

				if (wtfCliloc)
					wtf = L'\n' + g_ClilocManager.Cliloc(g_Language)->GetW(wtfCliloc).c_str();

				wstring text = L"<left>" + title + description + wtf + L"</left>";

				gump->AddBuff(table[iconID], timer, text);
			}
			else
				gump->DeleteBuff(table[iconID]);
		}
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(SecureTrading)
{
	if (g_World == NULL)
		return;

	uchar type = ReadUInt8();
	uint serial = ReadUInt32BE();

	if (type == 0) //����� ����� ����
	{
		uint id1 = ReadUInt32BE();
		uint id2 = ReadUInt32BE();
		uchar hasName = ReadUInt8();

		CGumpSecureTrading *gump = new CGumpSecureTrading(serial, 0, 0, id1, id2);

		if (hasName && *m_Ptr)
			gump->Text = ReadString(0);

		g_GumpManager.AddGump(gump);
	}
	else if (type == 1) //������
		g_GumpManager.CloseGump(serial, 0, GT_TRADE);
	else if (type == 2) //����������
	{
		CGumpSecureTrading *gump = (CGumpSecureTrading*)g_GumpManager.UpdateGump(serial, 0, GT_TRADE);

		if (gump != NULL)
		{
			uint id1 = ReadUInt32BE();
			uint id2 = ReadUInt32BE();

			gump->StateMy = id1 != 0;
			gump->StateOpponent = id2 != 0;
			gump->WantRedraw = true;
		}
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(TextEntryDialog)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	uchar parentID = ReadUInt8();
	uchar buttonID = ReadUInt8();

	short textLen = ReadInt16BE();
	string text = ReadString(textLen);

	bool haveCancel = !ReadUInt8();
	uchar variant = ReadUInt8();
	int maxLength = ReadUInt32BE();

	short descLen = ReadInt16BE();
	string desc = ReadString(descLen);

	CGumpTextEntryDialog *gump = new CGumpTextEntryDialog(serial, 143, 172, variant, maxLength, text, desc);
	gump->NoClose = haveCancel;
	gump->ParentID = parentID;
	gump->ButtonID = buttonID;

	g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenMenuGump)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	uint id = ReadUInt16BE();

	uchar nameLen = ReadUInt8();
	string name = ReadString(nameLen);

	uchar count = ReadUInt8();

	if (unpack16(m_Ptr)) //menu
	{
		CGumpMenu *gump = new CGumpMenu(serial, id, 0, 0);

		gump->Add(new CGUIGumppic(0x0910, 0, 0));

		CGUIText *text = (CGUIText*)gump->Add(new CGUIText(0x0386, 39, 18));
		text->CreateTextureA(1, name, 200, TS_LEFT, UOFONT_FIXED);

		CGUIHTMLGump *htmlGump = (CGUIHTMLGump*)gump->Add(new CGUIHTMLGump(CGumpMenu::ID_GM_HTMLGUMP, 0, 40, 42, 217, 49, false, true));
		htmlGump->Initalize(true);

		htmlGump->Add(new CGUIShader(g_ColorizerShader, true));

		int posX = 0;

		IFOR(i, 0, count)
		{
			ushort graphic = ReadUInt16BE();
			ushort color = ReadUInt16BE();

			nameLen = ReadUInt8();
			name = ReadString(nameLen);

			WISP_GEOMETRY::CSize size = g_Orion.GetArtDimension(graphic + 0x4000);

			if (size.Width && size.Height)
			{
				int posY = size.Height;

				if (posY >= 47)
					posY = 0;
				else
					posY = ((47 - posY) / 2);

				CGUIMenuObject *menuObject = (CGUIMenuObject*)htmlGump->Add(new CGUIMenuObject(i + 1, graphic, color, posX, posY, name));

				posX += size.Width;
			}
		}

		htmlGump->Add(new CGUIShader(g_ColorizerShader, false));

		htmlGump->CalculateDataSize();

		gump->m_TextObject = (CGUIText*)gump->Add(new CGUIText(0x0386, 42, 105));
		//gump->m_TextObject->CreateTextureA(1, name, 200, TS_LEFT, UOFONT_FIXED); //�� ������ ����� ��������� ������

		g_GumpManager.AddGump(gump);
	}
	else //gray menu
	{
		int x = (g_OrionWindow.Size.Width / 2) - 200;
		int y = (g_OrionWindow.Size.Height / 2) - ((121 + (count * 21)) / 2);

		CGumpGrayMenu *gump = new CGumpGrayMenu(serial, id, x, y);

		CGUIResizepic *background = (CGUIResizepic*)gump->Add(new CGUIResizepic(0, 0x13EC, 0, 0, 400, 11111));

		CGUIText *text = (CGUIText*)gump->Add(new CGUIText(0x0386, 20, 16));
		text->CreateTextureA(1, name);

		int offsetY = 51;
		int gumpHeight = 121;

		IFOR(i, 0, count)
		{
			Move(4);

			nameLen = ReadUInt8();
			name = ReadString(nameLen);

			gump->Add(new CGUIRadio(i + 1, 0x138A, 0x138B, 0x138A, 20, offsetY)); //Button

			offsetY += 2;

			text = (CGUIText*)gump->Add(new CGUIText(0x0386, 50, offsetY));
			text->CreateTextureA(1, name);

			int addHeight = text->m_Texture.Height;

			if (addHeight < 21)
				addHeight = 21;

			offsetY += addHeight;
			gumpHeight += addHeight;
		}

		offsetY += 5;

		gump->Add(new CGUIButton(CGumpGrayMenu::ID_GGM_CANCEL, 0x1450, 0x1450, 0x1451, 70, offsetY)); //CANCEL
		gump->Add(new CGUIButton(CGumpGrayMenu::ID_GGM_CONTINUE, 0x13B2, 0x13B2, 0x13B3, 200, offsetY)); //CONTINUE

		background->Height = gumpHeight;

		g_GumpManager.AddGump(gump);
	}
}
//----------------------------------------------------------------------------------
void CPacketManager::AddHTMLGumps(class CGump *gump, vector<HTMLGumpDataInfo> &list)
{
	IFOR(i, 0, (int)list.size())
	{
		HTMLGumpDataInfo &data = list[i];

		CGUIHTMLGump *htmlGump = (CGUIHTMLGump*)gump->Add(new CGUIHTMLGump(data.TextID, 0x0BB8, data.X, data.Y, data.Width, data.Height, data.HaveBackground, data.HaveScrollbar));
		htmlGump->DrawOnly = (data.HaveScrollbar == 0);

		int width = htmlGump->Width;

		if (data.HaveScrollbar)
			width -= 16;

		uint htmlColor = 0xFFFFFFFF;

		if (!data.HaveBackground)
		{
			data.Color = 0xFFFF;

			if (!data.HaveScrollbar)
				htmlColor = 0x010101FF;
		}
		else
		{
			width -= 9;
			htmlColor = 0x010101FF;
		}

		CGUIHTMLText *htmlText = (CGUIHTMLText*)htmlGump->Add(new CGUIHTMLText(data.TextID, 0, data.Color, 0, 0, width, TS_LEFT, /*UOFONT_BLACK_BORDER*/0, htmlColor));

		if (data.IsXMF)
		{
			htmlText->Text = g_ClilocManager.Cliloc(g_Language)->GetW(data.TextID);
			htmlText->CreateTexture();
			htmlGump->CalculateDataSize();
		}
	}

	list.clear();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenGump)
{
	if (g_World == NULL)
		return;

	vector<HTMLGumpDataInfo> htmlGumlList;

	//TPRINT("Gump dump::\n");
	//TDUMP(buf, size);

	uint serial = ReadUInt32BE();
	uint id = ReadUInt32BE();
	int x = ReadInt32BE();
	int y = ReadInt32BE();

	CGumpGeneric *gump = new CGumpGeneric(serial, x, y, id);

	puchar p = m_Start + 21;
	puchar e = p;

	int commandsLen = unpack16(m_Start + 19);
	puchar end = p + commandsLen;

	while (p < end)
	{
		while (p < end && *p && *p != '{')
			p++;

		e = p + 1;

		while (e < end && *e && *e == ' ')
			e++;

		char lowc[20] = { 0 };

		int eLen = strlen((char*)e);
		eLen = (eLen > 19 ? 20 : eLen);
		memcpy(&lowc[0], &e[0], eLen);
		lowc[19] = 0;
		_strlwr(lowc);

		CBaseGUI *go = NULL;

		//TPRINT("\tlwr.token=%s\n", lowc);
		if (!memcmp(lowc, "nodispose", 9))
			e += 10;
		else if (!memcmp(lowc, "nomove", 6))
		{
			gump->NoMove = true;
			e += 7;
		}
		else if (!memcmp(lowc, "noclose", 7))
		{
			gump->NoClose = true;
			e += 7;
		}
		else if (!memcmp(lowc, "page", 4))
		{
			e += 5;
			int page = 0;
			sscanf((char*)e, "%d", &page);

			AddHTMLGumps(gump, htmlGumlList);

			go = new CGUIPage(page);
		}
		else if (!memcmp(lowc, "group", 5))
		{
			e += 6;
			int group = 0;
			sscanf((char*)e, "%d", &group);

			go = new CGUIGroup(group);
		}
		else if (!memcmp(lowc, "endgroup", 8))
		{
			e += 9;

			go = new CGUIGroup(0);
		}
		else if (!memcmp(lowc, "resizepic", 9))
		{
			e += 10;
			int x = 0, y = 0, w = 0, h = 0, graphic = 0;
			sscanf((char*)e, "%d %d %d %d %d", &x, &y, &graphic, &w, &h);

			go = new CGUIResizepic(0, graphic, x, y, w, h);
		}
		else if (!memcmp(lowc, "checkertrans", 12))
		{
			e += 13;
			int x = 0, y = 0, w = 0, h = 0;
			sscanf((char*)e, "%d %d %d %d", &x, &y, &w, &h);

			go = new CGUIChecktrans(x, y, w, h);
		}
		else if (!memcmp(lowc, "buttontileart", 13))
		{
			e += 14;
			int x = 0, y = 0, action = 0, topage = 0, number = 0, up = 0, down = 0, tid = 0, tcolor = 0, tx = 0, ty = 0;
			sscanf((char*)e, "%d %d %d %d %d %d %d %d %d %d %d", &x, &y, &up, &down, &action, &topage, &number, &tid, &tcolor, &tx, &ty);

			if (action == 1)
				topage = -1;

			go = new CGUIButtonTileart(number, up, up, down, x, y, tid, tcolor, tx, ty);

			((CGUIButton*)go)->ToPage = topage;
		}
		else if (!memcmp(lowc, "button", 6))
		{
			e += 7;
			int x = 0, y = 0, action = 0, topage = 0, number = 0, up = 0, down = 0;

			sscanf((char*)e, "%i %i %i %i %i %i %i", &x, &y, &up, &down, &action, &topage, &number);

			if (action == 1)
				topage = -1;

			go = new CGUIButton(number, up, up, down, x, y);

			((CGUIButton*)go)->ToPage = topage;
		}
		else if (!memcmp(lowc, "checkbox", 8))
		{
			e += 9;
			int x = 0, y = 0, state = 0, number = 0, up = 0, down = 0;
			sscanf((char*)e, "%d %d %d %d %d %d", &x, &y, &up, &down, &state, &number);

			go = new CGUICheckbox(number, up, down, up, x, y);

			((CGUICheckbox*)go)->Checked = state != 0;
		}
		else if (!memcmp(lowc, "radio", 5))
		{
			e += 6;
			int x = 0, y = 0, state = 0, number = 0, up = 0, down = 0;
			sscanf((char*)e, "%d %d %d %d %d %d", &x, &y, &up, &down, &state, &number);

			go = new CGUIRadio(number, up, down, up, x, y);

			((CGUIRadio*)go)->Checked = state != 0;
		}
		else if (!memcmp(lowc, "croppedtext", 11))
		{
			e += 12;
			int x = 0, y = 0, w = 0, h = 0, number = 0, color = 0;
			sscanf((char*)e, "%d %d %d %d %d %d", &x, &y, &w, &h, &color, &number);

			if (color)
				color++;

			go = new CGUIGenericText(number, color, x, y, w);
			go->DrawOnly = true;
		}
		else if (!memcmp(lowc, "textentrylimited", 16))
		{
			e += 17;
			int x = 0, y = 0, w = 0, h = 0, index = 0, number = 0, length = 0, color = 0;
			sscanf((char*)e, "%d %d %d %d %d %d %d %d", &x, &y, &w, &h, &color, &index, &number, &length);

			if (color)
				color++;

			go = new CGUIGenericTextEntry(number, index, color, x, y, w, length);
		}
		else if (!memcmp(lowc, "textentry", 9))
		{
			e += 10;
			int x = 0, y = 0, w = 0, h = 0, index = 0, number = 0, color = 0;
			sscanf((char*)e, "%d %d %d %d %d %d %d", &x, &y, &w, &h, &color, &index, &number);

			if (color)
				color++;

			go = new CGUIGenericTextEntry(number, index, color, x, y);
		}
		else if (!memcmp(lowc, "text", 4))
		{
			e += 5;
			int x = 0, y = 0, n = 0, color = 0;
			sscanf((char*)e, "%d %d %d %d", &x, &y, &color, &n);

			if (color)
				color++;

			go = new CGUIGenericText(n, color, x, y);
			go->DrawOnly = true;
		}
		else if (!memcmp(lowc, "tilepichue", 10))
		{
			e += 11;
			int x = 0, y = 0, graphic = 0, color = 0;
			sscanf((char*)e, "%d %d %d %d", &x, &y, &graphic, &color);

			go = new CGUITilepic(graphic, color, x, y);
		}
		else if (!memcmp(lowc, "tilepic", 7))
		{
			e += 8;
			int x = 0, y = 0, graphic = 0;
			sscanf((char*)e, "%d %d %d", &x, &y, &graphic);

			go = new CGUITilepic(graphic, 0, x, y);
		}
		else if (!memcmp(lowc, "gumppictiled", 12))
		{
			e += 13;
			int x = 0, y = 0, w = 0, h = 0, graphic = 0;
			sscanf((char*)e, "%d %d %d %d %d", &x, &y, &w, &h, &graphic);

			go = new CGUIGumppicTiled(graphic, x, y, w, h);
		}
		else if (!memcmp(lowc, "gumppic", 7))
		{
			e += 8;
			int x = 0, y = 0, graphic = 0, color = 0;
			sscanf((char*)e, "%d %d %d", &x, &y, &graphic);

			char bufptr[20] = { 0 };
			sprintf(bufptr, "%d %d %d", x, y, graphic);

			int curlen = strlen(bufptr);
			while (e + curlen < end && e[curlen] == ' ')
				curlen++;

			if (e[curlen] != '}')
				sscanf((char*)(e + strlen(bufptr) + 5), "%d", &color);

			go = new CGUIGumppic(graphic, x, y);
			go->Color = color;
		}
		else if (!memcmp(lowc, "xmfhtmlgump", 11))
		{
			HTMLGumpDataInfo htmlInfo = { 0 };
			htmlInfo.IsXMF = true;

			if (!memcmp(lowc, "xmfhtmlgumpcolor", 16))
			{
				e += 17;
				sscanf((char*)e, "%d %d %d %d %d %d %d %d", &htmlInfo.X, &htmlInfo.Y, &htmlInfo.Width, &htmlInfo.Height, &htmlInfo.TextID, &htmlInfo.HaveBackground, &htmlInfo.HaveScrollbar, &htmlInfo.Color);
			}
			else
			{
				e += 12;
				sscanf((char*)e, "%d %d %d %d %d %d %d", &htmlInfo.X, &htmlInfo.Y, &htmlInfo.Width, &htmlInfo.Height, &htmlInfo.TextID, &htmlInfo.HaveBackground, &htmlInfo.HaveScrollbar);
				htmlInfo.Color = 0;
			}

			htmlGumlList.push_back(htmlInfo);
		}
		else if (!memcmp(lowc, "htmlgump", 8))
		{
			e += 9;

			HTMLGumpDataInfo htmlInfo = { 0 };

			sscanf((char*)e, "%d %d %d %d %d %d %d", &htmlInfo.X, &htmlInfo.Y, &htmlInfo.Width, &htmlInfo.Height, &htmlInfo.TextID, &htmlInfo.HaveBackground, &htmlInfo.HaveScrollbar);

			htmlGumlList.push_back(htmlInfo);
		}
		/*else if (!memcmp(lowc, "xfmhtmltok", 10))
		{
		e += 11;
		int x = 0, y = 0, w = 0, h = 0, background = 0, scrollbar = 0, clilocID = 0;
		sscanf((char*)e, "%d %d %d %d %d %d", &x, &y, &w, &h, &background, &scrollbar, &clilocID);

		go = new TGumpXFMHTMLToken(clilocID, x, y, w, h, background, scrollbar);
		}*/
		/*else if (!memcmp(lowc, "tooltip", 7))
		{
			e += 8;
			int clilocID = 0;
			sscanf((char*)e, "%d", &clilocID);

			go = new TGumpTooltip(clilocID);
		}
		else if (!memcmp(lowc, "mastergump", 10))
		{
			e += 11;
			int index = 0;
			sscanf((char*)e, "%d", &index);

			go = new TGumpMasterGump(index);
		}*/

		if (go != NULL)
			gump->Add(go);

		while (e < end && *e && *e != '}')
			e++;

		p = e + 1;
	}

	AddHTMLGumps(gump, htmlGumlList);

	m_Ptr = m_Start + 21 + commandsLen;

	short textLinesCount = ReadInt16BE();

	IFOR(i, 0, textLinesCount)
	{
		int linelen = ReadInt16BE();

		if (linelen)
			gump->AddText(i, ReadWString(linelen, true));
	}

	g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenCompressedGump)
{
	if (g_World == NULL)
		return;

	uint senderID = ReadUInt32BE();
	uint gumpID = ReadUInt32BE();
	uint x = ReadUInt32BE();
	uint y = ReadUInt32BE();
	uLongf cLen = ReadUInt32BE() - 4; //Compressed Length (4 == sizeof(DecompressedLen)
	uLongf dLen = ReadUInt32BE(); //Decompressed Length

	if (cLen < 1)
	{
		LOG("CLen=%d\nServer Sends bad Compressed Gumpdata!\n", cLen);

		return;
	}
	else if ((int)(28 + cLen) > m_Size)
	{
		LOG("Server Sends bad Compressed Gumpdata!\n");

		return;
	}

	// Layout data.....
	UCHAR_LIST decLayoutData(dLen);
	LOG("Gump layout:\n\tSenderID=0x%08X\n\tGumpID=0x%08X\n\tCLen=%d\n\tDLen=%d\nDecompressing layout gump data...\n", senderID, gumpID, cLen, dLen);

	int z_err = uncompress(&decLayoutData[0], &dLen, m_Ptr, cLen);

	if (z_err != Z_OK)
	{
		LOG("Decompress layout gump error %d\n", z_err);

		return;
	}

	LOG("Layout gump data decompressed!\n");
	// Text data.....

	Move(cLen);

	uint linesCount = ReadUInt32BE(); //Text lines count
	uint cTLen = 0;
	uLongf dTLen = 0;
	UCHAR_LIST gumpDecText;

	if (linesCount > 0)
	{
		cTLen = ReadUInt32BE(); //Compressed lines length
		dTLen = ReadUInt32BE(); //Decompressed lines length

		gumpDecText.resize(dTLen);

		LOG("Decompressing text gump data...\n");

		z_err = uncompress(&gumpDecText[0], &dTLen, m_Ptr, cTLen);

		if (z_err != Z_OK)
		{
			LOG("Decompress text gump error %d\n", z_err);

			return;
		}

		LOG("Text gump data decompressed!\nGump text lines:\n\tLinesCount=%d\n\tCTLen=%d\n\tDTLen=%d\n", linesCount, cTLen, dTLen);
	}

	int newsize = 21 + dLen + 2 + dTLen;

	UCHAR_LIST newbufData(newsize);
	puchar newbuf = &newbufData[0];
	newbuf[0] = 0xb0;
	pack16(newbuf + 1, newsize);
	pack32(newbuf + 3, senderID);
	pack32(newbuf + 7, gumpID);
	pack32(newbuf + 11, x);
	pack32(newbuf + 15, y);
	pack16(newbuf + 19, (ushort)dLen);
	memcpy(newbuf + 21, &decLayoutData[0], dLen);
	pack16(newbuf + 21 + dLen, (ushort)linesCount);

	if (linesCount > 0)
		memcpy(newbuf + 23 + dLen, &gumpDecText[0], dTLen);
	else
		newbuf[newsize - 1] = 0x00;

	LOG("Gump decompressed! newsize=%d\n", newsize);

	m_Size = newsize;
	m_Start = newbuf;
	m_End = m_Start + m_Size;

	OnPacket();
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(DyeData)
{
	uint serial = ReadUInt32BE();
	Move(2);
	ushort graphic = ReadUInt16BE();

	WISP_GEOMETRY::CSize gumpSize = g_Orion.GetGumpDimension(0x0906);

	int x = (WORD)((g_OrionWindow.Size.Width / 2) - (gumpSize.Width / 2));
	int y = (WORD)((g_OrionWindow.Size.Height / 2) - (gumpSize.Height / 2));

	CGumpDye *gump = new CGumpDye(serial, x, y, graphic);

	g_GumpManager.AddGump(gump);
}
//---------------------------------------------------------------------------
PACKET_HANDLER(DisplayMap)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	ushort gumpid = ReadUInt16BE();

	//TPRINT("gumpid = 0x%04X\n", gumpid);

	ushort startX = ReadUInt16BE();
	ushort startY = ReadUInt16BE();
	ushort endX = ReadUInt16BE();
	ushort endY = ReadUInt16BE();
	ushort width = ReadUInt16BE();
	ushort height = ReadUInt16BE();

	CGumpMap *gump = new CGumpMap(serial, gumpid, startX, startY, endX, endY, width, height);

	if (*m_Start == 0xF5)
		g_MultiMap.LoadFacet(gump, gump->m_Texture, ReadUInt16BE());
	else
		g_MultiMap.LoadMap(gump, gump->m_Texture);

	//TPRINT("GumpX=%d GumpY=%d\n", startX, startY);
	//TPRINT("GumpTX=%d GumpTY=%d\n", endX, endY);
	//TPRINT("GumpW=%d GumpH=%d\n", width, height);

	g_GumpManager.AddGump(gump);

	CGameItem *obj = g_World->FindWorldItem(serial);

	if (obj != NULL)
		obj->Opened = true;
}
//---------------------------------------------------------------------------
PACKET_HANDLER(MapData)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	CGumpMap *gump = (CGumpMap*)g_GumpManager.UpdateGump(serial, 0, GT_MAP);

	if (gump != NULL && gump->m_DataBox != NULL)
	{
		switch ((MAP_MESSAGE)ReadUInt8()) //Action
		{
			case MM_ADD: //Add Pin
			{
				Move(1);

				short x = ReadUInt16BE();
				short y = ReadUInt16BE();

				gump->m_DataBox->Add(new CGUIGumppic(0x139B, x, y));
				gump->WantRedraw = true;

				break;
			}
			case MM_INSERT: //Insert New Pin
			{
				break;
			}
			case MM_MOVE: //Change Pin
			{
				break;
			}
			case MM_REMOVE: //Remove Pin
			{
				break;
			}
			case MM_CLEAR: //Clear Pins
			{
				gump->m_DataBox->Clear();
				gump->WantRedraw = true;

				break;
			}
			case MM_EDIT_RESPONSE: //Reply From Server to Action 6 (Plotting request)
			{
				gump->PlotState = ReadUInt8();

				break;
			}
			default:
				break;
		}
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(TipWindow)
{
	uchar flag = ReadUInt8();

	if (flag != 1) //1 - ignore
	{
		uint serial = ReadUInt32BE();
		short len = ReadInt16BE();

		string str = ReadString(len);

		int x = 20;
		int y = 20;

		if (!flag)
		{
			x = 200;
			y = 100;
		}

		CGumpTip *gump = new CGumpTip(serial, x, y, str, flag != 0);

		g_GumpManager.AddGump(gump);
	}
}
//---------------------------------------------------------------------------
PACKET_HANDLER(CharacterProfile)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	wstring topText = ToWString(ReadString(0));

	wstring bottomText = ReadWString(0);
	wstring dataText = ReadWString(0);

	CGumpProfile *gump = new CGumpProfile(serial, 170, 90, topText, bottomText, dataText);

	g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(BulletinBoardData)
{
	if (g_World == NULL)
		return;

	switch (ReadUInt8())
	{
		case 0: //Open board
		{
			uint serial = ReadUInt32BE();

			CGameItem *item = g_World->FindWorldItem(serial);

			if (item != NULL)
			{
				CGumpBulletinBoard *bbGump = (CGumpBulletinBoard*)g_GumpManager.UpdateGump(serial, 0, GT_BULLETIN_BOARD);

				if (bbGump != NULL)
				{
					CBaseGUI *bbItem = (CBaseGUI*)bbGump->m_HTMLGump->m_Items;

					while (bbItem != NULL)
					{
						CBaseGUI *bbNext = (CBaseGUI*)bbItem->m_Next;

						if (bbItem->Type == GOT_BB_OBJECT)
							bbGump->m_HTMLGump->Delete(bbItem);

						bbItem = bbNext;
					}

					bbGump->m_HTMLGump->CalculateDataSize();
				}

				item->Opened = true;
			}

			string str((char*)Ptr);

			int x = (g_OrionWindow.Size.Width / 2) - 245;
			int y = (g_OrionWindow.Size.Height / 2) - 205;

			CGumpBulletinBoard *gump = new CGumpBulletinBoard(serial, x, y, str);

			g_GumpManager.AddGump(gump);

			break;
		}
		case 1: //Summary message
		{
			uint boardSerial = ReadUInt32BE();

			CGumpBulletinBoard *gump = (CGumpBulletinBoard*)g_GumpManager.GetGump(boardSerial, 0, GT_BULLETIN_BOARD);

			if (gump != NULL)
			{
				uint serial = ReadUInt32BE();
				uint parentID = ReadUInt32BE();

				//poster
				int len = ReadUInt8();
				string text = ReadString(len) + " - ";

				//subject
				len = ReadUInt8();
				text += ReadString(len) + " - ";

				//data time
				len = ReadUInt8();
				text += ReadString(len);

				int posY = (gump->m_HTMLGump->GetItemsCount() - 5) * 18;

				if (posY < 0)
					posY = 0;

				gump->m_HTMLGump->Add(new CGUIBulletinBoardObject(serial, 0, posY, text));
				gump->m_HTMLGump->CalculateDataSize();
			}

			break;
		}
		case 2: //Message
		{
			uint boardSerial = ReadUInt32BE();

			CGumpBulletinBoard *gump = (CGumpBulletinBoard*)g_GumpManager.GetGump(boardSerial, 0, GT_BULLETIN_BOARD);

			if (gump != NULL)
			{
				uint serial = ReadUInt32BE();

				//poster
				int len = ReadUInt8();
				string poster = ReadString(len);

				//subject
				len = ReadUInt8();
				string subject = ReadString(len);

				//data time
				len = ReadUInt8();
				string dataTime = ReadString(len);

				Move(5);

				uchar lines = ReadUInt8();
				string data = "";

				IFOR(i, 0, lines)
				{
					uchar linelen = ReadUInt8();

					if (data.length())
						data += "\n";

					data += ReadString(linelen);
				}

				uchar variant = 1 + (int)(poster == g_Player->Name);
				g_GumpManager.AddGump(new CGumpBulletinBoardItem(serial, 0, 0, variant, boardSerial, poster, subject, dataTime, data));
			}

			break;
		}
		default:
			break;
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenBook)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	uchar flags = ReadUInt8();
	Move(1);
	WORD pageCount = ReadUInt16BE();

	CGumpBook *gump = new CGumpBook(serial, 0, 0, pageCount, flags != 0, false);

	gump->m_EntryTitle->m_Entry.SetText(ReadString(60));
	gump->m_EntryAuthor->m_Entry.SetText(ReadString(30));

	g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(OpenBookNew)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();
	uchar flag1 = ReadUInt8();
	uchar flag2 = ReadUInt8();
	ushort pageCount = ReadUInt16BE();

	CGumpBook *gump = new CGumpBook(serial, 0, 0, pageCount, (flag1 + flag2) != 0, true);

	int authorLen = ReadUInt16BE();

	puchar ptr = m_Start + 13;

	if (authorLen > 0)
	{
		wchar_t *author = new wchar_t[authorLen];
		*author = 0;
		puchar aptr = (puchar)author;
		//
		ptr += (authorLen * 2);
		//
		gump->m_EntryAuthor->m_Entry.SetText(author);

		delete author;
	}
	else
		ptr += 2;

	int titleLen = unpack16(ptr);
	ptr += 2;

	if (titleLen > 0)
	{
		wchar_t *title = new wchar_t[titleLen];
		*title = 0;
		puchar tptr = (puchar)title;
		//
		gump->m_EntryTitle->m_Entry.SetText(title);

		delete title;
	}

	g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(BookData)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGumpBook *gump = (CGumpBook*)g_GumpManager.GetGump(serial, 0, GT_BOOK);

	if (gump != NULL)
	{
		ushort pageCount = ReadUInt16BE();

		IFOR(i, 0, pageCount)
		{
			ushort page = ReadUInt16BE();

			if (page >= gump->PageCount)
				continue;

			ushort lineCount = ReadUInt16BE();

			if (!gump->Unicode)
			{
				string str = "";

				IFOR(j, 0, lineCount)
				{
					if (j)
						str += '\n';

					str += ReadString(0);
				}

				gump->SetPageData(page, str);

				//TPRINT("BookPageData[%i] = %s\n", page, str.c_str());
			}
			else
			{
				wstring str = L"";

				IFOR(j, 0, lineCount)
				{
					if (j)
						str += L'\n';

					str += ReadWString(0);
				}

				gump->SetPageData(page, str);
			}
		}
	}
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(BuyList)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameItem *container = g_World->FindWorldItem(serial);

	if (container == NULL)
	{
		LOG("Error!!! Buy container is not found!!!\n");
		return;
	}

	uint vendorSerial = container->Container;

	CGameCharacter *vendor = g_World->FindWorldCharacter(vendorSerial);

	if (vendor == NULL)
	{
		LOG("Error!!! Buy vendor is not found!!!\n");
		return;
	}

	CGumpShop *gump = (CGumpShop*)g_GumpManager.GetGump(vendorSerial, 0, GT_SHOP);

	if (gump != NULL && (gump->Serial != vendorSerial || !gump->IsBuyGump))
	{
		g_GumpManager.RemoveGump(gump);
		gump = NULL;
	}

	if (gump == NULL)
	{
		gump = new CGumpShop(vendorSerial, true, 150, 5);
		g_GumpManager.AddGump(gump);
	}

	gump->WantRedraw = true;

	if (container->Layer == OL_BUY_RESTOCK || container->Layer == OL_BUY)
	{
		uchar count = ReadUInt8();

		int i = (g_Orion.InverseBuylist ? count - 1 : 0);
		int end = (g_Orion.InverseBuylist ? -1 : count);
		int add = (g_Orion.InverseBuylist ? -1 : 1);

		CGameItem *item = (CGameItem*)container->m_Items;

		if (g_Orion.InverseBuylist)
		{
			while (item != NULL && item->m_Next != NULL)
				item = (CGameItem*)item->m_Next;
		}

		CGUIHTMLGump *htmlGump = gump->m_ItemList[0];

		int currentY = 0;

		QFOR(shopItem, htmlGump->m_Items, CBaseGUI*)
		{
			if (shopItem->Type == GOT_SHOPITEM)
				currentY += shopItem->GetSize().Height;
		}

		for (; i != end; i += add)
		{
			if (item == NULL)
			{
				LOG("Error!!! Buy item is not found!!!\n");
				break;
			}

			uint price = ReadUInt32BE();

			uchar nameLen = ReadUInt8();
			string name = ReadString(nameLen);

			CGUIShopItem *shopItem = (CGUIShopItem*)htmlGump->Add(new CGUIShopItem(item->Serial, item->Graphic, item->Color, item->Count, price, name, 0, currentY));

			if (!currentY)
			{
				shopItem->Selected = true;
				shopItem->CreateNameText();
			}

			currentY += shopItem->GetSize().Height;

			if (g_Orion.InverseBuylist)
				item = (CGameItem*)item->m_Prev;
			else
				item = (CGameItem*)item->m_Next;
		}

		htmlGump->CalculateDataSize();
	}
	else
		LOG("Unknown layer for buy container!!!\n");
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(SellList)
{
	if (g_World == NULL)
		return;

	uint serial = ReadUInt32BE();

	CGameCharacter *vendor = g_World->FindWorldCharacter(serial);

	if (vendor == NULL)
	{
		LOG("Error!!! Sell vendor is not found!!!\n");
		return;
	}

	ushort count = ReadUInt16BE();

	if (!count)
	{
		LOG("Error!!! Sell list is empty.\n");
		return;
	}

	g_GumpManager.CloseGump(0, 0, GT_SHOP);

	CGumpShop *gump = new CGumpShop(serial, false, 100, 0);
	CGUIHTMLGump *htmlGump = gump->m_ItemList[0];

	int currentY = 0;

	IFOR(i, 0, count)
	{
		uint itemSerial = ReadUInt32BE();
		ushort graphic = ReadUInt16BE();
		ushort color = ReadUInt16BE();
		ushort count = ReadUInt16BE();
		ushort price = ReadUInt16BE();
		int nameLen = ReadInt16BE();
		string name = ReadString(0);

		CGUIShopItem *shopItem = (CGUIShopItem*)htmlGump->Add(new CGUIShopItem(itemSerial, graphic, color, count, price, name, 0, currentY));

		if (!i)
		{
			shopItem->Selected = true;
			shopItem->CreateNameText();
		}

		currentY += shopItem->GetSize().Height;
	}

	htmlGump->CalculateDataSize();

	g_GumpManager.AddGump(gump);
}
//----------------------------------------------------------------------------------
PACKET_HANDLER(BuyReply)
{
	uint serial = ReadUInt32BE();
	uchar flag = ReadUInt8();

	if (!flag) //Close shop gump
		g_GumpManager.CloseGump(serial, 0, GT_SHOP);
}
//----------------------------------------------------------------------------------
