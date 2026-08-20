#ifndef VGUI_TEAMMENUPANEL_H
#define VGUI_TEAMMENUPANEL_H

#include "vgui_TeamFortressViewport.h"

class CTeamMenuPanel : public CMenuPanel
{
  public:
	ScrollPanel *m_pScrollPanel;
	CTransparentPanel *m_pTeamWindow;
	Label *m_pMapTitle;
	TextPanel *m_pBriefing;
	TextPanel *m_pTeamInfoPanel[6];
	CommandButton *m_pButtons[6];
	bool m_bUpdatedMapName;
	CommandButton *m_pCancelButton;
	CommandButton *m_pSpectateButton;

	int m_iCurrentInfo;

  public:
	CTeamMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );

	virtual bool SlotInput( int iSlot );
	virtual void Open( void );
	virtual void Update( void );
	virtual void SetActiveInfo( int iInput );
	virtual void paintBackground( void );

	virtual void Initialize( void );

	virtual void Reset( void )
	{
		CMenuPanel::Reset();
		m_iCurrentInfo = 0;
	}
};

#endif // VGUI_TEAMMENUPANEL_H
