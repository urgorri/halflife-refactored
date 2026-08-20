#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_TeamMenuPanel.h"
#include "vgui_ClassMenuPanel.h"
#include "vgui_CommandMenuPanel.h"

void CCommandMenu::AddButton( CommandButton *pButton )
{
	if ( m_iButtons >= MAX_BUTTONS )
		return;

	m_aButtons[m_iButtons] = pButton;
	m_iButtons++;
	pButton->setParent( this );
	pButton->setFont( Scheme::sf_primary3 );

	// give the button a default key binding
	if ( m_iButtons < 10 )
	{
		pButton->setBoundKey( m_iButtons + '0' );
	}
	else if ( m_iButtons == 10 )
	{
		pButton->setBoundKey( '0' );
	}
}

void CCommandMenu::RecalculatePositions( int iYOffset )
{
	int iTop;
	int iAdjust = 0;

	m_iYOffset += iYOffset;

	if ( m_iDirection )
		iTop = ScreenHeight - ( m_iYOffset + _size[1] );
	else
		iTop = m_iYOffset;

	if ( iTop < 0 )
		iTop = 0;

	// Calculate if this is going to fit onscreen, and shuffle it up if it won't
	int iBottom = iTop + _size[1];

	if ( iBottom > ScreenHeight )
	{
		// Move in increments of button sizes
		while ( iAdjust < ( iBottom - ScreenHeight ) )
		{
			iAdjust += m_flButtonSizeY - 1;
		}

		iTop -= iAdjust;

		// Make sure it doesn't move off the top of the screen (the menu's too big to fit it all)
		if ( iTop < 0 )
		{
			iAdjust -= ( 0 - iTop );
			iTop = 0;
		}
	}

	setPos( _pos[0], iTop );

	// We need to force all menus below this one to update their positions now, because they
	// might have submenus riding off buttons in this menu that have just shifted.
	for ( int i = 0; i < m_iButtons; i++ )
		m_aButtons[i]->UpdateSubMenus( iAdjust );
}

void CCommandMenu::MakeVisible( CCommandMenu *pChildMenu )
{
	/*
	    // Push down the button leading to the child menu
	    for (int i = 0; i < m_iButtons; i++)
	    {
	        if ( (pChildMenu != NULL) && (m_aButtons[i]->GetSubMenu() == pChildMenu) )
	        {
	            m_aButtons[i]->setArmed( true );
	        }
	        else
	        {
	            m_aButtons[i]->setArmed( false );
	        }
	    }
	*/

	setVisible( true );
	if ( m_pParentMenu )
		m_pParentMenu->MakeVisible( this );
}

CommandButton *CCommandMenu::FindButtonWithSubmenu( CCommandMenu *pSubMenu )
{
	for ( int i = 0; i < GetNumButtons(); i++ )
	{
		if ( m_aButtons[i]->GetSubMenu() == pSubMenu )
			return m_aButtons[i];
	}

	return NULL;
}

void CCommandMenu::ClearButtonsOfArmedState( void )
{
	for ( int i = 0; i < GetNumButtons(); i++ )
	{
		m_aButtons[i]->setArmed( false );

		if ( m_aButtons[i]->GetSubMenu() )
		{
			m_aButtons[i]->GetSubMenu()->ClearButtonsOfArmedState();
		}
	}
}

void CCommandMenu::RemoveAllButtons( void )
{
	/*
	for(int i=0;i<m_iButtons;i++)
	{
	    CommandButton *pTemp = m_aButtons[i];
	    m_aButtons[i] = NULL;

	    pTemp
	    if(pTemp)
	    {
	        delete(pTemp);
	    }

	}
	*/
	removeAllChildren();
	m_iButtons = 0;
}

bool CCommandMenu::KeyInput( int keyNum )
{
	// loop through all our buttons looking for one bound to keyNum
	for ( int i = 0; i < m_iButtons; i++ )
	{
		if ( !m_aButtons[i]->IsNotValid() )
		{
			if ( m_aButtons[i]->getBoundKey() == keyNum )
			{
				// hit the button
				if ( m_aButtons[i]->GetSubMenu() )
				{
					// open the sub menu
					gViewPort->SetCurrentCommandMenu( m_aButtons[i]->GetSubMenu() );
					return false;
				}
				else
				{
					// run the bound command
					m_aButtons[i]->fireActionSignal();
					return true;
				}
			}
		}
	}

	return false;
}
