#include "StdAfx.h"  
#include "UIItemInfoWnd.h"  
#include "../../xrUI/Widgets/UIStatic.h"  
#include "../../xrUI/Widgets/UI3dStatic.h"  
#include "../../xrUI/Widgets/UIFrameWindow.h"  
#include "../../xrUI/UIXmlInit.h"  
#include "../../xrUI/UICursor.h"  
#include "../../xrEngine/xr_input.h"  
#include "../../xrEngine/Render.h"  
#include "../../xrCore/EngineExternal.h"  
#include "UICellItem.h"  
#include "../inventory_item.h"  
#include "../PhysicsShellHolder.h" 
#include "UIInventoryUtilities.h" 

#define INV_GRID_WIDTH2(SCALE_ICON)  (40.0f * SCALE_ICON)  
#define INV_GRID_HEIGHT2(SCALE_ICON) (40.0f * SCALE_ICON)

void 	CUIItemInfoWnd::Init(LPCSTR xml_name, LPCSTR start_from)  
{  
	CUIXml uiXml;  
	uiXml.Load(CONFIG_PATH, UI_PATH, xml_name);  
  
	CUIXmlInit xml_init;  
	string512 str;  
  
	strcpy_s(str, sizeof(str), start_from);  
	xml_init.InitWindow(uiXml, str, 0, this);  

		xr_strconcat(str, start_from, ":background");  
	if (uiXml.NavigateToNode(str))  
	{  
		m_UIBackground = new CUIFrameWindow();  
		m_UIBackground->SetAutoDelete(true);  
		AttachChild(m_UIBackground);  
		xml_init.InitFrameWindow(uiXml, str, 0, m_UIBackground);  
	}

	xr_strconcat(str, start_from, ":model");  
	if (uiXml.NavigateToNode(str))  
	{  
		m_UIModel = new CUI3dStatic();  
		m_UIModel->SetAutoDelete(true);  
		xml_init.InitStatic(uiXml, str, 0, m_UIModel);  
		AttachChild(m_UIModel);  
	}  
  
	xr_strconcat(str, start_from, ":name_static");  
	m_UIName = new CUIStatic();  
	m_UIName->SetAutoDelete(true);  
	xml_init.InitStatic(uiXml, str, 0, m_UIName);  
	AttachChild(m_UIName);  
  
	xr_strconcat(str, start_from, ":descr_static");  
	m_UIDesc = new CUIStatic();  
	m_UIDesc->SetAutoDelete(true);  
	xml_init.InitStatic(uiXml, str, 0, m_UIDesc);  
	AttachChild(m_UIDesc);  
}  
  
void CUIItemInfoWnd::InitItem(CUICellItem* cell_item)  
{  
	PIItem pInvItem = (PIItem)cell_item->m_pData;  
	if (!pInvItem)  
		return;  
  
	m_UIName->SetText(pInvItem->NameItem());  
	m_UIDesc->SetText(pInvItem->ItemDescription().c_str());  
  
	if (m_UIModel)  
	{  
		static const bool enable3DIcons = EngineExternal()[EEngineExternalGame::Enable3DIcons];  
		if (enable3DIcons)  
		{  
			m_UIModel->SetIconVisual(pInvItem->m_3d_static_visual_name);  
			m_UIModel->SetScaleFactor(pInvItem->m_3d_static_scale);  
			m_UIModel->SetIconOffset(pInvItem->IconOffset);  
			m_UIModel->SetXYZ(pInvItem->m_3d_static_rotate);  
			m_UIModel->SetBonesVisible(pInvItem->object().Visual()->dcast_PKinematics());  
		}  
		else  
		{  
			m_UIModel->SetVisual(nullptr);  
		}  
  
		// 2D-иконка — в обеих ветках  
		m_UIModel->SetShader(InventoryUtilities::GetEquipmentIconsShader(pInvItem->IconsTexture.c_str()));  
  
		Irect item_grid_rect = pInvItem->GetInvGridRect();  
		float scaleIcon = pInvItem->ScaleIcon;  
  
		Frect texture_rect = {};  
		texture_rect.lt.set(item_grid_rect.x1 * INV_GRID_WIDTH(scaleIcon),  
		                    item_grid_rect.y1 * INV_GRID_HEIGHT(scaleIcon));  
		texture_rect.rb.set(item_grid_rect.x2 * INV_GRID_WIDTH(scaleIcon),  
		                    item_grid_rect.y2 * INV_GRID_HEIGHT(scaleIcon));  
		texture_rect.rb.add(texture_rect.lt);  
  
		m_UIModel->GetUIStaticItem().SetTextureRect(texture_rect);  
		m_UIModel->TextureOn();  
		m_UIModel->SetStretchTexture(true);  
  
		Fvector2 v_r = {};  
		v_r.set(item_grid_rect.x2 * INV_GRID_WIDTH2(scaleIcon) / scaleIcon,  
		        item_grid_rect.y2 * INV_GRID_HEIGHT2(scaleIcon) / scaleIcon);  
		v_r.x *= UI().get_current_kx();  
  
		m_UIModel->GetUIStaticItem().SetSize(v_r);  
		m_UIModel->SetWidth(v_r.x);  
		m_UIModel->SetHeight(v_r.y);  
	}  
}


bool CUIItemInfoWnd::OnMouseAction(float x, float y, EUIMessages mouse_action)  
{  
	// вращение модели  
	if (m_UIModel)  
	{  
		if (mouse_action == WINDOW_LBUTTON_UP && m_UIModel->IsCaptMoving())  
			m_UIModel->SetCaptMoving(false);  
  
		if (m_UIModel->CursorOverWindow() && mouse_action == WINDOW_LBUTTON_DOWN)  
			m_UIModel->SetCaptMoving(true);  
  
		if (m_UIModel->IsCaptMoving() && m_UIModel->CursorOverWindow())  
		{  
			Fvector xyz = m_UIModel->GetXYZ();  
			Fvector2 delta = GetUICursor().GetCursorPositionDelta();  
			xyz.y += delta.x / 40.f;  
			m_UIModel->SetXYZ(xyz);  
			return true;  
		}  
	}  
  
	// перетаскивание окна — только не по модели  
	if (mouse_action == WINDOW_LBUTTON_DOWN && CursorOverWindow()  
		&& !(m_UIModel && m_UIModel->CursorOverWindow()))  
		m_bDragging = true;  
  
	if (mouse_action == WINDOW_LBUTTON_UP)  
		m_bDragging = false;  
  
	if (m_bDragging && mouse_action == WINDOW_MOUSE_MOVE)  
	{  
		Fvector2 delta = GetUICursor().GetCursorPositionDelta();  
		Fvector2 pos = GetWndPos();  
		pos.x += delta.x;  
		pos.y += delta.y;  
		SetWndPos(pos);  
		return true;  
	}  
  
	return inherited::OnMouseAction(x, y, mouse_action);   // <-- обязательно в конце  
}