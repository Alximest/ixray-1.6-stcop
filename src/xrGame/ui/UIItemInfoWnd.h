#pragma once  
#include "../../xrUI/Widgets/UIWindow.h"  
#include "../../xrUI/Widgets/UIFrameWindow.h"
#include "UIItemInfo.h" 
class CUI3dStatic;  
class CUIStatic;  
class CUICellItem;  


class CUIItemInfoWnd : public CUIWindow  
{  
	typedef CUIWindow inherited;  
public:  
	void			Init(LPCSTR xml_name, LPCSTR start_from);  
	void			InitItem(CUICellItem* cell_item);  
	virtual bool	OnMouseAction(float x, float y, EUIMessages mouse_action);  
protected:  
	CUIStatic* 		m_UIWeight = nullptr;  
	CUIStatic* 		m_UICondition = nullptr;
	CUIFrameWindow*	m_UIBackground = nullptr;  
	CUI3dStatic*	m_UIModel = nullptr;  
	CUIStatic*		m_UIName = nullptr;  
	CUIStatic*		m_UIDesc = nullptr;  
	bool			m_use_3d_icon = false;  
	bool			m_bDragging = false;  
	Fvector2		m_modelAreaPos = {};  
	Fvector2		m_modelAreaSize = {};
};