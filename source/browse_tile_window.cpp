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

#include "main.h"

#include "map.h"
#include "gui.h"
#include "raw_brush.h"
#include "tile.h"
#include "graphics.h"
#include "gui.h"
#include "browse_tile_window.h"


	/* TASK: Item Reordering Implementation
	 * Current state: Static item list with no reordering capability
	 * 
	 * Required changes:
	 * 1. Add Up/Down buttons to allow item reordering
	 * 2. Implement item index swapping functionality
	 * 3. Update tile item order after reordering
	 * 4. Refresh display after reordering
	 * 
	 * Future enhancement possibility:
	 * - Consider implementing drag-and-drop functionality
	 *   using custom wxVListBox implementation
	 */


// ============================================================================
//

class BrowseTileListBox : public wxVListBox {
public:
	BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile);
	~BrowseTileListBox();

	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const;
	wxCoord OnMeasureItem(size_t index) const;
	Item* GetSelectedItem();
	void RemoveSelected();
	void MoveSelectedUp();
	void MoveSelectedDown();
	Tile* GetEditTile() const { return edit_tile; }

protected:
	void UpdateItems();

	typedef std::map<int, Item*> ItemsMap;
	ItemsMap items;
	Tile* edit_tile;
};

BrowseTileListBox::BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile) :
	wxVListBox(parent, id, wxDefaultPosition, wxSize(200, 180), wxLB_MULTIPLE), edit_tile(tile) {
	UpdateItems();
}

BrowseTileListBox::~BrowseTileListBox() {
	////
}

void BrowseTileListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
	ItemsMap::const_iterator item_iterator = items.find(int(n));
	Item* item = item_iterator->second;

	Sprite* sprite = g_gui.gfx.getSprite(item->getClientID());
	if (sprite) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}

	if (IsSelected(n)) {
		item->select();
		if (HasFocus()) {
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		} else {
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
		}
	} else {
		item->deselect();
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}

	wxString label;
	label << item->getID() << " - " << item->getName();
	dc.DrawText(label, rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrowseTileListBox::OnMeasureItem(size_t n) const {
	return 32;
}

Item* BrowseTileListBox::GetSelectedItem() {
	if (GetItemCount() == 0 || GetSelectedCount() == 0) {
		return nullptr;
	}

	return edit_tile->getTopSelectedItem();
}

void BrowseTileListBox::RemoveSelected() {
	if (GetItemCount() == 0 || GetSelectedCount() == 0) {
		return;
	}

	Clear();
	items.clear();

	// Delete the items from the tile
	ItemVector tile_selection = edit_tile->popSelectedItems(true);
	for (ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); ++iit) {
		delete *iit;
	}

	UpdateItems();
	Refresh();
}

void BrowseTileListBox::UpdateItems() {
	int n = 0;
	for (ItemVector::reverse_iterator it = edit_tile->items.rbegin(); it != edit_tile->items.rend(); ++it) {
		items[n] = (*it);
		++n;
	}

	if (edit_tile->ground) {
		items[n] = edit_tile->ground;
		++n;
	}

	SetItemCount(n);
}

void BrowseTileListBox::MoveSelectedUp() {
	if (GetItemCount() == 0 || GetSelectedCount() == 0) {
		return;
	}

	int selected = GetSelection();
	if (selected <= 0) {
		return;
	}

	// Don't allow moving ground items
	Item* selectedItem = items[selected];
	if (selectedItem == edit_tile->ground) {
		return;
	}

	// Swap items in the tile's item vector
	ItemVector::iterator it = edit_tile->items.begin() + (edit_tile->items.size() - selected - 1);
	if (it != edit_tile->items.end() && (it + 1) != edit_tile->items.end()) {
		std::iter_swap(it, it + 1);
	}

	// Update display
	UpdateItems();
	SetSelection(selected - 1);
	Refresh();
}

void BrowseTileListBox::MoveSelectedDown() {
	if (GetItemCount() == 0 || GetSelectedCount() == 0) {
		return;
	}

	int selected = GetSelection();
	if (selected >= GetItemCount() - 1) {
		return;
	}

	// Swap items in the tile's item vector
	ItemVector::iterator it = edit_tile->items.begin() + (edit_tile->items.size() - selected - 1);
	if (it != edit_tile->items.begin()) {
		std::iter_swap(it, it - 1);
	}

	// Update display
	UpdateItems();
	SetSelection(selected + 1);
	Refresh();
}

// ============================================================================
//

BEGIN_EVENT_TABLE(BrowseTileWindow, wxDialog)
EVT_BUTTON(wxID_REMOVE, BrowseTileWindow::OnClickDelete)
EVT_BUTTON(wxID_FIND, BrowseTileWindow::OnClickSelectRaw)
EVT_BUTTON(wxID_OK, BrowseTileWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, BrowseTileWindow::OnClickCancel)
EVT_BUTTON(wxID_UP, BrowseTileWindow::OnClickUp)
EVT_BUTTON(wxID_DOWN, BrowseTileWindow::OnClickDown)
END_EVENT_TABLE()

BrowseTileWindow::BrowseTileWindow(wxWindow* parent, Tile* tile, wxPoint position /* = wxDefaultPosition */) :
	wxDialog(parent, wxID_ANY, "Browse Field", position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER) {
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	item_list = newd BrowseTileListBox(this, wxID_ANY, tile);
	sizer->Add(item_list, wxSizerFlags(1).Expand());

	wxString pos;
	pos << "x=" << tile->getX() << ",  y=" << tile->getY() << ",  z=" << tile->getZ();

	wxSizer* infoSizer = newd wxBoxSizer(wxVERTICAL);
	wxBoxSizer* buttons = newd wxBoxSizer(wxHORIZONTAL);
	delete_button = newd wxButton(this, wxID_REMOVE, "Delete");
	delete_button->Enable(false);
	buttons->Add(delete_button);
	buttons->AddSpacer(5);
	select_raw_button = newd wxButton(this, wxID_FIND, "Select RAW");
	select_raw_button->Enable(false);
	buttons->Add(select_raw_button);
	wxBoxSizer* move_buttons = newd wxBoxSizer(wxHORIZONTAL);
	up_button = newd wxButton(this, wxID_UP, "Up");
	up_button->Enable(false);
	move_buttons->Add(up_button);
	move_buttons->AddSpacer(5);
	down_button = newd wxButton(this, wxID_DOWN, "Down");
	down_button->Enable(false);
	move_buttons->Add(down_button);
	buttons->Add(move_buttons);
	infoSizer->Add(buttons);
	infoSizer->AddSpacer(5);
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "Position:  " + pos), wxSizerFlags(0).Left());
	infoSizer->Add(item_count_txt = newd wxStaticText(this, wxID_ANY, "Item count:  " + i2ws(item_list->GetItemCount())), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "Protection zone:  " + b2yn(tile->isPZ())), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "No PvP:  " + b2yn(tile->getMapFlags() & TILESTATE_NOPVP)), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "No logout:  " + b2yn(tile->getMapFlags() & TILESTATE_NOLOGOUT)), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "PvP zone:  " + b2yn(tile->getMapFlags() & TILESTATE_PVPZONE)), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "House:  " + b2yn(tile->isHouseTile())), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "Zone brush:  " + b2yn(tile->getMapFlags() & TILESTATE_ZONE_BRUSH)), wxSizerFlags(0).Left());

	sizer->Add(infoSizer, wxSizerFlags(0).Left().DoubleBorder());

	// OK/Cancel buttons
	wxSizer* btnSizer = newd wxBoxSizer(wxHORIZONTAL);
	btnSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(0).Center());
	btnSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(0).Center());
	sizer->Add(btnSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(sizer);

	// Connect Events
	item_list->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(BrowseTileWindow::OnItemSelected), NULL, this);
}

BrowseTileWindow::~BrowseTileWindow() {
	// Disconnect Events
	item_list->Disconnect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(BrowseTileWindow::OnItemSelected), NULL, this);
}

void BrowseTileWindow::OnItemSelected(wxCommandEvent& WXUNUSED(event)) {
	const size_t count = item_list->GetSelectedCount();
	const int selected = item_list->GetSelection();
	
	delete_button->Enable(count != 0);
	select_raw_button->Enable(count == 1);
	
	// Only enable Up button if not ground item
	bool isGroundItem = (selected >= 0 && item_list->GetSelectedItem() == item_list->GetEditTile()->ground);
	up_button->Enable(count == 1 && selected > 0 && !isGroundItem);
	down_button->Enable(count == 1 && selected < item_list->GetItemCount() - 1);
}

void BrowseTileWindow::OnClickDelete(wxCommandEvent& WXUNUSED(event)) {
	item_list->RemoveSelected();
	item_count_txt->SetLabelText("Item count:  " + i2ws(item_list->GetItemCount()));
}

void BrowseTileWindow::OnClickSelectRaw(wxCommandEvent& WXUNUSED(event)) {
	Item* item = item_list->GetSelectedItem();
	if (item && item->getRAWBrush()) {
		g_gui.SelectBrush(item->getRAWBrush(), TILESET_RAW);
	}

	EndModal(1);
}

void BrowseTileWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	EndModal(1);
}

void BrowseTileWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}

void BrowseTileWindow::OnClickUp(wxCommandEvent& WXUNUSED(event)) {
	item_list->MoveSelectedUp();
}

void BrowseTileWindow::OnClickDown(wxCommandEvent& WXUNUSED(event)) {
	item_list->MoveSelectedDown();
}
