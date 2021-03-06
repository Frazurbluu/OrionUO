/***********************************************************************************
**
** GumpScreenCreateCharacter.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef GUMPSCREENCREATECHARACTER_H
#define GUMPSCREENCREATECHARACTER_H
//----------------------------------------------------------------------------------
#include "Gump.h"
//----------------------------------------------------------------------------------
class CGumpScreenCreateCharacter : public CGump
{
private:
	//!�������������� ������
	static const int ID_CCS_QUIT = 1;
	static const int ID_CCS_ARROW_PREV = 2;
	static const int ID_CCS_ARROW_NEXT = 3;
	static const int ID_CCS_NAME_FIELD = 4;
	static const int ID_CCS_MALE_BUTTON = 5;
	static const int ID_CCS_FEMALE_BUTTON = 6;
	static const int ID_CCS_HUMAN_RACE_BUTTON = 7;
	static const int ID_CCS_ELVEN_RACE_BUTTON = 8;
	static const int ID_CCS_GARGOYLE_RACE_BUTTON = 9;

	static const int ID_CCS_SKIN_TONE = 10;
	static const int ID_CCS_SHIRT_COLOR = 11;
	static const int ID_CCS_SKIRT_OR_PANTS_COLOR = 12;
	static const int ID_CCS_HAIR_COLOR = 13;
	static const int ID_CCS_FACIAL_HAIR_COLOR = 14;

	static const int ID_CCS_HAIR_STYLE = 20;
	static const int ID_CCS_FACIAL_HAIR_STYLE = 40;
	static const int ID_CCS_COLOR_RANGE = 100;

	enum
	{
		CCSID_HAIR_STYLE = 1,
		CCSID_FACIAL_HAIR_STYLE,
		CCSID_SKIN_TONE = 1,
		CCSID_SHIRT_COLOR,
		CCSID_SKIRT_OR_PANTS_COLOR,
		CCSID_HAIR_COLOR,
		CCSID_FACIAL_HAIR_COLOR
	};

	CGUIGumppic *m_Hair;
	CGUIGumppic *m_FacialHair;

public:
	CGumpScreenCreateCharacter();
	virtual ~CGumpScreenCreateCharacter();

	virtual void UpdateContent();

	virtual void InitToolTip();



	virtual void OnButton(const uint &serial);
	virtual void OnTextEntry(const uint &serial);
	GUMP_COMBOBOX_SELECTION_EVENT_H;
 };
 //----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
