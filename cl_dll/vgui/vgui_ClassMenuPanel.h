//=========== (C) Copyright 1996-2002 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: TFC Class Menu
//
//=============================================================================

#ifndef VGUI_CLASSMENUPANEL_H
#define VGUI_CLASSMENUPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_TeamFortressViewport.h"

using namespace vgui;

class CClassMenuDescriptionPanel : public CTransparentPanel
{
  private:
	Label *m_pPlayers;
	CImageLabel *m_pClassImages[MAX_TEAMS];
	int m_iClass;

  public:
	CClassMenuDescriptionPanel( int iClass, int x, int y, int wide, int tall, int clientWide );

	void UpdatePlayerCount( int iTotal, const char *szPlayersOnTeamString, int r, int g, int b );
	void SetActiveTeamGraphic( int team );
};

class CClassMenuSelectionPanel : public CTransparentPanel
{
  private:
	ClassButton *m_pButtons[PC_LASTCLASS];
	CommandButton *m_pCancelButton;

  public:
	CClassMenuSelectionPanel( int x, int y, int wide, int tall );

	void InitializeButtons( Panel *pParent );
	void UpdateButtons( int &iYPos, int &iCurrentInfo );
	bool SlotInput( int iSlot );
	void SetActiveButton( int iInput );
};

class CClassMenuPanel : public CMenuPanel
{
  private:
	CClassMenuDescriptionPanel *m_pClassInfoPanel[PC_LASTCLASS];
	CClassMenuSelectionPanel *m_pSelectionPanel;
	ScrollPanel *m_pScrollPanel;

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
