#ifndef RACE_SPLITSCREEN_SELECT_MENU_H
#define RACE_SPLITSCREEN_SELECT_MENU_H

/* GAME MENU cursor indices (gRaceSplitscreenMode while this menu is open). */
#define GAME_MENU_CURSOR_ONLINE_BATTLE 0
#define GAME_MENU_CURSOR_COUNT         6
/*
 * Keep the original list top (-0x60) so row 0 stays on-screen, and use a
 * slightly tighter pitch so six options still clear the description panel.
 */
#define GAME_MENU_ROW_PITCH            0x10
#define GAME_MENU_LIST_BASE_Y          (-0x60)
/* Sentinel: reopen menu on Online Battle after converting race mode → cursor. */
#define GAME_MENU_CURSOR_ONLINE_REOPEN 0xFF

void gameMenuApplyRaceModeToCursor(void);
void returnToRaceSplitscreenSelectMenu(void);
void initRaceSplitscreenSelectMenu(void);
void updateRaceSplitscreenSelectMenu(void);
void handleRaceSplitscreenSelectMenuSelection(void);
void fadeOutRaceSplitscreenSelectMenu(void);

#endif
