/***********************************************************************************
**
** GameObject.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "GameObject.h"
#include "GameWorld.h"
#include "GameEffect.h"
#include "../Managers/ClilocManager.h"
#include "../Managers/MouseManager.h"
#include "../Managers/FontsManager.h"
#include "../Managers/ConfigManager.h"
#include "../Managers/ColorManager.h"
#include "../SelectedObject.h"
#include "../OrionUO.h"
#include "../ServerList.h"
#include "../OrionWindow.h"
//----------------------------------------------------------------------------------
CGameObject::CGameObject(const uint &serial)
: CRenderStaticObject(ROT_GAME_OBJECT, serial, 0, 0, 0, 0, 0), m_Container(0xFFFFFFFF),
m_MapIndex(0), m_Count(0), m_Flags(0), m_Name(""), m_NPC(false), m_Clicked(false),
m_Effects(NULL), m_AnimIndex(0), m_YouSeeJournalPrefix(false),
m_LastAnimationChangeTime(GetTickCount()), m_ClilocMessage(L"")
{
	memset(&m_FrameInfo, 0, sizeof(DRAW_FRAME_INFORMATION));

#if UO_DEBUG_INFO!=0
	g_GameObjectsCount++;
#endif //UO_DEBUG_INFO!=0
}
//----------------------------------------------------------------------------------
CGameObject::~CGameObject()
{
	if (m_Effects != NULL)
	{
		delete m_Effects;
		m_Effects = NULL;
	}
	
	m_Next = NULL;
	m_Prev = NULL;

	if (m_TextureObjectHalndes.Texture != NULL)
	{
		glDeleteTextures(1, &m_TextureObjectHalndes.Texture);
		m_TextureObjectHalndes.Texture = NULL;
	}

	Clear();

#if UO_DEBUG_INFO!=0
	g_GameObjectsCount--;
#endif //UO_DEBUG_INFO!=0
}
//----------------------------------------------------------------------------------
void CGameObject::OnChangeName(const string &newName)
{
	if (IsPlayer())
	{
		if (g_GameState >= GS_GAME)
		{
			char buf[256] = { 0 };
			sprintf_s(buf, "Ultima Online - %s (%s)", newName.c_str(), g_ServerList.GetSelectedServer()->Name.c_str());

			g_OrionWindow.SetTitle(buf);
		}

		/*if (PluginManager != NULL)
			PluginManager->WindowProc(g_hWnd, UOMSG_SET_PLAYER_NAME, (WPARAM)val.c_str(), 0);*/
	}
}
//----------------------------------------------------------------------------------
/*!
���������� �������� ����� Object Handles (������������� ������� ��������, ���� ��� �� ���� �������)
@param [__in] x �������� ���������� X
@param [__in] y �������� ���������� Y
@return 
*/
void CGameObject::DrawObjectHandlesTexture(const int &x, const int &y)
{
	if (m_TextureObjectHalndes.Texture == NULL)
	{
		if (m_NPC || IsCorpse())
			GenerateObjectHandlesTexture(ToWString(m_Name));
		else
			GenerateObjectHandlesTexture(g_ClilocManager.Cliloc(g_Language)->GetW(102000 + m_Graphic, m_Name));
	}

	m_TextureObjectHalndes.Draw(x, y);
}
//----------------------------------------------------------------------------------
/*!
������� �������� ����� Object Handles
@param [__in] x �������� ���������� X
@param [__in] y �������� ���������� Y
@return
*/
void CGameObject::SelectObjectHandlesTexture(const int &x, const int &y)
{
	if (m_TextureObjectHalndes.Texture != NULL)
	{
		int testX = g_MouseManager.Position.X - x;

		if (testX < 0 || testX >= g_ObjectHandlesWidth)
			return;

		int testY = g_MouseManager.Position.Y - y;

		if (testY < 0 || testY >= g_ObjectHandlesHeight)
			return;

		if (g_ObjectHandlesBackgroundPixels[(testY * g_ObjectHandlesWidth) + testX] != 0)
			g_SelectedObject.Init(this);
	}
}
//----------------------------------------------------------------------------------
/*!
������� �������� ��� ����� Object Handles
@param [__in] text ����� �������� �������
@return 
*/
void CGameObject::GenerateObjectHandlesTexture(wstring text)
{
	if (m_TextureObjectHalndes.Texture != NULL)
	{
		glDeleteTextures(1, &m_TextureObjectHalndes.Texture);
		m_TextureObjectHalndes.Texture = NULL;
	}

	int width = g_ObjectHandlesWidth - 20;

	uchar font = 1;
	CGLTextTexture textTexture;
	ushort color = 0xFFFF;
	uchar cell = 30;
	TEXT_ALIGN_TYPE tat = TS_CENTER;
	ushort flags = 0;

	if (g_FontManager.GetWidthW(font, text.c_str(), text.length()) > width)
		text = g_FontManager.GetTextByWidthW(font, text.c_str(), text.length(), width - 6, true);

	UINT_LIST textData = g_FontManager.GeneratePixelsW(font, textTexture, text.c_str(), color, cell, width, tat, flags);

	if (!textData.size())
		return;

	static const int size = g_ObjectHandlesWidth * g_ObjectHandlesHeight;
	ushort pixels[size] = { 0 };

	memcpy(&pixels[0], &g_ObjectHandlesBackgroundPixels[0], size * 2);

	color = 0;

	if (m_NPC)
	{
		if (IsPlayer())
			color = 0x0386;
		else
			color = g_ConfigManager.GetColorByNotoriety(GameCharacterPtr()->Notoriety);

		if (color)
		{
			IFOR(x, 0, g_ObjectHandlesWidth)
			{
				IFOR(y, 0, g_ObjectHandlesHeight)
				{
					ushort &pixel = pixels[(y * g_ObjectHandlesWidth) + x];

					if (pixel)
					{
						uchar r = (pixel & 0x1F);
						uchar g = ((pixel >> 5) & 0x1F);
						uchar b = ((pixel >> 10) & 0x1F);

						if (r == g && r == b)
							pixel = g_ColorManager.GetColor16(pixel, color) | 0x8000;
					}
				}
			}
		}
	}

	int maxHeight = textTexture.Height;

	IFOR(x, 0, width)
	{
		int gumpDataX = x + 10;

		if (gumpDataX >= g_ObjectHandlesWidth)
			break;

		IFOR(y, 0, maxHeight)
		{
			int gumpDataY = y + 1;

			if (gumpDataY >= g_ObjectHandlesHeight)
				break;

			uint &pixel = textData[(y * textTexture.Width) + x];

			if (pixel)
			{
				puchar bytes = (PBYTE)&pixel;
				uchar buf = bytes[0];
				bytes[0] = bytes[3];
				bytes[3] = buf;
				buf = bytes[1];
				bytes[1] = bytes[2];
				bytes[2] = buf;
				pixels[(gumpDataY * g_ObjectHandlesWidth) + gumpDataX] = g_ColorManager.Color32To16(pixel) | 0x8000;
			}
		}
	}

	m_TextureObjectHalndes.Width = g_ObjectHandlesWidth;
	m_TextureObjectHalndes.Height = g_ObjectHandlesHeight;

	g_GL.BindTexture16(m_TextureObjectHalndes.Texture, g_ObjectHandlesWidth, g_ObjectHandlesHeight, pixels);
}
//----------------------------------------------------------------------------------
/*!
�������� ����� � ���������
@param [__in] td ������ �� ������ ������
@return
*/
void CGameObject::AddText(CTextData *msg)
{
	m_TextControl->Add(msg);

	string msgname = "You see:";

	if (!m_YouSeeJournalPrefix)
		msgname = m_Name + ": ";

	/*if (m_Clicked)
	{
		m_Clicked = false;

		if (IsPlayer()) //(m_NPC)
			msgname = m_Name + ": ";
	}*/

	g_Orion.AddJournalMessage(msg, msgname);
}
//----------------------------------------------------------------------------------
/*!
�������� ������ ��������
@return ������ ��������
*/
ushort CGameObject::GetMountAnimation()
{
	return m_Graphic; // + UO->GetStaticPointer(m_Graphic)->Increment;
}
//----------------------------------------------------------------------------------
/*!
�������� ���������
@return
*/
void CGameObject::Clear()
{
	if (!Empty())
	{
		CGameObject *obj = (CGameObject*)m_Items;

		while (obj != NULL)
		{
			CGameObject *next = (CGameObject*)obj->m_Next;

			g_World->RemoveObject(obj);

			obj = next;
		}

		m_Items = NULL;
	}
}
//----------------------------------------------------------------------------------
/*!
������ �� ���
@return ������ � ������� ������
*/
int CGameObject::IsGold()
{
	switch (m_Graphic)
	{
		case 0x0EED:
			return 1;
		/*case 0x0EEA:
			return 2;*/
		case 0x0EF0:
			return 3;
		default:
			break;
	}

	return 0;
}
//----------------------------------------------------------------------------------
/*!
�������� ������ �������� ��� ���������
@param [__out] doubleDraw ������� ��������� �������
@return ������ ��������
*/
ushort CGameObject::GetDrawGraphic(bool &doubleDraw)
{
	int index = IsGold();
	ushort result = m_Graphic;

	const ushort graphicAccociateTable[3][3] =
	{
		{0x0EED, 0x0EEE, 0x0EEF},
		{0x0EEA, 0x0EEB, 0x0EEC},
		{0x0EF0, 0x0EF1, 0x0EF2}
	};

	if (index)
	{
		int graphicIndex = (int)(m_Count > 1) + (int)(m_Count > 5);
		result = graphicAccociateTable[index - 1][graphicIndex];
	}
	else
		doubleDraw = IsStackable() && (m_Count > 1);

	return result;
}
//----------------------------------------------------------------------------------
/*!
���������� ������
@param [__in] drawX �������� ���������� X
@param [__in] drawY �������� ���������� Y
@param [__in] ticks ������ �������
@return 
*/
void CGameObject::DrawEffects(int x, int y)
{
	CGameEffect *effect = m_Effects;
	char z = m_Z * 4;

	if (m_NPC)
	{
		CGameCharacter *gc = GameCharacterPtr();

		x += gc->OffsetX;
		y += gc->OffsetY;

		//ANIMATION_DIMENSIONS dims = AnimationManager->GetAnimationDimensions(this);

		//x += dims.Width / 2;
		//y -= dims.Height / 2;
	}

	while (effect != NULL)
	{
		CGameEffect *next = (CGameEffect*)effect->m_Next;
		
		ushort eGraphic = 0;
		ushort gGraphic = 0;

		if (effect->Duration < g_Ticks)
			RemoveEffect(effect);
		else if (effect->LastChangeFrameTime < g_Ticks)
		{
			effect->LastChangeFrameTime = g_Ticks + effect->Speed;

			if (effect->EffectType == EF_LIGHTING) //lighting
			{
				int animIndex = effect->AnimIndex;
				
				gGraphic = 0x4E20 + animIndex;

				animIndex++;
				if (animIndex >= 10)
					RemoveEffect(effect);
				else
					effect->AnimIndex = animIndex;
			}
			else if (effect->EffectType == EF_STAY_AT_SOURCE)
				eGraphic = effect->CalculateCurrentGraphic();
		}
		else
		{
			if (effect->EffectType == EF_LIGHTING) //lighting
				gGraphic = 0x4E20 + effect->AnimIndex;
			else if (effect->EffectType == EF_STAY_AT_SOURCE)
				eGraphic = effect->GetCurrentGraphic();
		}
		
		if (gGraphic != 0)
		{
			WISP_GEOMETRY::CSize size = g_Orion.GetGumpDimension(gGraphic);

			effect->ApplyRenderMode();

			g_Orion.DrawGump(gGraphic, effect->Color, x - (size.Width / 2), (y - size.Height) - z);

			effect->RemoveRenderMode();
		}
		else if (eGraphic != 0)
		{
			effect->ApplyRenderMode();

			g_Orion.DrawStaticArt(eGraphic, effect->Color, x, y, m_Z);

			effect->RemoveRenderMode();
		}

		effect = next;
	}
}
//----------------------------------------------------------------------------------
/*!
�������� ������
@param [__in] effect ������ �� ������
@return 
*/
void CGameObject::AddEffect(CGameEffect *effect)
{
	if (m_Effects == NULL)
	{
		m_Effects = effect;
		effect->m_Next = NULL;
		effect->m_Prev = NULL;
	}
	else
	{
		effect->m_Next = m_Effects;
		m_Effects->m_Prev = effect;
		effect->m_Prev = NULL;
		m_Effects = effect;
	}
}
//----------------------------------------------------------------------------------
/*!
������� ������
@param [__in] effect ������ �� ������
@return 
*/
void CGameObject::RemoveEffect(CGameEffect *effect)
{
	if (effect->m_Prev == NULL)
	{
		m_Effects = (CGameEffect*)effect->m_Next;

		if (m_Effects != NULL)
			m_Effects->m_Prev = NULL;
	}
	else
	{
		effect->m_Prev->m_Next = effect->m_Next;

		if (effect->m_Next != NULL)
			effect->m_Next->m_Prev = effect->m_Prev;
	}
	
	effect->m_Next = NULL;
	effect->m_Prev = NULL;
	delete effect;
}
//----------------------------------------------------------------------------------
/*!
�������� ������ � ������ �������� �������� ����������
@param [__in] obj ������ �� ������
@return 
*/
void CGameObject::AddObject(CGameObject *obj)
{
	g_World->RemoveFromContainer(obj);

	if (m_Next == NULL)
	{
		m_Next = obj;
		m_Next->m_Prev = this;
		m_Next->m_Next = NULL;

		((CGameObject*)m_Next)->Container = m_Container;
	}
	else
	{
		CGameObject *item = (CGameObject*)m_Next;

		while (item->m_Next != NULL)
			item = (CGameObject*)item->m_Next;

		item->m_Next = obj;
		obj->m_Next = NULL;
		obj->m_Prev = item;

		obj->Container = m_Container;
	}
}
//----------------------------------------------------------------------------------
/*!
�������� ������ � ��������� (this - ���������)
@param [__in] obj ������ �� ������
@return 
*/
void CGameObject::AddItem(CGameObject *obj)
{
	if (obj->Container != 0xFFFFFFFF)
		return;

	g_World->RemoveFromContainer(obj);

	if (m_Items != NULL)
	{
		CGameObject *item = (CGameObject*)Last();

		item->m_Next = obj;
		obj->m_Next = NULL;
		obj->m_Prev = item;
	}
	else
	{
		m_Items = obj;
		m_Items->m_Next = NULL;
		m_Items->m_Prev = NULL;
	}

	obj->Container = Serial;
}
//----------------------------------------------------------------------------------
/*!
������ ������ �� ����������
@param [__in] obj ������ �� ������
@return 
*/
void CGameObject::Reject(CGameObject *obj)
{
	if (obj->Container != m_Serial)
		return;

	if (m_Items != NULL)
	{
		if (((CGameObject*)m_Items)->Serial == obj->Serial)
		{
			if (m_Items->m_Next != NULL)
			{
				m_Items = m_Items->m_Next;
				m_Items->m_Prev = NULL;
			}
			else
				m_Items = NULL;
		}
		else
		{
			if (obj->m_Next != NULL)
			{
				if (obj->m_Prev != NULL)
				{
					obj->m_Prev->m_Next = obj->m_Next;
					obj->m_Next->m_Prev = obj->m_Prev;
				}
				else //WTF???
					obj->m_Next->m_Prev = NULL;
			}
			else if (obj->m_Prev != NULL)
				obj->m_Prev->m_Next = NULL;
		}
	}

	obj->m_Next = NULL;
	obj->m_Prev = NULL;
	obj->Container = 0xFFFFFFFF;
}
//----------------------------------------------------------------------------------
/*!
����� ������ � ����, � ������� ���������� ���������
@return ������ �� ������ � ����
*/
CGameObject *CGameObject::GetTopObject()
{
	CGameObject *obj = this;

	while (obj->Container != 0xFFFFFFFF)
		obj = g_World->FindWorldObject(obj->Container);

	return obj;
}
//----------------------------------------------------------------------------------
