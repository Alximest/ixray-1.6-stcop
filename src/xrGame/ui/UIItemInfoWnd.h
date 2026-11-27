#pragma once  
#include "../../xrUI/Widgets/UIWindow.h"  
  
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
	CUI3dStatic*	m_UIModel	= nullptr;  
	CUIStatic*		m_UIName	= nullptr;  
	CUIStatic*		m_UIDesc	= nullptr;  
};