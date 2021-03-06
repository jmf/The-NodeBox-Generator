#include "MenuState.h"
#include "FileParser.h"

MenuState::MenuState(EditorState* state)
:_state(state),
		_projectmb(NULL),
		menubar(NULL)
	{}


void MenuState::init(){
	IGUIEnvironment* guienv = GetState()->GetDevice()->getGUIEnvironment();
	guienv->clear();
	guienv->getSkin()->setFont(guienv->getFont("media/fontlucida.png"));

	// Main menu bar
	menubar=guienv->addMenu();
	menubar->addItem(L"File",-1,true,true);
	menubar->addItem(L"Edit",-1,true,true);
	menubar->addItem(L"View",-1,true,true);
	menubar->addItem(L"Project",-1,true,true);
	menubar->addItem(L"Help",-1,true,true);
	gui::IGUIContextMenu* submenu;

	// File
	submenu = menubar->getSubMenu(0);
	submenu->addItem(L"New Project",GUI_FILE_NEW_PROJECT,false);
	submenu->addItem(L"New Item",GUI_FILE_NEW_ITEM,false);
	submenu->addSeparator();
	submenu->addItem(L"Open Project",GUI_FILE_OPEN_PROJECT);
	submenu->addSeparator();
	submenu->addItem(L"Save Project",GUI_FILE_SAVE_PROJECT);
	submenu->addItem(L"Export",GUI_FILE_EXPORT);
	submenu->addSeparator();
	submenu->addItem(L"Exit",GUI_FILE_EXIT);

	// Edit
	submenu = menubar->getSubMenu(1);
	submenu->addItem(L"Undo",GUI_EDIT_UNDO,false);
	submenu->addItem(L"Redo",GUI_EDIT_REDO,false);
	submenu->addSeparator();
	submenu->addItem(
		L"Snapping",GUI_EDIT_SNAP,true,false,
		GetState()->Settings()->getSettingAsBool("snapping"),
		true
	);
	submenu->addItem(
		L"Limiting",GUI_EDIT_LIMIT,true,false,
		GetState()->Settings()->getSettingAsBool("limiting"),
		true
	);

	// View
	submenu = menubar->getSubMenu(2);
	submenu->addItem(L"Tiled View",GUI_VIEW_SP_ALL);
	submenu->addItem(L"Perspective View",GUI_VIEW_SP_PER);
	submenu->addItem(L"Top View",GUI_VIEW_SP_TOP);
	submenu->addItem(L"Front View",GUI_VIEW_SP_FRT);
	submenu->addItem(L"Side View",GUI_VIEW_SP_RHT);

	// Project
	_projectmb = menubar->getSubMenu(3);

	// Help
	submenu = menubar->getSubMenu(4);
	//submenu->addItem(L"Help",GUI_HELP_HELP,false);
	submenu->addItem(L"About",GUI_HELP_ABOUT);

	// Sidebar root
	u32 top = menubar->getAbsoluteClippingRect().LowerRightCorner.Y;
	_sidebar = guienv->addStaticText(L"Loading...",
		rect<s32>(
			(GetState()->GetDevice()->getVideoDriver()->getScreenSize().Width - 246), top+10,
			GetState()->GetDevice()->getVideoDriver()->getScreenSize().Width, GetState()->GetDevice()->getVideoDriver()->getScreenSize().Height
		),false,true,0,GUI_SIDEBAR_TITLE
	);
	_sidebar->setAlignment(EGUIA_LOWERRIGHT,EGUIA_LOWERRIGHT,EGUIA_UPPERLEFT,EGUIA_UPPERLEFT);
}

bool MenuState::OnEvent(const SEvent& event){
	if (event.EventType == EET_GUI_EVENT){
		if (event.GUIEvent.EventType == EGET_MENU_ITEM_SELECTED){
			IGUIContextMenu* menu = (IGUIContextMenu*)event.GUIEvent.Caller;
			switch (menu->getItemCommandId(menu->getSelectedItem())){
			case GUI_FILE_OPEN_PROJECT:
				{
					NBEFileParser parser(GetState());
					Project* tmp = parser.open("save.nbe");

					if (tmp){
						delete GetState()->project;
						GetState()->project = tmp;
						GetState()->project->SelectNode(0);
						GetState()->Mode()->unload();
						init();
						GetState()->Mode()->load();
					}
					return true;
				}
				break;
			case GUI_FILE_SAVE_PROJECT:
				{
					NBEFileParser parser(GetState());
					parser.save(GetState()->project,"save.nbe");
					return true;
				}
				break;
			case GUI_FILE_EXPORT:
				{
					LUAFileParser parser(GetState());
					parser.save(GetState()->project,"export.lua");
					return true;
				}
				break;
			case GUI_FILE_EXIT:
				{
					IGUIEnvironment* guienv = GetState()->GetDevice()->getGUIEnvironment();
					IGUIWindow* win = guienv->addWindow(rect<irr::s32>(100,100,356,215),true,L"Are you sure?");
					guienv->addButton(rect<irr::s32>(128-40,80,128+40,105),win,GUI_FILE_EXIT,L"Close",L"Close the editor");
					return true;
				}
				break;
			case GUI_EDIT_SNAP:
				{					
					if (menu->isItemChecked(menu->getSelectedItem()))
						GetState()->Settings()->setStringSetting("snapping","true");
					else
						GetState()->Settings()->setStringSetting("snapping","false");

					menu->setItemChecked(menu->getSelectedItem(),GetState()->Settings()->getSettingAsBool("snapping"));
				}
				break;
			case GUI_EDIT_LIMIT:
				{					
					if (menu->isItemChecked(menu->getSelectedItem()))
						GetState()->Settings()->setStringSetting("limiting","true");
					else
						GetState()->Settings()->setStringSetting("limiting","false");

					menu->setItemChecked(menu->getSelectedItem(),GetState()->Settings()->getSettingAsBool("limiting"));
				}
				break;
			case GUI_HELP_ABOUT:
				{
					core::stringw msg = L"The Nodebox Editor\n"
						L"Version: ";

					msg.append(EDITOR_TEXT_VERSION);
					msg.append(
						L"\n\n"
						L"This free nodebox editor was made by rubenwardy in C++ and Irrlicht.\n"
						L"You can download newer versions from the Minetest forum."
					);

					GetState()->GetDevice()->getGUIEnvironment()->addMessageBox(L"About",msg.c_str());
					return true;
				}
				break;
			}	
		}else if(event.GUIEvent.EventType == EGET_BUTTON_CLICKED){
			if (event.GUIEvent.Caller->getID() == GUI_FILE_EXIT){
				NBEFileParser parser(GetState());
				parser.save(GetState()->project,"exit.nbe");
				GetState()->CloseEditor();
				return true;
			}
		}
	}
	return false;
}

void MenuState::draw(IVideoDriver* driver){
	if (GetState()->Settings()->getSettingAsBool("hide_sidebar")){
		GetSideBar()->setVisible(false);
	}else{
		GetSideBar()->setVisible(true);
		u32 top = menubar->getAbsoluteClippingRect().LowerRightCorner.Y;
		GetState()->GetDevice()->getGUIEnvironment()->getSkin()
			->draw3DWindowBackground(NULL,false,0,rect<s32>((driver->getScreenSize().Width - 256),top,driver->getScreenSize().Width,driver->getScreenSize().Height));
		
	}

}
