#ifndef ONLINE_LOBBY_MENU_H
#define ONLINE_LOBBY_MENU_H

void openOnlineBattleConnectFlow(void);
void handleOnlineBattleConnectFlow(void);
void initOnlineLobbyMenu(void);
void updateOnlineLobbyMenu(void);
void handleOnlineLobbyMenuSelection(void);
#ifdef ONLINE_BATTLE_AUTOTEST
void bootOnlineBattleAutotest(void);
#endif

#endif
