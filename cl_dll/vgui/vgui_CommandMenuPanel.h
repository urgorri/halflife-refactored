#ifndef VGUI_COMMANDMENUPANEL_H
#define VGUI_COMMANDMENUPANEL_H


#include <VGUI_Panel.h>
#include <VGUI_ActionSignal.h>
#include <VGUI_InputSignal.h>
#include <VGUI_Button.h>
#include "vgui_TeamFortressViewport.h" // Since many handlers depend on gViewPort and macros like MAX_COMMAND_SIZE



class CommandButton;
class Button;
class CCommandMenu;

class CCommandMenu : public vgui::Panel
{
  private:
	CCommandMenu *m_pParentMenu;
	int m_iXOffset;
	int m_iYOffset;

	// Buttons in this menu
	CommandButton *m_aButtons[MAX_BUTTONS];
	int m_iButtons;

	// opens menu from top to bottom (0 = default), or from bottom to top (1)?
	int m_iDirection;

  public:
	CCommandMenu( CCommandMenu *pParentMenu, int x, int y, int wide, int tall )
	    : vgui::Panel( x, y, wide, tall )
	{
		m_pParentMenu = pParentMenu;
		m_iXOffset    = x;
		m_iYOffset    = y;
		m_iButtons    = 0;
		m_iDirection  = 0;
	}

	CCommandMenu( CCommandMenu *pParentMenu, int direction, int x, int y, int wide, int tall )
	    : vgui::Panel( x, y, wide, tall )
	{
		m_pParentMenu = pParentMenu;
		m_iXOffset    = x;
		m_iYOffset    = y;
		m_iButtons    = 0;
		m_iDirection  = direction;
	}

	float m_flButtonSizeY;
	int m_iSpectCmdMenu;
	void AddButton( CommandButton *pButton );
	bool RecalculateVisibles( int iNewYPos, bool bHideAll );
	void RecalculatePositions( int iYOffset );
	void MakeVisible( CCommandMenu *pChildMenu );

	CCommandMenu *GetParentMenu() { return m_pParentMenu; };
	int GetXOffset() { return m_iXOffset; };
	int GetYOffset() { return m_iYOffset; };
	int GetDirection() { return m_iDirection; };
	int GetNumButtons() { return m_iButtons; };
	CommandButton *FindButtonWithSubmenu( CCommandMenu *pSubMenu );

	void ClearButtonsOfArmedState( void );

	void RemoveAllButtons( void );

	bool KeyInput( int keyNum );

	virtual void paintBackground();
};

class CMenuHandler_StringCommand : public vgui::ActionSignal
{
  protected:
	char m_pszCommand[MAX_COMMAND_SIZE];
	int m_iCloseVGUIMenu;

  public:
	CMenuHandler_StringCommand( char *pszCommand )
	{
		strncpy( m_pszCommand, pszCommand, MAX_COMMAND_SIZE );
		m_pszCommand[MAX_COMMAND_SIZE - 1] = '\0';
		m_iCloseVGUIMenu                   = false;
	}

	CMenuHandler_StringCommand( char *pszCommand, int iClose )
	{
		strncpy( m_pszCommand, pszCommand, MAX_COMMAND_SIZE );
		m_pszCommand[MAX_COMMAND_SIZE - 1] = '\0';
		m_iCloseVGUIMenu                   = true;
	}

	virtual void actionPerformed( vgui::Panel *panel )
	{
		gEngfuncs.pfnClientCmd( m_pszCommand );

		if ( m_iCloseVGUIMenu )
			gViewPort->HideTopMenu();
		else
			gViewPort->HideCommandMenu();
	}
};

class CMenuHandler_StringCommandWatch : public CMenuHandler_StringCommand
{
  private:
  public:
	CMenuHandler_StringCommandWatch( char *pszCommand )
	    : CMenuHandler_StringCommand( pszCommand )
	{
	}

	CMenuHandler_StringCommandWatch( char *pszCommand, int iClose )
	    : CMenuHandler_StringCommand( pszCommand, iClose )
	{
	}

	virtual void actionPerformed( vgui::Panel *panel )
	{
		CMenuHandler_StringCommand::actionPerformed( panel );

		// Try to guess the player's new team (it'll be corrected if it's wrong)
		if ( !strcmp( m_pszCommand, "jointeam 1" ) )
			g_iTeamNumber = 1;
		else if ( !strcmp( m_pszCommand, "jointeam 2" ) )
			g_iTeamNumber = 2;
		else if ( !strcmp( m_pszCommand, "jointeam 3" ) )
			g_iTeamNumber = 3;
		else if ( !strcmp( m_pszCommand, "jointeam 4" ) )
			g_iTeamNumber = 4;
	}
};

class CMenuHandler_StringCommandClassSelect : public CMenuHandler_StringCommand
{
  private:
  public:
	CMenuHandler_StringCommandClassSelect( char *pszCommand )
	    : CMenuHandler_StringCommand( pszCommand )
	{
	}

	CMenuHandler_StringCommandClassSelect( char *pszCommand, int iClose )
	    : CMenuHandler_StringCommand( pszCommand, iClose )
	{
	}

	virtual void actionPerformed( vgui::Panel *panel );
};

class CMenuHandler_PopupSubMenuInput : public vgui::InputSignal
{
  private:
	CCommandMenu *m_pSubMenu;
	vgui::Button *m_pButton;

  public:
	CMenuHandler_PopupSubMenuInput( vgui::Button *pButton, CCommandMenu *pSubMenu )
	{
		m_pSubMenu = pSubMenu;
		m_pButton  = pButton;
	}

	virtual void cursorMoved( int x, int y, vgui::Panel *panel )
	{
		// gViewPort->SetCurrentCommandMenu( m_pSubMenu );
	}

	virtual void cursorEntered( vgui::Panel *panel )
	{
		gViewPort->SetCurrentCommandMenu( m_pSubMenu );

		if ( m_pButton )
			m_pButton->setArmed( true );
	};
	virtual void cursorExited( vgui::Panel *Panel ) {};
	virtual void mousePressed( vgui::MouseCode code, vgui::Panel *panel ) {};
	virtual void mouseDoublePressed( vgui::MouseCode code, vgui::Panel *panel ) {};
	virtual void mouseReleased( vgui::MouseCode code, vgui::Panel *panel ) {};
	virtual void mouseWheeled( int delta, vgui::Panel *panel ) {};
	virtual void keyPressed( vgui::KeyCode code, vgui::Panel *panel ) {};
	virtual void keyTyped( vgui::KeyCode code, vgui::Panel *panel ) {};
	virtual void keyReleased( vgui::KeyCode code, vgui::Panel *panel ) {};
	virtual void keyFocusTicked( vgui::Panel *panel ) {};
};

class CMenuHandler_LabelInput : public vgui::InputSignal
{
  private:
	vgui::ActionSignal *m_pActionSignal;

  public:
	CMenuHandler_LabelInput( vgui::ActionSignal *pSignal )
	{
		m_pActionSignal = pSignal;
	}

	virtual void mousePressed( vgui::MouseCode code, vgui::Panel *panel )
	{
		m_pActionSignal->actionPerformed( panel );
	}

	virtual void mouseReleased( vgui::MouseCode code, vgui::Panel *panel ) {};
	virtual void cursorEntered( vgui::Panel *panel ) {};
	virtual void cursorExited( vgui::Panel *Panel ) {};
	virtual void cursorMoved( int x, int y, vgui::Panel *panel ) {};
	virtual void mouseDoublePressed( vgui::MouseCode code, vgui::Panel *panel ) {};
	virtual void mouseWheeled( int delta, vgui::Panel *panel ) {};
	virtual void keyPressed( vgui::KeyCode code, vgui::Panel *panel ) {};
	virtual void keyTyped( vgui::KeyCode code, vgui::Panel *panel ) {};
	virtual void keyReleased( vgui::KeyCode code, vgui::Panel *panel ) {};
	virtual void keyFocusTicked( vgui::Panel *panel ) {};
};


#define HIDE_TEXTWINDOW 0
#define SHOW_MAPBRIEFING 1
#define SHOW_CLASSDESC 2
#define SHOW_MOTD 3
#define SHOW_SPECHELP 4

class CMenuHandler_TextWindow : public vgui::ActionSignal

{
  private:
	int m_iState;

  public:
	CMenuHandler_TextWindow( int iState )
	{
		m_iState = iState;
	}

	virtual void actionPerformed( vgui::Panel *panel )
	{
		if ( m_iState == HIDE_TEXTWINDOW )
		{
			gViewPort->HideTopMenu();
		}
		else
		{
			gViewPort->HideCommandMenu();
			gViewPort->ShowVGUIMenu( m_iState );
		}
	}
};

class CMenuHandler_ToggleCvar : public vgui::ActionSignal
{
  private:
	struct cvar_s *m_cvar;

  public:
	CMenuHandler_ToggleCvar( char *cvarname )
	{
		m_cvar = gEngfuncs.pfnGetCvarPointer( cvarname );
	}

	virtual void actionPerformed( vgui::Panel *panel )
	{
		if ( m_cvar->value )
			m_cvar->value = 0.0f;
		else
			m_cvar->value = 1.0f;

		// hide the menu
		gViewPort->HideCommandMenu();

		gViewPort->UpdateSpectatorPanel();
	}
};

class CMenuHandler_SpectateFollow : public vgui::ActionSignal
{
  protected:
	char m_szplayer[MAX_COMMAND_SIZE];

  public:
	CMenuHandler_SpectateFollow( char *player )
	{
		strncpy( m_szplayer, player, MAX_COMMAND_SIZE );
		m_szplayer[MAX_COMMAND_SIZE - 1] = '\0';
	}

	virtual void actionPerformed( vgui::Panel *panel )
	{
		gHUD.m_Spectator.FindPlayer( m_szplayer );
		gViewPort->HideCommandMenu();
	}
};

#endif // VGUI_COMMANDMENUPANEL_H
