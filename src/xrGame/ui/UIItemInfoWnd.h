#pragma once  
#include "../../xrUI/Widgets/UIWindow.h"  
#include "../../xrUI/Widgets/UIFrameWindow.h"  
  
class CUI3dStatic;  
class CUIStatic;  
class CUICellItem;  
class CUIScrollView;  
class CUIConditionParams;  
class CUIWpnParams;  
class CUIKnifeParams;  
class CUIGrenadeParams;  
class CUIArtefactParams;  
class CUIOutfitInfo;  
class CUIBoosterInfo;  
class CInventoryItem;  
  
class CUIItemInfoWnd : public CUIWindow  
{  
	typedef CUIWindow inherited;  
public:  
	void			Init(LPCSTR xml_name, LPCSTR start_from);  
	void			InitItem(CUICellItem* cell_item);  
	virtual bool	OnMouseAction(float x, float y, EUIMessages mouse_action);  
  
protected:  
	CUIFrameWindow*		m_UIBackground = nullptr;  
	CUI3dStatic*		m_UIModel = nullptr;  
	CUIStatic*			m_UIName = nullptr;  
	CUIScrollView*		m_UIDesc = nullptr;  
  
	// виджеты параметров (грузятся из actor_menu_item.xml)  
	CUIConditionParams*	m_UIConditionWnd = nullptr;  
	CUIWpnParams*		m_UIWpnParams = nullptr;  
	CUIKnifeParams*		m_UIKnifeParams = nullptr;  
	CUIGrenadeParams*	m_UIGrenadeParams = nullptr;  
	CUIArtefactParams*	m_UIArtefactParams = nullptr;  
	CUIArtefactParams*	m_UIOutfitParams = nullptr;  
	CUIArtefactParams*	m_UIBackpackParams = nullptr;  
	CUIOutfitInfo*		m_UIOutfitInfo = nullptr;  
	CUIBoosterInfo*		m_UIBoosterInfo = nullptr;  
  
	// шрифт и цвет текста описания (из descr_list:font)  
	CGameFont*			m_pDescFont = nullptr;  
	u32					m_uDescClr = 0;  
  
	bool				m_use_3d_icon = false;  
	bool				m_bDragging = false;  
	Fvector2			m_modelAreaPos = {};  
	Fvector2			m_modelAreaSize = {};  
};