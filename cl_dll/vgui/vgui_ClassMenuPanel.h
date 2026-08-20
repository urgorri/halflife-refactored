#ifndef VGUI_CLASSMENUPANEL_H
#define VGUI_CLASSMENUPANEL_H

#include "vgui_TeamFortressViewport.h"

class CClassMenuPanel : public CMenuPanel
{
  private:
	CTransparentPanel *m_pClassInfoPanel[PC_LASTCLASS];
	Label *m_pPlayers[PC_LASTCLASS];
	ClassButton *m_pButtons[PC_LASTCLASS];
	CommandButton *m_pCancelButton;
	ScrollPanel *m_pScrollPanel;

	CImageLabel *m_pClassImages[MAX_TEAMS][PC_LASTCLASS];

	int m_iCurrentInfo;

	enum
	{
		STRLENMAX_PLAYERSONTEAM = 128
	};
	char m_sPlayersOnTeamString[STRLENMAX_PLAYERSONTEAM];

  public:
	CClassMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );

	virtual bool SlotInput( int iSlot );
	virtual void Open( void );
	virtual void Update( void );
	virtual void SetActiveInfo( int iInput );
	virtual void Initialize( void );

	virtual void Reset( void )
	{
		CMenuPanel::Reset();
		m_iCurrentInfo = 0;
	}
};

#endif // VGUI_CLASSMENUPANEL_H
