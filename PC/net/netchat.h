/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netconnect.h
	Programmer	: 
	Date		:

----------------------------------------------------------------------------------------------- */
#ifndef __NETCHAT_H
#define __NETCHAT_H

enum CHAT_FORMAT {
	CHAT_NORMAL,
	CHAT_SYSTEM,
	CHAT_ERROR,
};

int RunNetChatWindow(HWND);
void ChatShowMessage(const char* string, CHAT_FORMAT f);


#endif