#include "StdAfx.h"  
#include "UIItemInfoWnd.h"  
  
#include "../../xrUI/Widgets/UIStatic.h"  
#include "../../xrUI/Widgets/UI3dStatic.h"  
#include "../../xrUI/Widgets/UIScrollView.h"  
#include "../../xrUI/UIXmlInit.h"  
#include "../../xrUI/UIHelper.h"  
#include "../../xrUI/UICursor.h"  
#include "../../xrEngine/xr_input.h"  
#include "../../xrEngine/string_table.h"  
#include "../../xrCore/EngineExternal.h"  
  
#include "UICellItem.h"  
#include "UIInventoryUtilities.h"  
#include "UIWpnParams.h"  
#include "UIKnifeParams.h"  
#include "UIGrenadeParams.h"  
#include "ui_af_params.h"  
#include "UIOutfitInfo.h"  
#include "UIBoosterInfo.h"  
  
#include "../inventory_item.h"  
#include "../Inventory.h"  
#include "../Weapon.h"  
#include "../WeaponKnife.h" 
#include "../Grenade.h"  
#include "../Artefact.h"  
#include "../CustomOutfit.h"  
#include "../ActorHelmet.h"  
#include "../ActorBackpack.h"  
#include "../eatable_item.h"  
  
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
  
	// фон  
	xr_strconcat(str, start_from, ":background");  
	if (uiXml.NavigateToNode(str))  
	{  
		m_UIBackground = new CUIFrameWindow();  
		m_UIBackground->SetAutoDelete(true);  
		AttachChild(m_UIBackground);  
		xml_init.InitFrameWindow(uiXml, str, 0, m_UIBackground);  
	}  
  
	// 3D-модель  
	xr_strconcat(str, start_from, ":model");  
	if (uiXml.NavigateToNode(str))  
	{  
		m_UIModel = new CUI3dStatic();  
		m_UIModel->SetAutoDelete(true);  
		xml_init.InitStatic(uiXml, str, 0, m_UIModel);  
		AttachChild(m_UIModel);  
  
		m_modelAreaPos = m_UIModel->GetWndPos();  
		m_modelAreaSize.set(m_UIModel->GetWidth(), m_UIModel->GetHeight());  
	}  
  
	// название  
	xr_strconcat(str, start_from, ":name_static");  
	if (uiXml.NavigateToNode(str))  
	{  
		m_UIName = new CUIStatic();  
		m_UIName->SetAutoDelete(true);  
		xml_init.InitStatic(uiXml, str, 0, m_UIName);  
		AttachChild(m_UIName);  
	}  
  
	// скролл с описанием и параметрами  
	xr_strconcat(str, start_from, ":descr_list");  
	if (uiXml.NavigateToNode(str))  
	{  
		m_UIDesc = new CUIScrollView();  
		m_UIDesc->SetAutoDelete(true);  
		AttachChild(m_UIDesc);  
		xml_init.InitScrollView(uiXml, str, 0, m_UIDesc);  
  
		xr_strconcat(str, start_from, ":descr_list:font");  
		xml_init.InitFont(uiXml, str, 0, m_uDescClr, m_pDescFont);  
	}  
  
	// виджеты параметров — из actor_menu_item.xml (переопределяется модами)  
	CUIXml paramsXml;  
	paramsXml.Load(CONFIG_PATH, UI_PATH, "actor_menu_item.xml");  
  
	m_UIConditionWnd = new CUIConditionParams();  
	m_UIConditionWnd->InitFromXml(paramsXml);  
  
	m_UIWpnParams = new CUIWpnParams();  
	m_UIWpnParams->InitFromXml(paramsXml);  
  
	m_UIKnifeParams = new CUIKnifeParams();  
	m_UIKnifeParams->InitFromXml(paramsXml);  
  
	m_UIGrenadeParams = new CUIGrenadeParams();  
	m_UIGrenadeParams->InitFromXml(paramsXml);  
  
	m_UIArtefactParams = new CUIArtefactParams(CUIArtefactParams::CParamType::eParamTypeArtefact);  
	m_UIArtefactParams->InitFromXml(paramsXml);  
  
	m_UIOutfitParams = new CUIArtefactParams(CUIArtefactParams::CParamType::eParamTypeOutfit);  
	m_UIOutfitParams->InitFromXml(paramsXml);  
  
	m_UIBackpackParams = new CUIArtefactParams(CUIArtefactParams::CParamType::eParamTypeBackpack);  
	m_UIBackpackParams->InitFromXml(paramsXml);  
  
	m_UIOutfitInfo = new CUIOutfitInfo();  
	m_UIOutfitInfo->InitFromXml(paramsXml);  
  
	m_UIBoosterInfo = new CUIBoosterInfo();  
	m_UIBoosterInfo->InitFromXml(paramsXml);  
}  
void CUIItemInfoWnd::InitItem(CUICellItem* cell_item)  
{  
	PIItem pInvItem = (PIItem)cell_item->m_pData;  
	if (!pInvItem)  
		return;  
  
	if (m_UIName)  
		m_UIName->SetText(pInvItem->NameItem());  
  
	// --- описание + параметры в скролле ---  
	if (m_UIDesc)  
	{  
		m_UIDesc->Clear();  
  
		// описание первым элементом скролла  
		CUIStatic* pDesc = new CUIStatic();  
		pDesc->SetTextColor(m_uDescClr);  
		pDesc->SetFont(m_pDescFont);  
		pDesc->SetWidth(m_UIDesc->GetDesiredChildWidth());  
		pDesc->SetTextComplexMode(true);  
		pDesc->SetText(pInvItem->ItemDescription().c_str());  
		pDesc->AdjustHeightToText();  
		m_UIDesc->AddWindow(pDesc, true);  
  
		CInventoryItem* pCompareItem = nullptr;  
  
		// состояние  
		if (m_UIConditionWnd)  
		{  
			if (pInvItem->IsUsingCondition())  
			{  
				m_UIConditionWnd->SetInfo(pCompareItem, *pInvItem);  
				m_UIDesc->AddWindow(m_UIConditionWnd, false);  
			}  
			else  
			{  
				m_UIDesc->RemoveWindow(m_UIConditionWnd);  
			}  
		}  
  
		// оружие  
		if (m_UIWpnParams && m_UIWpnParams->Check(*pInvItem))  
		{  
			m_UIWpnParams->SetInfo(pCompareItem, *pInvItem);  
			m_UIDesc->AddWindow(m_UIWpnParams, false);  
		}  
  
		// нож  
		if (m_UIKnifeParams && m_UIKnifeParams->Check(*pInvItem))  
		{  
			m_UIKnifeParams->SetInfo(pCompareItem, *pInvItem);  
			m_UIDesc->AddWindow(m_UIKnifeParams, false);  
		}  
  
		// граната  
		if (m_UIGrenadeParams && m_UIGrenadeParams->Check(*pInvItem))  
		{  
			m_UIGrenadeParams->SetInfo(pCompareItem, *pInvItem);  
			m_UIDesc->AddWindow(m_UIGrenadeParams, false);  
		}  
  
		// артефакт — Check принимает секцию  
		if (m_UIArtefactParams && m_UIArtefactParams->Check(pInvItem->object().cNameSect()))  
		{  
			m_UIArtefactParams->SetInfo(*pInvItem);  
			m_UIDesc->AddWindow(m_UIArtefactParams, false);  
		}  
  
		// броня / шлем / рюкзак  
		CCustomOutfit* outfit = pInvItem->cast_outfit();  
		CHelmet* helmet = pInvItem->cast_helmet();  
		CBackpack* backpack = pInvItem->cast_backpack();  
  
		if (outfit && m_UIOutfitInfo)  
		{  
			m_UIOutfitInfo->UpdateInfo(outfit, nullptr);  
			m_UIDesc->AddWindow(m_UIOutfitInfo, false);  
		}  
		if (helmet && m_UIOutfitInfo)  
		{  
			m_UIOutfitInfo->UpdateInfo(helmet, nullptr);  
			m_UIDesc->AddWindow(m_UIOutfitInfo, false);  
		}  
		if (m_UIOutfitParams && outfit)  
		{  
			m_UIOutfitParams->SetInfo(*pInvItem);  
			m_UIDesc->AddWindow(m_UIOutfitParams, false);  
		}  
		if (m_UIBackpackParams && backpack)  
		{  
			m_UIBackpackParams->SetInfo(*pInvItem);  
			m_UIDesc->AddWindow(m_UIBackpackParams, false);  
		}  
  
		// еда/бустеры — SetInfo принимает секцию  
		if (m_UIBoosterInfo && pInvItem->cast_eatable_item())  
		{  
			m_UIBoosterInfo->SetInfo(pInvItem->object().cNameSect());  
			m_UIDesc->AddWindow(m_UIBoosterInfo, false);  
		}  
  
		m_UIDesc->ScrollToBegin();  
	}
  
	if (!m_UIModel)  
		return;  
  
	// 3D — копия визуала из ячейки инвентаря  
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
	if (inherited::OnMouseAction(x, y, mouse_action))  
		return true;  
  
	// вращение 3D-модели — курсор над model + зажатая ЛКМ  
	if (m_UIModel && m_UIModel->GetVisual()  
		&& m_UIModel->CursorOverWindow()  
		&& pInput->LeftMouseButtonPressed())  
	{  
		Fvector xyz = m_UIModel->GetXYZ();  
		Fvector2 delta = GetUICursor().GetCursorPositionDelta();  
		xyz.x += delta.y / 150.f;  
		xyz.y += delta.x / 150.f;  
		m_UIModel->SetXYZ(xyz);  
		return true;  
	}  
  
	// перетаскивание окна за фон  
	if (mouse_action == WINDOW_LBUTTON_DOWN && CursorOverWindow())  
	{  
		m_bDragging = true;  
		return true;  
	}  
	if (mouse_action == WINDOW_LBUTTON_UP && m_bDragging)  
	{  
		m_bDragging = false;  
		return true;  
	}  
	if (mouse_action == WINDOW_MOUSE_MOVE && m_bDragging)  
	{  
		Fvector2 pos = GetWndPos();  
		pos.add(GetUICursor().GetCursorPositionDelta());  
		SetWndPos(pos);  
		return true;  
	}  
  
	// глотаем клики по окну  
	if (CursorOverWindow())  
		return true;  
  
	return false;  
}