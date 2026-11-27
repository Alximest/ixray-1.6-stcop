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
  
void CUIItemInfoWnd::Init(LPCSTR xml_name, LPCSTR start_from)  
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
  
		m_use_3d_icon = uiXml.ReadAttribBool(str, 0, "use_3d",  
			EngineExternal()[EEngineExternalGame::Enable3DIcons]);  
  
		m_modelAreaPos = m_UIModel->GetWndPos();  
		m_modelAreaSize.set(m_UIModel->GetWidth(), m_UIModel->GetHeight());  
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
  
	// свойства: вес и состояние  
	xr_strconcat(str, start_from, ":static_weight");  
	if (uiXml.NavigateToNode(str))  
		m_UIWeight = UIHelper::CreateStatic(uiXml, str, this);  
  
	xr_strconcat(str, start_from, ":static_condition");  
	if (uiXml.NavigateToNode(str))  
		m_UICondition = UIHelper::CreateStatic(uiXml, str, this);  
}
  
void CUIItemInfoWnd::InitItem(CUICellItem* cell_item)  
{  
	PIItem pInvItem = (PIItem)cell_item->m_pData;  
	if (!pInvItem)  
		return;  
  
	m_UIName->SetText(pInvItem->NameItem());  
	m_UIDesc->SetText(pInvItem->ItemDescription().c_str());  
  
	// свойства — подписи из string table  
	string128 buf;  
  
	if (m_UIWeight)  
	{  
		xr_sprintf(buf, sizeof(buf), "%s: %.2f %s",  
			StringTable()->translate("st_weight").c_str(),  
			pInvItem->Weight(),  
			StringTable()->translate("st_kg").c_str());  
		m_UIWeight->SetText(buf);  
	}  
  
	if (m_UICondition)  
	{  
		xr_sprintf(buf, sizeof(buf), "%s: %d%%",  
			StringTable()->translate("st_condition").c_str(),  
			iFloor(pInvItem->GetCondition() * 100.f));  
		m_UICondition->SetText(buf);  
	}  
  
	if (!m_UIModel)  
		return;  
  
	// пробуем 3D — копия визуала из ячейки инвентаря  
	m_UIModel->SetVisual(cell_item->GetVisual());  
  
	if (m_UIModel->GetVisual())  
	{  
		m_UIModel->SetScaleFactor(pInvItem->m_3d_static_scale);  
		m_UIModel->SetIconOffset(pInvItem->IconOffset);  
		m_UIModel->SetXYZ(pInvItem->m_3d_static_rotate);  
		m_UIModel->TextureOff();  
  
		m_UIModel->SetWndPos(m_modelAreaPos);  
		m_UIModel->SetWidth(m_modelAreaSize.x);  
		m_UIModel->SetHeight(m_modelAreaSize.y);  
	}  
}

  
bool CUIItemInfoWnd::OnMouseAction(float x, float y, EUIMessages mouse_action)  
{  
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
  
	return inherited::OnMouseAction(x, y, mouse_action);  
}