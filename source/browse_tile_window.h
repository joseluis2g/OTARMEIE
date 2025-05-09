//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_BROWSE_TILE_WINDOW_H_
#define RME_BROWSE_TILE_WINDOW_H_

#include "main.h"
#include "map.h"
#include "tile.h"

class BrowseTileListBox;

class BrowseTileWindow : public wxDialog {
public:
	BrowseTileWindow(wxWindow* parent, Tile* tile, wxPoint position = wxDefaultPosition);
	~BrowseTileWindow();

	void OnItemSelected(wxCommandEvent&);
	void OnClickDelete(wxCommandEvent&);
	void OnClickSelectRaw(wxCommandEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void OnClickUp(wxCommandEvent&);
	void OnClickDown(wxCommandEvent&);

protected:
	BrowseTileListBox* item_list;
	wxStaticText* item_count_txt;
	wxButton* delete_button;
	wxButton* select_raw_button;
	wxButton* up_button;
	wxButton* down_button;

	DECLARE_EVENT_TABLE();
};

#endif
