#include <wchar.h>
#include <sys/prx.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/ppu_thread.h>
#include <string.h>
#include <cstring>
#include <sys/sys_time.h>
#include <sys/time_util.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/timer.h>
#include <sys/return_code.h>
#include <sys/prx.h>
#include <stddef.h>
#include <math.h>
#include <cmath>
#include <stdarg.h>
#include <cellstatus.h>
#include <typeinfo>
#include <vector>
#include <pthread.h>
#include <locale.h>
#include <cell/error.h>
#include <sys/paths.h>
#include <time.h>

//   -----------------------------   //
//		  INLUDED HEADER FILES       //	
		#include "Button.h"
		#include "Function.h"
		#include "Hud.h"
		#include "PS3.h"
		#include "Verification.h"
//   -----------------------------   //



#define TOC ( 0x00724C38 )

sys_ppu_thread_t MainThread;
sys_ppu_thread_t thrd_Calls;
sys_ppu_thread_t ThreadModuleID;

sys_ppu_thread_t create_thread(void (*entry)(uint64_t), int priority, size_t stacksize, const char* threadname,sys_ppu_thread_t id)
{	
	if(sys_ppu_thread_create(&id, entry, 0, priority , stacksize, 0, threadname) == CELL_OK)
	{	
	}
		
    return id;
}

bool Loaded;
bool ModLoaded[18];
bool MenuOpened[18];
bool StatusForever=false;//fix glitch?
char mapBuf[100];
int Scroll[18];
int MaxScroll[18];
const char* weapon;
char* CurrentMenu[18];
char* ParentMenu[18];
char* setMenuTitle[18];
char* customMessage;
const char* Players[18];
int SelectedClient;
int myIndex;
bool Theme;
//for aimbot
#define Angleoffset (0x3358)
#define BaseAngle (0xE0)
bool Aimbot = true;//true when aimbot enabled
bool Required = false;//true when you have to aim down sight
bool visibleOnly = true;//true if you want it to only check if they are visible
bool shootToEnable = false;//true if r1 makes it work
bool autoShoot = false;//makes you shoot when a player is visible
bool autoAim = false;//makes you aim when a client is visible
bool semiAutoMode = false;
bool isCurrentlyShooting = false;
bool UnfairAimbot = false;//overrides the rest. Kills closest person when you shoot
bool silentAimbot = false;//not really used YET
bool isAiming() {
	return (ButtonMonitoring::ButtonPressed(ButtonMonitoring::L2) && ButtonMonitoring::ButtonPressed(ButtonMonitoring::R2) || ButtonMonitoring::ButtonPressed(ButtonMonitoring::L2)) || (shootToEnable && ButtonMonitoring::ButtonPressed(ButtonMonitoring::R2)) || *(char*)0x0095afe0 == 1 || *(char*)*(int*)0x9FD590 == 1;
}


//   -----------------------------   //
//		     Client Mods			 //
//   -----------------------------   //
void TeleportOutClient(int client)
	{
		if(AccessLevel[client] != Host)
		{
		*(char*)(0x014E2220 + 0x3700 * client) = 0xC5;
		_toc::SV_GameSendServerCommand(SelectedClient, 1, "f \"The Host ^7[^1Sent you far far away...^7]\"");
		char temp[0x50];
		_toc::stdc_snprintf(temp, sizeof(temp), "f \"^2You Sent [^5 %s ^2] far far away...\"", getPlayerNameStr(client));
		_toc::SV_GameSendServerCommand(myIndex, 1, temp);
		}
		else
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1Can't Fuck With the Host\"");
	}


void Kill(int clientIndex) {
	if(AccessLevel[clientIndex] != Host)
		{

			char bytes[2][1] = { { 0x01 }, { 0x00 } };
			write_process(0x01319901 + (clientIndex * 0x280), bytes[0], sizeof(bytes));//kill
			_toc::SV_GameSendServerCommand(clientIndex, 1, "f \"The Host ^7[^1Killed You^7]\"");
			char buf[100];
			_toc::stdc_snprintf(buf, 100, "f \"^2You Killed [^1 %s ^2]", getPlayerNameStr(clientIndex));
			_toc::SV_GameSendServerCommand(myIndex, 1, buf);
			sleep(600);
			write_process(0x01319901 + (clientIndex * 0x280), bytes[1], sizeof(bytes));//stop killing

		}
		else
			_toc::SV_GameSendServerCommand(clientIndex, 1, "f \"^1Can't Fuck With the Host\"");
}


void KickPlayer(int client)
{
	if(AccessLevel[client] != Host)
	{
		char buf[100];
		_toc::stdc_snprintf(buf, 100, "f \"^2You Kicked [^1 %s ^2]", getPlayerNameStr(client));
		_toc::SV_GameSendServerCommand(myIndex, 1, buf);
		_toc::ClientDisconnect(client);
	}
	else
		_toc::SV_GameSendServerCommand(client, 1, "f \"^1Can't Fuck With the Host\"");
}


bool SpawnKilling[18];
void SpawnKill(int clientIndex) {
	if(AccessLevel[clientIndex] != Host)
	{
		SpawnKilling[clientIndex] =! SpawnKilling[clientIndex];
		char bytes[2][1] = { { 0x01 }, { 0x00 } };
		write_process(0x01319901 + (clientIndex * 0x280), SpawnKilling[clientIndex] ? bytes[0] : bytes[1], sizeof(bytes));
		_toc::SV_GameSendServerCommand(clientIndex, 1, SpawnKilling[clientIndex] ? "f \"Spawn Kill [^2On^7]\"" : "f \"Spawn Kill [^1Off^7]\"");
		_toc::SV_GameSendServerCommand(myIndex, 1, SpawnKilling[clientIndex] ? "f \"Spawn Kill [^2On^7]\"" : "f \"Spawn Kill [^1Off^7]\"");
	}
	else
		_toc::SV_GameSendServerCommand(clientIndex, 1, "f \"^1Can't Fuck With the Host\"");
}

//huds::gameHudelem_s* BG[18];
huds::gameHudelem_s* MenuTitle[18];
huds::gameHudelem_s* MenuText[18][99];
//huds::gameHudelem_s* Scrollbar[18];
huds::gameHudelem_s* InfoBox[18];
huds::gameHudelem_s* InfoText1[18];
huds::gameHudelem_s* InfoText2[18];
huds::gameHudelem_s* InfoText3[18];
huds::gameHudelem_s* InfoText4[18];
huds::gameHudelem_s* InfoText5[18];



void MenuHuds(int client)
{
	//BG[client] = huds::setShader(client, "white", 200, 600, 400, 0, 0, 0, 0, 0, 150);
	
	MenuTitle[client] = huds::setText(  client,   "",      3,   2.25,    320, 150, 5, 25, 25, 25, 0, 55, 55, 55, 255);
	
	InfoBox[client] =   huds::setShader(client, "white", 210, 75, 40, 205, 0, 0, 0, 0, 0);
	InfoText1[client] = huds::setText(client, "PlautGaming's Black Ice v2 SPRX!", 3, 1, 145, 490, 5, 255, 255, 255, 0, 17, 255, 255, 255);
	InfoText2[client] = huds::setText(client, "[{+actionslot 1}] / [{+actionslot 2}] to Scroll", 3, 1, 145, 540, 5, 255, 255, 255, 0);
	InfoText3[client] = huds::setText(client, "[{+gostand}] to Select Option", 3, 1, 145, 570, 5, 255, 255, 255, 0);
	InfoText4[client] = huds::setText(client, "[{+usereload}] to go Back / Exit Menu", 3, 1, 145, 600, 5, 255, 255, 255, 255);

		//Scrollbar[client] = huds::setShader(client, "white", 200, 16.5, 150, 50,             0,   0,  0,   0,   165);
	for(int x = 1; x < 21; x++)									      //x,  y
		MenuText[client][x] = huds::setText(client, "",      4,   3,    320,170 + (x * 50), 5, 255, 255, 255, 255, 85, 0, 130, 25);
}

void Scrolling(int client)
{
	for(int x = 1; x < 21; x++)
	{
		MenuTitle[client]->elem.color.r = 85;
		MenuTitle[client]->elem.color.g = 0;
		MenuTitle[client]->elem.color.b = 130;
		MenuTitle[client]->elem.glowColor.a = 255;

		InfoText4[client]->elem.color.r = 255;
		InfoText4[client]->elem.color.g = 255;
		InfoText4[client]->elem.color.b = 255;
		InfoText4[client]->elem.color.a = 255;

		MenuText[client][x]->elem.fontScale = 1.1;
		MenuText[client][x]->elem.color.r = 255;
		MenuText[client][x]->elem.color.g = 255;
		MenuText[client][x]->elem.color.b = 255;
		MenuText[client][x]->elem.glowColor.r = 85;
		MenuText[client][x]->elem.glowColor.r = 0;
		MenuText[client][x]->elem.glowColor.r = 130;
		MenuText[client][x]->elem.glowColor.r = 25;
	}
	//float pos = 70.085 + (21.92 * Scroll[client]);
	//huds::moveOverTime(Scrollbar[client], 0.20, 400, pos);
	MenuText[client][Scroll[client]]->elem.color.r = 85;
	MenuText[client][Scroll[client]]->elem.color.g = 0;
	MenuText[client][Scroll[client]]->elem.color.b = 130;
	sleep(50);
	huds::scaleOverTimeText(MenuText[client][Scroll[client]], 0.20, .5);
	huds::glowText(MenuText[client][Scroll[client]], 0.35, 25, 255, 255, 255);
	huds::scaleOverTimeText(MenuText[client][Scroll[client]], 0.20, 1.6);
	huds::glowText(MenuText[client][Scroll[client]], 0.20, 85, 0, 130, 255);

	MenuText[client][Scroll[client]]->elem.color.r = 25;
	MenuText[client][Scroll[client]]->elem.color.g = 255;
	MenuText[client][Scroll[client]]->elem.color.b = 255;
	MenuText[client][Scroll[client]]->elem.glowColor.a = 200;
	
}

void Close(int client)
{
	_toc::SV_GameSendServerCommand(client, 1, "v r_blur \"0\" cg_drawCrosshair \"1\"");
	_toc::SV_GameSendServerCommand(client, 1, mapBuf);//reset vision
	freezeControls(client,false);
	//huds::fadeOverTime(BG[client], 0.50, 0, 0, 0, 0);
	//huds::fadeOverTime(Scrollbar[client], 0.50, 0, 0, 0, 0);
	huds::fadeOverTime(InfoBox[client], 0.50, 0, 0, 0, 0);
	huds::fadeOverTime(InfoText1[client], 0.50, 0, 0, 0, 0);
	huds::fadeOverTime(InfoText2[client], 0.50, 0, 0, 0, 0);
	huds::fadeOverTime(InfoText3[client], 0.50, 0, 0, 0, 0);
	huds::fadeOverTime(InfoText4[client], 0.50, 0, 0, 0, 0);
	huds::fadeOverTime(MenuTitle[client], 0.50, 0, 0, 0, 0);
	for(int x = 1; x < 21; x++)
		huds::fadeOverTime(MenuText[client][x], 0.50, 0, 0, 0, 0);
}
void Open(int client)
{
	_toc::SV_GameSendServerCommand(client, 1, "v r_blur \"5\" cg_drawCrosshair \"0\"");
	if(Theme)
		_toc::SV_GameSendServerCommand(client, 1, "Q cheat_invert_contrast");
	else
		_toc::SV_GameSendServerCommand(client, 1, "Q ac130_inverted");
	myIndex = client;
	freezeControls(client, true);
	//huds::fadeOverTime(BG[client], 0.50, 0, 0, 0, 140);
	//huds::fadeOverTime(Scrollbar[client], 0.50, 0, 0, 0, 170);
	huds::fadeOverTime(InfoBox[client], 0.50, 0, 0, 0, 140);
	huds::fadeOverTime(InfoText1[client], 0.50, 255, 255, 255, 255);
	huds::fadeOverTime(InfoText2[client], 0.50, 255, 255, 255, 255);
	huds::fadeOverTime(InfoText3[client], 0.50, 255, 255, 255, 255);
	huds::fadeOverTime(InfoText4[client], 0.50, 255, 255, 255, 255);
	huds::fadeOverTime(MenuTitle[client], 0.50, 25, 25, 25, 255);
	for(int x = 1; x < 21; x++)
			huds::fadeOverTime(MenuText[client][x], 0.50, 255, 255, 255, 255);
}

void setMenuList(int client)
{
	if(CurrentMenu[client] == "main")
	{
		Scroll[client] = 1;
		MaxScroll[client] = 10;
		huds::ChangeText(MenuTitle[client], "Black Ice v2");
		huds::ChangeText(MenuText[client][1], "Main Mods");
		huds::ChangeText(MenuText[client][2], "Fun Menu");
		huds::ChangeText(MenuText[client][3], "Lobby Menu");
		huds::ChangeText(MenuText[client][4], "Message Menu");
		huds::ChangeText(MenuText[client][5], "All Players");
		huds::ChangeText(MenuText[client][6], "Weapons");
		huds::ChangeText(MenuText[client][7], "Account");
		huds::ChangeText(MenuText[client][8], "Aimbot Menu");
		huds::ChangeText(MenuText[client][9], "Menu Settings");
		huds::ChangeText(MenuText[client][10], "Players Menu");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
	}
	else if(CurrentMenu[client] == "Main Mods")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 9;
		huds::ChangeText(MenuTitle[client], "Main Mods");
		huds::ChangeText(MenuText[client][1], "God Mode");
		huds::ChangeText(MenuText[client][2], "No Clip");
		huds::ChangeText(MenuText[client][3], "Infinite Ammo");
		huds::ChangeText(MenuText[client][4], "Little Crosshair");
		huds::ChangeText(MenuText[client][5], "No Recoil");
		huds::ChangeText(MenuText[client][6], "Red Boxes");
		huds::ChangeText(MenuText[client][7], "Explosive Bullets");
		huds::ChangeText(MenuText[client][8], "Aim Assist");
		huds::ChangeText(MenuText[client][9], "Change Name");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
		else if(AccessLevel[client] == Vip, Verified)
		{
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1You Need a Higher Access Level\"");
		}
	}
	else if(CurrentMenu[client] == "Fun Menu")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin || AccessLevel[client] == Vip || AccessLevel[client] == Verified)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 5;
		huds::ChangeText(MenuTitle[client], "Fun Menu");
		huds::ChangeText(MenuText[client][1], "Double Run Speed");
		huds::ChangeText(MenuText[client][2], "FOV");
		huds::ChangeText(MenuText[client][3], "Pro Skater");
		huds::ChangeText(MenuText[client][4], "Third Person");
		huds::ChangeText(MenuText[client][5], "Left Hand Gun");
		huds::ChangeText(MenuText[client][6], "");
		huds::ChangeText(MenuText[client][7], "");
		huds::ChangeText(MenuText[client][8], "");
		huds::ChangeText(MenuText[client][9], "");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
	}
	else if(CurrentMenu[client] == "Lobby Menu")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 9;
		huds::ChangeText(MenuTitle[client], "Lobby Menu");
		huds::ChangeText(MenuText[client][1], "High Jump");
		huds::ChangeText(MenuText[client][2], "Super Speed");
		huds::ChangeText(MenuText[client][3], "Force Host");
		huds::ChangeText(MenuText[client][4], "Super Force Host");
		huds::ChangeText(MenuText[client][5], "Gravity");
		huds::ChangeText(MenuText[client][6], "TimeScale");
		huds::ChangeText(MenuText[client][7], "Super Melee");
		huds::ChangeText(MenuText[client][8], "Allow Endgame");
		huds::ChangeText(MenuText[client][9], "Prohibit Endgame");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
		else if(AccessLevel[client] == Verified || AccessLevel[client] == Admin || AccessLevel[client] == Vip)
		{
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");
		}
	}
	else if(CurrentMenu[client] == "Message Menu")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin || AccessLevel[client] == Vip || AccessLevel[client] == Verified)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 5;
		huds::ChangeText(MenuTitle[client], "Message Menu");
		huds::ChangeText(MenuText[client][1], "Credits");
		huds::ChangeText(MenuText[client][2], "Fuck You Bitch");
		huds::ChangeText(MenuText[client][3], "TrickShot Last");
		huds::ChangeText(MenuText[client][4], "OMG that's Legitness!");
		huds::ChangeText(MenuText[client][5], "Custom Message");
		huds::ChangeText(MenuText[client][6], "");
		huds::ChangeText(MenuText[client][7], "");
		huds::ChangeText(MenuText[client][8], "");
		huds::ChangeText(MenuText[client][9], "");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
	}
	else if(CurrentMenu[client] == "All Players")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 6;
		huds::ChangeText(MenuTitle[client], "All Players");
		huds::ChangeText(MenuText[client][1], "GodMode");
		huds::ChangeText(MenuText[client][2], "No Clip");
		huds::ChangeText(MenuText[client][3], "Red Boxes");
		huds::ChangeText(MenuText[client][4], "Third Person");
		huds::ChangeText(MenuText[client][5], "FOV");
		huds::ChangeText(MenuText[client][6], "Infinite Ammo");
		huds::ChangeText(MenuText[client][7], "");
		huds::ChangeText(MenuText[client][8], "");
		huds::ChangeText(MenuText[client][9], "");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
		else if(AccessLevel[client] == Verified || AccessLevel[client] == Admin || AccessLevel[client] == Vip)
		{
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");
		}
	}
	else if(CurrentMenu[client] == "Weapons")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin || AccessLevel[client] == Vip)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 5;
		huds::ChangeText(MenuTitle[client], "Weapons");
		huds::ChangeText(MenuText[client][1], "AC130");
		huds::ChangeText(MenuText[client][2], "Intervention");
		huds::ChangeText(MenuText[client][3], "Barrett");
		huds::ChangeText(MenuText[client][4], "Gold Deagle");
		huds::ChangeText(MenuText[client][5], "Default Weapon");
		huds::ChangeText(MenuText[client][6], "");
		huds::ChangeText(MenuText[client][7], "");
		huds::ChangeText(MenuText[client][8], "");
		huds::ChangeText(MenuText[client][9], "");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
		else if(AccessLevel[client] == Verified)
		{
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1You Need a Higher Access Level\"");
		}
	}
	else if(CurrentMenu[client] == "Account")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin || AccessLevel[client] == Vip)
		{
		Scroll[client] = 1;
		MaxScroll[client] = 4;
		huds::ChangeText(MenuTitle[client], "Account");
		huds::ChangeText(MenuText[client][1], "Level 70");
		huds::ChangeText(MenuText[client][2], "Unlock All");
		huds::ChangeText(MenuText[client][3], "Toggle Prestige");
		huds::ChangeText(MenuText[client][4], "^5B^7lack^5I^7ce ClanTag {BI}");
		huds::ChangeText(MenuText[client][5], "");
		huds::ChangeText(MenuText[client][6], "");
		huds::ChangeText(MenuText[client][7], "");
		huds::ChangeText(MenuText[client][8], "");
		huds::ChangeText(MenuText[client][9], "");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
		}
		else if(AccessLevel[client] == Verified)
		{
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1You Need a Higher Access Level\"");
		}
	}
	else if(CurrentMenu[client] == "Players Menu")
	{
		if(AccessLevel[client] == Host || isHost(client) || client == host)
		{
		Scroll[client] = 1;
		huds::ChangeText(MenuTitle[client], "Players Menu");
		int j = 1;
		for( int i = 0; i < 18; i++)
		{
			Players[i] = (char*)0x014E5408 + (i * 0x3700);
			if(strcmp(Players[i], ""))
			{
					char dest[0x50];
					_toc::stdc_snprintf(dest, sizeof(dest), "[%s]%s", getAccessLevelStr(i), Players[i]);
					huds::ChangeText(MenuText[client][j], dest);
					MaxScroll[client] = j;
					j++;
			}
			for (int k = j; k < 21; k++)
			{
				huds::ChangeText(MenuText[client][k], "");
			}
		}
		}
		else if(AccessLevel[client] == Admin, Vip, Verified)
		{
			_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");
		}
	}
	else if(CurrentMenu[client] == "Menu Settings")
	{
		Scroll[client] = 1;
		MaxScroll[client] = 2;
		huds::ChangeText(MenuTitle[client], "Menu Settings");
		huds::ChangeText(MenuText[client][1], "Default Theme");
		huds::ChangeText(MenuText[client][2], "Cheat Theme");
		huds::ChangeText(MenuText[client][3], "");
		huds::ChangeText(MenuText[client][4], "");
		huds::ChangeText(MenuText[client][5], "");
		huds::ChangeText(MenuText[client][6], "");
		huds::ChangeText(MenuText[client][7], "");
		huds::ChangeText(MenuText[client][8], "");
		huds::ChangeText(MenuText[client][9], "");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");

	}
	else if(CurrentMenu[client] == "client")
	{
		Scroll[client] = 1;
		MaxScroll[client] = 13;
		char temp[100];
		_toc::stdc_snprintf(temp, sizeof(temp), "Do What to %s?", (char*)0x014E5408 + (SelectedClient * 0x3700));
		huds::ChangeText(MenuTitle[client], temp);
		huds::ChangeText(MenuText[client][1], "Kill");
		huds::ChangeText(MenuText[client][2], "Kick");
		huds::ChangeText(MenuText[client][3], "Remove Access");
		huds::ChangeText(MenuText[client][4], "Give Verified");
		huds::ChangeText(MenuText[client][5], "Give VIP");
		huds::ChangeText(MenuText[client][6], "Give Admin");
		huds::ChangeText(MenuText[client][7], "Level 70");
		huds::ChangeText(MenuText[client][8], "Unlock All");
		huds::ChangeText(MenuText[client][9], "Spawn Kill Loop");
		huds::ChangeText(MenuText[client][10], "Teleport Far Away(Might Kill)");
		huds::ChangeText(MenuText[client][11], "Aim Assist");
		huds::ChangeText(MenuText[client][12], "God Mode");
		huds::ChangeText(MenuText[client][13], "Change Name");
		huds::ChangeText(MenuText[client][14], "");
		huds::ChangeText(MenuText[client][15], "");
		huds::ChangeText(MenuText[client][16], "");
		huds::ChangeText(MenuText[client][17], "");
		huds::ChangeText(MenuText[client][18], "");
		huds::ChangeText(MenuText[client][19], "");
		huds::ChangeText(MenuText[client][20], "");
	}
	else if(CurrentMenu[client] == "Aimbot Menu")
	{
		Scroll[client] = 1;
		MaxScroll[client] =9;
		huds::ChangeText(MenuTitle[client], "Aimbot Menu");
	
		huds::ChangeText(MenuText[client][1], Aimbot ? "^2ON:^7Toggle Aimbot" : "^1OFF:^7Toggle Aimbot");
		huds::ChangeText(MenuText[client][2], Required ? "^2ON:^7Aiming Required" : "^1OFF:^7Aiming Required");
		huds::ChangeText(MenuText[client][3], visibleOnly ? "^2ON:^7Must Be Visible" : "^1OFF:^7Must Be Visible");
		huds::ChangeText(MenuText[client][4], shootToEnable ? "^2ON:^7Shoot To Enable" : "^1OFF:^7Shoot To Enable");
		huds::ChangeText(MenuText[client][5], autoShoot ? "^2ON:^7Autoshoot" : "^1OFF:^7Autoshoot");
		huds::ChangeText(MenuText[client][6], semiAutoMode ? "^2ON:^7Semi Auto Mode" : "^1OFF:^7Semi Auto Mode");
		huds::ChangeText(MenuText[client][7], autoAim ? "^2ON:^7Auto Aim" : "^1OFF:^7Auto Aim");
		huds::ChangeText(MenuText[client][8], UnfairAimbot ? "^2ON:^7Unfair Aimbot" : "^1OFF:^7Unfair Aimbot");
		huds::ChangeText(MenuText[client][9], silentAimbot ? "^2ON:^7Silent Aimbot" : "^1OFF:^7Silent Aimbot");
		huds::ChangeText(MenuText[client][10], "");
		huds::ChangeText(MenuText[client][11], "");
		huds::ChangeText(MenuText[client][12], "");
		huds::ChangeText(MenuText[client][13], "");
	}
}

void loadMenu(int client, char* menu, char* parent)
{
	CurrentMenu[client] = menu;
	ParentMenu[client] = parent;
	setMenuList(client);
	Scrolling(client);
}
void reloadAimbotMenu(int client)
{
	Close(client);//reload
	Open(client);
	loadMenu(client, "Aimbot Menu", "main");
}
void setAccess(int Owner, int Target, Verification Accesslevel)
{
	if (host == Target)
			_toc::iPrintlnBold(Owner, "^2You cannot change the Accesslevel of the Host!");
	else
	{
			if (isAllowed(Target))
					Close(Target);
			AccessLevel[Target] = Accesslevel;
			char temp[0x50];
			_toc::stdc_snprintf(temp, sizeof(temp), "^2You have changed %s Accesslevel to: %s", getClientName(Target), AccessLevel);
			_toc::iPrintln(Owner, temp);
			_toc::stdc_snprintf(temp, sizeof(temp), "^2Your Accesslevel has been changed to %s by %s", AccessLevel, getClientName(Owner));
	}
}

void removeAccess(int Target, Verification Accesslevel)
{
	AccessLevel[Target] = Accesslevel;
}

void getMenuFuncList(int client)
{
	if(CurrentMenu[client] == "main")
	{
		if(Scroll[client]==1)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin)
				loadMenu(client, "Main Mods", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1You Need a Higher Access Level\"");
		}
		else if(Scroll[client]==2)
		{
			loadMenu(client, "Fun Menu", "main");
		}
		else if(Scroll[client]==3)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host)
				loadMenu(client, "Lobby Menu", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");
		}
		else if(Scroll[client]==4)
		{
			loadMenu(client, "Message Menu", "main");
		}
		else if(Scroll[client]==5)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host)
				loadMenu(client, "All Players", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");
		}
		else if(Scroll[client]==6)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin || AccessLevel[client] == Vip)
				loadMenu(client, "Weapons", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1You Need a Higher Access Level\"");
		}
		else if(Scroll[client]==7)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host || AccessLevel[client] == Admin || AccessLevel[client] == Vip)
				loadMenu(client, "Account", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1You Need a Higher Access Level\"");
		}
		else if(Scroll[client]==8)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host)
				loadMenu(client, "Aimbot Menu", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");
		}
		else if(Scroll[client]==9)
		{
			loadMenu(client, "Menu Settings", "main");
		}
		else if(Scroll[client]==10)
		{
			if(AccessLevel[client] == Host || isHost(client) || client == host)
				loadMenu(client, "Players Menu", "main");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1Only the Host Can Access This Menu\"");

		}
	}
	else if(CurrentMenu[client] == "Main Mods")
	{
		if(Scroll[client]==1)
		{
			GodMode(client);
		}
		else if(Scroll[client]==2)
		{
			NoClip(client);
		}
		else if(Scroll[client]==3)
		{
			UnlimitedAmmo(client);
		}
		else if(Scroll[client]==4)
		{
			LittleCrosshair(client);
		}
		else if(Scroll[client]==5)
		{
			NoRecoil(client);
		}
		else if(Scroll[client]==6)
		{
			RedBoxes(client);
		}
		else if(Scroll[client]==7)
		{
			ExplosiveBullets(client);
		}
		else if(Scroll[client]==8)
		{
			ToggleAim(client);
		}
		else if(Scroll[client] == 9)
		{
			ChangeName(client);
		}
	}
	else if(CurrentMenu[client] == "Fun Menu")
	{
		if(Scroll[client]==1)
		{
			DoubleRunSpeed(client);
		}
		else if(Scroll[client]==2)
		{
			FOV(client);
		}
		else if(Scroll[client]==3)
		{
			SkateMod(client);
		}
		else if(Scroll[client]==4)
		{
			ThirdPerson(client);
		}
		else if(Scroll[client]==5)
		{
			LeftGun(client);
		}
	}
	else if(CurrentMenu[client] == "Lobby Menu")
	{
		if(Scroll[client]==1)
		{
			SuperJump(client);
		}
		else if(Scroll[client]==2)
		{
			SuperSpeed(client);
		}
		else if(Scroll[client]==3)
		{
			ForceHost(client);
		}
		else if(Scroll[client]==4)
		{
			SuperForceHost(client);
		}
		else if(Scroll[client]==5)
		{
			Gravity(client);
		}
		else if(Scroll[client]==6)
		{
			Timescale(client);
		}
		else if(Scroll[client]==7)
		{
			SuperMelee(client);
		}
		else if(Scroll[client]==8)
		{
			*(char*)0x007EDCA4 = 0x65;
			_toc::SV_GameSendServerCommand(client, 1, "f \"Ending Game: ^2Allowed\"");
		}
		else if(Scroll[client]==9)
		{
			*(char*)0x007EDCA4 = 0x00;
			_toc::SV_GameSendServerCommand(client, 1, "f \"Ending Game: ^1Prohibited\"");
		}
	}
	else if(CurrentMenu[client] == "Message Menu")
	{
		if(Scroll[client]==1)
		{
			customMessage = Creators();
			huds::SetMessage(client, customMessage, " ", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
		}
		else if(Scroll[client]==2)
		{
			customMessage = FuckYou();
			huds::SetMessage(client, customMessage, " ", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
		}
		else if(Scroll[client]==3)
		{
			customMessage = TrickShot();
			huds::SetMessage(client, customMessage, " ", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
		}
		else if(Scroll[client]==4)
		{
			customMessage = Legitness();
			huds::SetMessage(client, customMessage, " ", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
		}
		else if(Scroll[client]==5)
		{
			customMessage = CustomMessage();
			huds::SetMessage(client, customMessage, " ", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
		}
	}
	else if(CurrentMenu[client] == "All Players")
	{
		if(Scroll[client]==1)
		{
			GodMode(0);
			GodMode(1);
			GodMode(2);
			GodMode(3);
			GodMode(4);
			GodMode(5);
			GodMode(6);
			GodMode(7);
			GodMode(8);
			GodMode(9);
			GodMode(10);
			GodMode(11);
			GodMode(12);
			GodMode(13);
			GodMode(14);
			GodMode(15);
			GodMode(16);
			GodMode(17);
		}
		else if(Scroll[client]==2)
		{
			NoClip(0);
			NoClip(1);
			NoClip(2);
			NoClip(3);
			NoClip(4);
			NoClip(5);
			NoClip(6);
			NoClip(7);
			NoClip(8);
			NoClip(9);
			NoClip(10);
			NoClip(11);
			NoClip(12);
			NoClip(13);
			NoClip(14);
			NoClip(15);
			NoClip(16);
			NoClip(17);
		}
		else if(Scroll[client]==3)
		{
			RedBoxes(0);
			RedBoxes(1);
			RedBoxes(2);
			RedBoxes(3);
			RedBoxes(4);
			RedBoxes(5);
			RedBoxes(6);
			RedBoxes(7);
			RedBoxes(8);
			RedBoxes(9);
			RedBoxes(10);
			RedBoxes(11);
			RedBoxes(12);
			RedBoxes(13);
			RedBoxes(14);
			RedBoxes(15);
			RedBoxes(16);
			RedBoxes(17);
		}
		else if(Scroll[client]==4)
		{
			ThirdPerson(0);
			ThirdPerson(1);
			ThirdPerson(2);
			ThirdPerson(3);
			ThirdPerson(4);
			ThirdPerson(5);
			ThirdPerson(6);
			ThirdPerson(7);
			ThirdPerson(8);
			ThirdPerson(9);
			ThirdPerson(10);
			ThirdPerson(11);
			ThirdPerson(12);
			ThirdPerson(13);
			ThirdPerson(14);
			ThirdPerson(15);
			ThirdPerson(16);
			ThirdPerson(17);
		}
		else if(Scroll[client]==5)
		{
			FOV(0);
			FOV(1);
			FOV(2);
			FOV(3);
			FOV(4);
			FOV(5);
			FOV(6);
			FOV(7);
			FOV(8);
			FOV(9);
			FOV(10);
			FOV(11);
			FOV(12);
			FOV(13);
			FOV(14);
			FOV(15);
			FOV(16);
			FOV(17);
		}
		else if(Scroll[client]==6)
		{
			UnlimitedAmmo(0);
			UnlimitedAmmo(1);
			UnlimitedAmmo(2);
			UnlimitedAmmo(3);
			UnlimitedAmmo(4);
			UnlimitedAmmo(5);
			UnlimitedAmmo(6);
			UnlimitedAmmo(7);
			UnlimitedAmmo(8);
			UnlimitedAmmo(9);
			UnlimitedAmmo(10);
			UnlimitedAmmo(11);
			UnlimitedAmmo(12);
			UnlimitedAmmo(13);
			UnlimitedAmmo(14);
			UnlimitedAmmo(15);
			UnlimitedAmmo(16);
			UnlimitedAmmo(17);
		}
	}
	else if(CurrentMenu[client] == "Weapons")
	{
		if(Scroll[client]==1)
		{
			GiveAC130(client);
		}
		else if(Scroll[client]==2)
		{
			GiveI(client);
		}
		else if(Scroll[client]==3)
		{
			GiveB(client);
		}
		else if(Scroll[client]==4)
		{
			GiveG(client);
		}
		else if(Scroll[client]==5)
		{
			GiveD(client);
		}
	}
	else if(CurrentMenu[client] == "Account")
	{
		if(Scroll[client]==1)
		{
			level(client);
		}
		else if(Scroll[client]==2)
		{
			UnlockAll(client);
		}
		else if(Scroll[client]==3)
		{
			Prestige(client);
		}
		else if(Scroll[client]==4)
		{
			IceClan(client);
		}
	}
	else if(CurrentMenu[client] == "Players Menu")
	{
		if(Scroll[client]==1)
		{
			SelectedClient = 0;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==2)
		{
			SelectedClient = 1;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==3)
		{
			SelectedClient = 2;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==4)
		{
			SelectedClient = 3;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==5)
		{
			SelectedClient = 4;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==6)
		{
			SelectedClient = 5;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==7)
		{
			SelectedClient = 6;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==8)
		{
			SelectedClient = 7;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==9)
		{
			SelectedClient = 8;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==10)
		{
			SelectedClient = 9;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==11)
		{
			SelectedClient = 10;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==12)
		{
			SelectedClient = 11;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==13)
		{
			SelectedClient = 12;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==14)
		{
			SelectedClient = 13;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==15)
		{
			SelectedClient = 14;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==16)
		{
			SelectedClient = 15;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==17)
		{
			SelectedClient = 16;
			loadMenu(client, "client", "Players Menu");
		}
		else if(Scroll[client]==18)
		{
			SelectedClient = 17;
			loadMenu(client, "client", "Players Menu");
		}
	}
	else if(CurrentMenu[client] == "Menu Settings")
	{
		if(Scroll[client]==1)
		{
			Theme = false;

			Close(client);//reload
			Open(client);
			loadMenu(client, "Menu Settings", "main");

			_toc::SV_GameSendServerCommand(client, 1, "f \"^2Default Theme Applied\"");
		}
		else if(Scroll[client]==2)
		{
			Theme = true;

			Close(client);//reload menu
			Open(client);
			loadMenu(client, "Menu Settings", "main");

			_toc::SV_GameSendServerCommand(client, 1, "f \"^2Cheat Theme Applied\"");
		}
	}
	else if(CurrentMenu[client] == "client")
	{
		if(Scroll[client]==1)
		{
			Kill(SelectedClient);
		}
		else if(Scroll[client]==2)
		{
			KickPlayer(SelectedClient);
		}
		else if(Scroll[client]==3)
		{
			if(MenuOpened[SelectedClient])
			{
				Close(SelectedClient);
				MenuOpened[SelectedClient] = false;
			}
			removeAccess(SelectedClient, None);
			Kill(SelectedClient);
		}
		else if(Scroll[client]==4)
		{
			if(MenuOpened[SelectedClient])
			{
				Close(SelectedClient);
				MenuOpened[SelectedClient] = false;
			}
			setAccess(client, SelectedClient, Verified);
			Kill(SelectedClient);
		}
		else if(Scroll[client]==5)
		{
			if(MenuOpened[SelectedClient])
			{
				Close(SelectedClient);
				MenuOpened[SelectedClient] = false;
			}
			setAccess(client, SelectedClient, Vip);
			Kill(SelectedClient);
		}
		else if(Scroll[client]==6)
		{
			if(MenuOpened[SelectedClient])
			{
				Close(SelectedClient);
				MenuOpened[SelectedClient] = false;
			}
			setAccess(client, SelectedClient, Admin);
			Kill(SelectedClient);
		}
		else if(Scroll[client]==7)
		{
			level(SelectedClient);
		}
		else if(Scroll[client]==8)
		{
			UnlockAll(SelectedClient);
		}
		else if(Scroll[client]==9)
		{
			SpawnKill(SelectedClient);
		}
		else if(Scroll[client]==10)
		{
			//TeleportOutClient(SelectedClient);
			_toc::SV_GameSendServerCommand(host, 1, "f \"^2Testing Unfair Aimbot\"");
			sleep(1000);
			//Player_Die(int* self, int* inflictor, int* attacker, int damage, int meansOfDeath, int WeaponIndex, const float *vDir, int hitLocation, int psTime)
			Player_Die((int*)G_Entity(SelectedClient),(int*)G_Entity(host),(int*)G_Entity(host), 0x3C, 2, getCurrentWeapon(host), (float*)0xD014ABF0, 0x1BF6000, 0x10009220);
		}
		else if(Scroll[client]==11)
		{
			//ToggleAim(SelectedClient);
			_toc::SV_GameSendServerCommand(host, 1, "f \"^2Testing Unfair Aimbot\"");
			sleep(1000);
			//Player_Die(int* self, int* inflictor, int* attacker, int damage, int meansOfDeath, int WeaponIndex, const float *vDir, int hitLocation, int psTime)
			//Player_Die((int*)G_Entity(SelectedClient),(int*)G_Entity(host),(int*)G_Entity(host), 0x3C, 2, getCurrentWeapon(host), (float*)0xD014ABF0, 0x1BF6000, 0x10009220);

			//G_DamageClient(int *target, int *inflictor, int *attacker, const float *dir, const float *point, int damage, int dflags, int mod, int weapon, int hitLoc, int timeOffset)
			//G_DamageClient(&victim, &killer, &killer, (float*)0xD014ABF0, (float*)0xD014ACB4, 0x3C, 2, 2, 0x10A, 0x1BF6000, 0x10009220);
			G_DamageClient((int*)G_Entity(SelectedClient),(int*)G_Entity(host),(int*)G_Entity(host), (float*)0xD014ABF0, (float*)0xD014ACB4, 0x3C, 2, 2, getCurrentWeapon(host), 0x1BF6000, 0x10009220);
		}
		else if(Scroll[client]==12)
		{
			GodMode(SelectedClient);
		}
		else if(Scroll[client]==13)
		{
			ChangeClientName(SelectedClient);			
		}
	}
	else if(CurrentMenu[client] == "Aimbot Menu")
	{
		if(Scroll[client]==1)
		{
			Aimbot = !Aimbot;
			if(Aimbot)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Aimbot Enabled\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Aimbot Disabled\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==2)
		{
			Required = !Required;
			if(Required)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Aiming Required\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Aiming Not Required\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==3)
		{
			visibleOnly = !visibleOnly;
			if(visibleOnly)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Visible Only\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Visible Only\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==4)
		{
			shootToEnable = !shootToEnable;
			if(shootToEnable)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Shoot to Enable\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Shoot to Enable\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==5)
		{
			if(autoShoot)
			{
				if(autoAim)
				{
					autoAim = false;
					_toc::SV_GameSendServerCommand(client, 1, "f \"^2SAVED YOU!\"");
					_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Auto Aim\"");
				}
				autoShoot = false;
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Auto Shoot\"");
			}
			else
			{
				autoShoot = true;
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Auto Shoot\"");
			}

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==6)
		{
			semiAutoMode = !semiAutoMode;
			if(semiAutoMode)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Semi Auto Mode\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Semi Auto Mode\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==7)
		{
			if(autoShoot)
			{
				autoAim = !autoAim;
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Auto Aim\"");
			}
			else if(!autoShoot)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1Turn ^2ON ^5Auto Shoot ^1First!\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Auto Aim\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==8)
		{
			UnfairAimbot = !UnfairAimbot;
			if(UnfairAimbot)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Unfair Aimbot\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Unfair Aimbot\"");

			reloadAimbotMenu(client);
		}
		else if(Scroll[client]==9)
		{
			silentAimbot = !silentAimbot;
			if(silentAimbot)
				_toc::SV_GameSendServerCommand(client, 1, "f \"^2ON:^7Silent Aimbot\"");
			else
				_toc::SV_GameSendServerCommand(client, 1, "f \"^1OFF:^7Silent Aimbot\"");

			reloadAimbotMenu(client);
		}
	}
}










//======================================//
//                                      //
//                                      //
//===============Aimbot=================//
//                                      //
//                                      //
//======================================//
//0x000135C0 AimTarget_IsTargetVisible
//0x00048600 CG_CanSeeFriendlyHead
//Original Aimbot By NickBeHaxing & Milky4444
#define CEX 0
int write_process1(void* destination, const void* source, size_t size)
{
#if CEX == 0
	system_call_4(905, (uint64_t)sys_process_getpid(), (uint64_t)destination, size, (uint64_t)source);//dex only
	return_to_user_prog(int);
#else
	memcpy(destination,source,size);
#endif
}
void PatchInJump(int Address, int Destination, bool Linked) 
{
	// use this data to copy over the address
	int FuncBytes[4];

	// Get the actual destination address
	Destination = *(int *)Destination;

	FuncBytes[0] = 0x3D600000 + ((Destination >> 16) & 0xFFFF); // lis     %r11, dest>>16
	if (Destination & 0x8000) // If bit 16 is 1
		FuncBytes[0] += 1;

		FuncBytes[1] = 0x396B0000 + (Destination & 0xFFFF); // addi    %r11, %r11, dest&0xFFFF
		FuncBytes[2] = 0x7D6903A6; // mtctr    %r11

		FuncBytes[3] = 0x4E800420; // bctr

		if (Linked)
			FuncBytes[3] += 1; // bctrl

		write_process1((void*)Address, FuncBytes, 4 * 4);
}
struct vec3
{
	float x;
	float y;
	float z;
};
bool killclients[20];
	void killClient(int clientIndex) {
		//killclients[clientIndex] = true;
		//vec3 *v = (vec3*)(G_Client(clientIndex)+0x1C);
		//v->z = (unsigned int)0xffffffff;
		*(short*)(G_Client(clientIndex)+0x24) = 0xFFFF;
	}
int GetIndex()
{
	int cg_s = *(int*)(0x915254);
	if (cg_s == 0)
		return -1;
	return *(int*)(cg_s + 0x3250);
}
bool isInGameOrLoading() {
	int cg_s = *(int*)(0x915254);
	return cg_s;//cg_s != 0 when loading or in game
}
float angleseeeee[3];
vec3 vectoangles(vec3 Angles)
{
	float forward;
	float yaw, pitch;
	float PI = 3.1415926535897931;
	if (Angles.x == 0 && Angles.y == 0)
	{
		yaw = 0;
		if (Angles.z > 0) pitch = 90.00;
		else pitch = 270.00;
	}
	else
	{
		if (Angles.x != -1) yaw = (float)(atan2((double)Angles.y, (double)Angles.x) * 180.00 / PI);
		else if (Angles.y > 0) yaw = 90.00;
		else yaw = 270;
		if (yaw < 0) yaw += 360.00;

		forward = (float)sqrt((double)(Angles.x * Angles.x + Angles.y * Angles.y));
		pitch = (float)(atan2((double)Angles.z, (double)forward) * 180.00 / PI);
		if (pitch < 0) pitch += 360.00;
	}
	angleseeeee[0] = -pitch;
	angleseeeee[1] = yaw;
	angleseeeee[2] = 0;

	vec3 Angless = { angleseeeee[0], angleseeeee[1], angleseeeee[2] };
	return Angless;
}

/*
struct usercmd_s
{
    int commandTime; //0x0000 
    int buttons; //0x0004 
    vec3 viewangles; //0x0008 
	char _0x0014[20];
};//Size=0x0028
*/
/*
opd_s ucmd = {0x00218D68,TOC};
void(*SV_GetUsercmd)(int clientNum, unsigned int) = (void(*)(int,unsigned int))&ucmd;
usercmd_s *getMyCmd() {
	//SV_GetUsercmd(client,0x10070500);//should store the pointer at 0x10070500
	return (usercmd_s*)0x10070500;
}*/
/*void setViewAngles(vec3 angles) {
	if (getMyCmd() != 0) {
		getMyCmd()->viewangles.x = angles.x;
		getMyCmd()->viewangles.y = angles.y;
		getMyCmd()->viewangles.z = 0;
	} else {
		printf("Problem!");
	}
}
void setViewAngles(float x, float y) {
	vec3 angl = {x,y,0};
	setViewAngles(angl);
}*/
float dx, dy, dz;
float GetDistance(vec3 c1, vec3 c2)
{
	float dx = c2.x - c1.x;
	float dy = c2.y - c1.y;
	float dz = c2.z - c1.z;

	return sqrt((float)((dx * dx) + (dy * dy) + (dz * dz)));
}

int GetHealth()
{
	int cg_s = *(int*)(0x915254);
	return *(char*)(cg_s + 0x153);
}
bool isFFA()
{
	int serverinfo = *(int*)(0x915258);
	char ffa = *(char*)(serverinfo + 0x21);
	if(ffa == 0x6D)
	{
		return true;
	}
	return false;
}
long ClActive()
{
	return *(long*)0x9FD590;
}

//Thanks To Kurt & Milky4444 For Addresses In This Funcution
//Not Working, Call in Hook
opd_s SL_GSOS = {0x1FEE88/*0x200E38*/,TOC};
unsigned int(*SL_GetString)(const char *str,int user, int unk) = (unsigned int(*)(const char*,int,int))&SL_GSOS;
opd_s GCDObJ = {0x1E59A0,TOC};
int (*Com_GetClientDObj)(unsigned int handle, int localClientNum) = (int(*)(unsigned int,int))&GCDObJ;
opd_s DObjGWTP = {/*0x52738 NOT RIGHT ADDRESS*//*0x519A0*/0x052738,TOC};
bool (*CG_DObjGetWorldTagPos)(unsigned int targ_centity, unsigned int dobj, unsigned int tagIndex, float *pos) = (bool(*)(unsigned int,unsigned int,unsigned int,float*))&DObjGWTP;
opd_s gti = {0x1BE740,TOC};
int (*G_TagIndex)(char *tag) = (int(*)(char*))&gti;
bool AimTarget_GetTagPos(int targEnt, unsigned int targIndex,float *outPos)
{
	if (Centity(targEnt) == -1)
		return false;
	//printf("Getting DOBJ...\n");
	unsigned int handle = *(unsigned int*)(Centity(targEnt) + 0xD0);
	//printf("Handle: %i\n",(int)handle);
	unsigned int DObj = Com_GetClientDObj(handle ,0);
	if (DObj == 0)
		return false;
	//printf("Got DOBJ %i. Getting tag pos...\n",DObj);
	return CG_DObjGetWorldTagPos(Centity(targEnt), DObj, targIndex, outPos);//crashes on this function
}
opd_s atgtp = {0x013460,TOC};
void (*AimTarget_GetTagPoss)(unsigned int centity, unsigned int tagName, float *pos) =  (void(*)(unsigned int,unsigned int, float *))&atgtp;
vec3 retttt;
int headTag = 0;
vec3 playerHeadCoords[20];
vec3 GetBonePos(int playerIndex/*, char* tagName*/)
{
	//if (headTag == 0) {
		//headTag = SL_GetString("j_neck",0,0);//G_TagIndex("j_neck");
		//printf("Tag: %i\n",headTag);
	//}
	//if (headTag != 0) {
	if (headTag == 0) {
		headTag = /*0x0BE3;*/(int)(*(short*)(*(int*)0x71E82C+0x1AC));
		printf("Head Tag: %i\n",headTag);
	}
	if (headTag != 0) {
		float out[3];
		if (AimTarget_GetTagPos(playerIndex, headTag/*SL_GetString(tagName)*/, out)) {
			//printf("Got tag pos\n");
			retttt.x = out[0];
			retttt.y = out[1];
			retttt.z = out[2];
		}
		
	} else {
		printf("ERROR: Head tag = 0\n");
	}
	return retttt;
}
/*
//I believe this worked, but it's not that great
opd_s CG_CanSeeFriendlyHead_t = {0x00048600,TOC};
bool(*CG_CanSeeFriendlyHead)(int localClient, unsigned int centity) = (bool(*)(int,unsigned int))&CG_CanSeeFriendlyHead_t;
bool isPlayerVisible(int player) {
	return CG_CanSeeFriendlyHead(0,Centity(player));
}*/
struct trace_t{
    float fraction;
    float normal[3];
    int surfaceFlags;
    int contents;
    const char *material;
    char hitType;
    unsigned short hitId;
    unsigned short modelIndex;
    unsigned short partName;
    unsigned short partGroup;
    bool allsolid;
    bool startsolid;
    bool walkable;
};//size = 10?
opd_s glt = {0x096178/*G0x0188560*/,TOC};
void (*CG_LocationalTrace)(trace_t* trace, float *start, float *end, int passEntityNum, int contentmask/*, char *priorityMap*/) = (void(*)(trace_t*, float*, float*, int, int/*, char**/))&glt;
bool Trace(int clientNum, vec3 start, vec3 end){
    trace_t trace;
    CG_LocationalTrace(&trace, (float*)&start, (float*)&end, clientNum, /*0x2806831*//*0x800811*/0x2803001/*, 0*/);
	//printf("Trace: %i\n",ServerDataStruct.loctrace);
	//if (trace.fraction > 0.9f)
		//printf("Trace: %i percent\n",(int)(trace.fraction*100));
	return (trace.fraction >= 0.95f);
}
int GetStance(int ent)
{
	int stance = (int)(*(char*)(Centity(ent) + 0x63));
	//0x00 = standing
	//0x04 = crouch
	//0x08 = prone
}
float getHeight(int client) {
	int stance = GetStance(client);
	switch(stance) {
	case 0:
		return 56;
	case 4:
		return 21;
	case 8:
		return 15;
	default:
		return 0;
	}
	return 0;
}
float dropHeightArray[20];
float getDrop(int client) {
	int stance = GetStance(client);
	switch(stance) {
	case 0:
		return -10;
	case 4:
		return -25;
	case 8:
		return -40;
	default:
		return -10;
	}
	return -10;
}
bool isSameTeam(int cl1,int cl2)
{
		//Online Address
        //ClientInfo_t = 0x367F4E85
	    //Offline Address
        //ClientInfo_t = 0x36BE4E85
	    int cg_s = *(int*)(0x915254);
		int ent2 = *(char*)(cg_s + 0xF6E80 + ((cl2) * 0x544) + 0x2F);
	    int ent1 = *(char*)(cg_s + 0xF6E80 + ((cl1) * 0x544) + 0x2F);
	    if((ent1) == (ent2))
	    {
	        return true;
	    }
	    return false;
}

vec3 getOrigin(int ent)
{
	int cent = Centity(ent);
	return *(vec3*)(cent + 0x18);
}

namespace Visible {

struct cTrace
{
    float Fraction;
    unsigned char szUnknown0[292];
};
struct tracearg_t {
	int unk1,unk2,unk3,unk4,unk5,unk6,unk7,unk8,unk9,unk10;
	char padding[100];
};
int CG_TraceAddress = 0x00095EC8;
opd_s trace_t = {CG_TraceAddress,TOC};
void (*CG_Trace)(cTrace *results, vec3 start, vec3 mins, vec3 maxs, vec3 end, int passEntityNum, int contentMask, tracearg_t* TraceArg, bool staticModels) = (void(*)(cTrace *, vec3, vec3 , vec3, vec3, int, int, tracearg_t*, bool))&trace_t;



tracearg_t TRACEARG;
 
bool IsVisible( vec3 v3EndPos, float nHeight )
{
	TRACEARG.unk5 = 1023;
	TRACEARG.unk6 = 1023;
    cTrace mTrace;
    v3EndPos.z += nHeight;
    vec3 v3Zero = { 0.0f, 0.0f, 0.0f };
    //CG_Trace( &mTrace, gRefDef->Location, v3EndPos,*(vec3*)0x71CEEC/*Found in AimTarget_IsTargetVisible*/ /**(vec3*)0x71E864*//*Used in CG_IsHeadVisible*/, v3Zero, 0x803003/*gGame->iLocalClient*/, 0x2801001/*0x2803001*/ ,0,0);
    CG_Trace( &mTrace, getOrigin(GetIndex()),v3Zero,v3Zero,v3EndPos,0x803003,0x2801001,&TRACEARG,0);
	return( mTrace.Fraction >= 0.95f );
}
/*
bool IsVisible2( vec3 End )
{
    cTrace t;
    vec3 Null = {0,0,0};
    CG_Trace( &t, getOrigin(GetIndex())/RefDef->Origin/,Null, Null,End,GetIndex()/Cg->ClientNumber/, 0x803003 ,0,0);
 
    return ( t.Fraction >= 0.97f );
}*/

}
bool playerVisibilities[20];
int GetNearestPlayer(int client, bool ignoreVisibleChecks = false)
{
	if (Centity(0) == -1)
		return -1;
	if (client == -1)
		return -1;
    int nearestClient = -1;
    double nearestDistance = 1E+08f;
	for (int i = 0; i < 18; i++)
    {
        if (i != client)
        {
            if (isAlive(i))
            {
				if (!isSameTeam(client, i) || isFFA())
				{
					if (ignoreVisibleChecks || playerVisibilities[i] == 1) {
						float Distance = GetDistance(getOrigin(client), getOrigin(i));
						if (Distance < nearestDistance)
						{
							nearestDistance = Distance;
							nearestClient = i;
						}
					}
				}
			}
        }
    }
	if (!visibleOnly && nearestClient == -1 && !ignoreVisibleChecks)//when visibleOnly is true, this won't happen
		return GetNearestPlayer(client,true);
	//int b = 2;
	//if (nearestClient != -1)
		//b = (int)playerVisibilities[nearestClient];
	//printf("Aiming At Client %i Visible: %i\n",nearestClient,b);
	return nearestClient;
}




vec3 AimAngles = {0,0,0};
bool aimbothit = false;
//bool silentAim = true;


bool checkActive()
{
	return ButtonPressed(host, "+frag");
}
//vec3 playerHeadCoordinates[20];
void Aimthread(uint64_t)
{
	/*
	0x0095afe0 aim
	0x0095b080 shoot
	*/
	for(;;) {
		if (*(char*)0x01D17A8C && ClActive() >= 0x10000) {
			//printf("\n\nStart\n");
			if (Aimbot && UnfairAimbot) {
				int targindex = GetNearestPlayer(GetIndex(),true);
				if (targindex != -1 && *(char*)0x0095b080 == 1) {
					vec3 Aim = {playerHeadCoords[targindex].x,playerHeadCoords[targindex].y,playerHeadCoords[targindex].z};//=getOrigin(targindex);
					/*Aim.x = playerHeadCoords[targindex].x;
					Aim.y = playerHeadCoords[targindex].y;
					Aim.z = playerHeadCoords[targindex].z;*/
					vec3 me=/**(vec3*)0x36af1198;*/getOrigin(GetIndex());
					/*me.x = playerHeadCoords[GetIndex()].x;
					me.y = playerHeadCoords[GetIndex()].y;
					me.z = playerHeadCoords[GetIndex()].z;*/
					//Aim.z+=getDrop(targindex);
					//printf("Drop Height: %i\n",(int)dropHeightArray[targindex]);
					vec3 angles = {Aim.x - me.x,Aim.y - me.y,Aim.z - me.z/* - 12*//*+dropHeightArray[targindex]*/};
					/*
					#if CEX == 0
						float x = *(float*)read_process(ClActive() + BaseAngle,4);
						float y = *(float*)read_process(ClActive() + BaseAngle + 0x4,4);
						float z = *(float*)read_process(ClActive() + BaseAngle + 0x8,4);
					#else
						float x = *(float*)(ClActive() + BaseAngle);
						float y = *(float*)(ClActive() + BaseAngle + 0x4);
						float z = *(float*)(ClActive() + BaseAngle + 0x8);
					#endif
					*/
					//printf("getting base angles");
					float x = 0; 
					float y = 0;
					if (ClActive() >= 0x10000) {
						x = *(float*)(ClActive() + BaseAngle);
						y = *(float*)(ClActive() + BaseAngle + 0x4);
						/*int a = (int)read_process(ClActive() + BaseAngle,4);
						if (a >= 0x10000)
							x = *(float*)a;
						a = (int)read_process(ClActive() + BaseAngle + 0x4,4);
						if (a >= 0x10000)
							y = *(float*)a;*/
					}
					//printf("Got base angles\n");
					//float z = *(float*)(ClActive() + BaseAngle + 0x8);
					vec3 base = {x,y,0};
					vec3 cangle = vectoangles(angles);
					vec3 fin = {cangle.x - base.x,cangle.y - base.y,0};
					AimAngles.x = fin.x;
					AimAngles.y = fin.y;
					AimAngles.z = 0;
					aimbothit = true;




					//G_Damage((int*)G_Entity(targindex),(int*)G_Entity(host),(int*)G_Entity(host),

					//Player_Die(int* self, int* inflictor, int* attacker, int damage, int meansOfDeath, int WeaponIndex, const float *vDir, int hitLocation, int psTime)
					//Player_Die((int*)G_Entity(targindex),(int*)G_Entity(host),(int*)G_Entity(host), 0x3C, 2, getCurrentWeapon(host), (float*)0xD014ABF0, 0x1BF6000, 0x10009220);

					//G_DamageClient(int *target, int *inflictor, int *attacker, const float *dir, const float *point, int damage, int dflags, int mod, int weapon, int hitLoc, int timeOffset)
					//G_DamageClient(&victim, &killer, &killer, (float*)0xD014ABF0, (float*)0xD014ACB4, 0x3C, 2, 2, 0x10A, 0x1BF6000, 0x10009220);
					G_DamageClient((int*)G_Entity(targindex),(int*)G_Entity(host),(int*)G_Entity(host), (float*)0xD014ABF0, (float*)0xD014ACB4, 0x3C, 2, 2, getCurrentWeapon(host), 0x1BF6000, 0x10009220);
					//killClient(targindex);
					//Kill(targindex);
					sleep(100);
				}
			}
			else if(Aimbot && (!Required || isAiming()))
			{
				//printf("Getting nearest player\n");
                int targindex = GetNearestPlayer(GetIndex());
				if (targindex != -1) {
					//printf("retrieving head coords\n");
					vec3 Aim = {playerHeadCoords[targindex].x,playerHeadCoords[targindex].y,playerHeadCoords[targindex].z};//=getOrigin(targindex);
					/*Aim.x = playerHeadCoords[targindex].x;
					Aim.y = playerHeadCoords[targindex].y;
					Aim.z = playerHeadCoords[targindex].z;*/
					vec3 me=/**(vec3*)0x36af1198;*/getOrigin(GetIndex());
					/*me.x = playerHeadCoords[GetIndex()].x;
					me.y = playerHeadCoords[GetIndex()].y;
					me.z = playerHeadCoords[GetIndex()].z;*/
					//Aim.z+=getDrop(targindex);
					//printf("Drop Height: %i\n",(int)dropHeightArray[targindex]);
					vec3 angles = {Aim.x - me.x,Aim.y - me.y,Aim.z - me.z/* - 12*//*+dropHeightArray[targindex]*/};
					/*
					#if CEX == 0
						float x = *(float*)read_process(ClActive() + BaseAngle,4);
						float y = *(float*)read_process(ClActive() + BaseAngle + 0x4,4);
						float z = *(float*)read_process(ClActive() + BaseAngle + 0x8,4);
					#else
						float x = *(float*)(ClActive() + BaseAngle);
						float y = *(float*)(ClActive() + BaseAngle + 0x4);
						float z = *(float*)(ClActive() + BaseAngle + 0x8);
					#endif
					*/
					//printf("getting base angles");
					float x = 0; 
					float y = 0;
					if (ClActive() >= 0x10000) {
						x = *(float*)(ClActive() + BaseAngle);
						y = *(float*)(ClActive() + BaseAngle + 0x4);
						/*int a = (int)read_process(ClActive() + BaseAngle,4);
						if (a >= 0x10000)
							x = *(float*)a;
						a = (int)read_process(ClActive() + BaseAngle + 0x4,4);
						if (a >= 0x10000)
							y = *(float*)a;*/
					}
					//printf("Got base angles\n");
					//float z = *(float*)(ClActive() + BaseAngle + 0x8);
					vec3 base = {x,y,0};
					vec3 cangle = vectoangles(angles);
					vec3 fin = {cangle.x - base.x,cangle.y - base.y,0};
					AimAngles.x = fin.x;
					AimAngles.y = fin.y;
					AimAngles.z = 0;
					aimbothit = true;
					if (!silentAimbot) {
						*(float*)(ClActive() + Angleoffset) = fin.x;
						*(float*)(ClActive() + Angleoffset + 0x4) = fin.y;
						*(float*)(ClActive() + Angleoffset + 0x8) = fin.z;
					}
					if (autoShoot/* && !silentAimbot*/) {
						if (playerVisibilities[targindex]) {
							if (!isCurrentlyShooting) {
								if(autoAim)
									*(char*)0x0095afe0 = 1;//aim
								*(char*)0x0095b080 = 1;
								if (semiAutoMode) {
									sleep(2);
									*(char*)0x0095b080 = 0;
								} else {
									isCurrentlyShooting = true;
								}
							}
						} else {
							if (isCurrentlyShooting || (autoAim && *(char*)0x0095afe0 == 1)) {
								isCurrentlyShooting = false;
								*(char*)0x0095b080 = 0;
								if(autoAim)
									*(char*)0x0095afe0 = 0;//unaim
							}
						}
					}
				} else {
					aimbothit = false;
					if (autoShoot/* && !silentAimbot*/) {
						//no current target so stop shooting
						if (isCurrentlyShooting) {//I am shooting
							isCurrentlyShooting = false;
							*(char*)0x0095b080 = 0;
							if(autoAim)
								*(char*)0x0095afe0 = 0;//unaim
						}
					}
				}
			}
			sleep(1);
		} else {
			sleep(200);
		}
	}
}

/*
void NoSpread()
{
    // get playerState_s struct
    int playerStatePtr = (int)cgArray + 263836;
 
    // get weaponDef struct and our starting spread float value
    float spread;
    int weaponDefPtr = BG_GetWeaponDef(GetCurrentWeapon());
    spread = *(float*)((int)cgArray + 457488) / 255.0f;
 
    // get final spread float value
    float minSpread, maxSpread;
    BG_GetSpreadForWeapon(playerStatePtr, weaponDefPtr, &minSpread, &maxSpread);
    if (*(float*)(playerStatePtr + 360) == 1.0f) // if (fWeaponPosFrac == 1.0f) get minSpread from weaponDef
        spread = *(float*)(weaponDefPtr + 1736) + ((maxSpread - *(float*)(weaponDefPtr + 1736)) * spread);
    else
        spread = minSpread + ((maxSpread - minSpread) * spread);
 
    // get our view origin
    vec3 viewOrigin;
    if (!CG_GetPlayerViewOrigin(0, playerStatePtr, viewOrigin))
        return;
 
    // get our view axis
    vec3 viewAngles;
    vec3 viewAxis[3];
        viewAngles[0] = *(float*)((int)cgArray + 389012);
        viewAngles[1] = *(float*)((int)cgArray + 389016);
        viewAngles[2] = 0;
        AngleVectors(viewAngles, viewAxis[0], viewAxis[1], viewAxis[2]);
 
    // get the range
    float range;
    //if (*(int*)(weaponDefPtr + 32) == 3) // if (weapClass == WEAPCLASS_SPREAD)
    //    range = *(float*)(weaponDefPtr + 1964); // range = fMinDamageRange; (probably for shotguns)
    //else
        range = 8192.0f; // from bulletrange dvar
 
    // get the end vector in the game world of the spread and the direction vector too
    // we use the direction vector directly instead of the end vector
    vec3 spreadEndVec;
    vec3 spreadDirection;
    GetSpreadEndVec(viewOrigin, spreadEndVec, spread, *(int*)playerStatePtr, spreadDirection, viewAxis[0], viewAxis[1], viewAxis[2], range);
    
    // convert the spread direction to angles
    vectoangles(spreadDirection, viewAngles);
    
    // fix our angles
    usercmd_s* oldcmd = CL_GetUserCmd(CL_GetCurrentCmdNumber() - 1);
 
    // get the angle delta and write it to the user cmd
    // do -= because we want to get the inverse of the spread
    oldcmd->angles[0] -= ANGLE2SHORT(-(*(float*)((int)cgArray + 389012) - viewAngles[0]));
    oldcmd->angles[1] -= ANGLE2SHORT(-(*(float*)((int)cgArray + 389016) - viewAngles[1]));
 
    oldcmd->serverTime += 1;
}*/



opd_s CL_WritePacket_t = {0x000A1A88,TOC};
int(*CL_WritePacket)(int localClient) = (int(*)(int))&CL_WritePacket_t;
opd_s CL_CreateNewCommands_t = {0x000A3BB0,TOC};
int(*CL_CreateNewCommands)(int localClient) = (int(*)(int))&CL_CreateNewCommands_t;

//cmdnum*0x2c+0x66AA0+*(int*)0x00AE9780
int getCurrentCmd(int add) {
	//0x3462BD58 works too
	//128*0x28+0x3364+*(int*)0x9FD590
	return ((*(int*)(128*0x28+0x3364+ClActive()))+add) & 0x7f;
	//return *(int*)(0x6A4A4+*(int*)0x00AE9780);
}
int getCmdAddr(int cmdnum) {
	return cmdnum*0x28+0x3364+ClActive();//*(int*)0x9FD590
	//cmdnum*0x28+0x3364+*(int*)0x9FD590
}
/*int getCCmdAddr(int cmdnum) {
	return cmdnum*0x24+0x680A4+*(int*)0x00AE9780;
}*/
#define ANGLE2SHORT(x)   ((int)((x)*65536/360) & 65535)
struct vec3i {
	int x,y,z;
};
float normalize180(float num) {
	while (num < -180)
		num+=360;
	while (num > 180)
		num-=360;
	return num;
}
float normalize90(float num) {
	while (num < -90)
		num+=180;
	while (num > 90)
		num-=180;
	if (num > 89)
		num = 88;
	if (num < -89)
		num = -88;
	return num;
}

/*void calcViewAngles() {
	int targindex = GetNearestPlayer(GetIndex());
	if (targindex != -1) {
		vec3 Aim = {positions[targindex].x,positions[targindex].y,positions[targindex].z};
		vec3 *me=getViewOrigin();
		vec3 angles = {Aim.x - me->x,Aim.y - me->y,Aim.z - me->z};
		getBaseAngles();
		vec3 cangle = vectoangles(angles);
		vec3 fin = {cangle.x - baseAngleRet.x,cangle.y - baseAngleRet.y,0};
		AimAngles.x = fin.x;
		AimAngles.y = fin.y;
		AimAngles.z = 0;
		aimbothit = true;
	} else {
		aimbothit = false;
	}
}*/

int hkCL(int r3);
int wp_redo(int r3) {
	char bytes[16] = {0x7D, 0x80, 0x00, 0x26, 0x1D, 0x63, 0x0C, 0x90, 0x39, 0x6B, 0x0B, 0x40, 0x81, 0x22, 0xCA, 0x4C};
	write_process1((void*)0x000A1A88,bytes,16);//crashes here
	int j = CL_WritePacket(r3);
	PatchInJump(0x000A1A88,(int)hkCL,false);
	return j;
}
int clcnc_redo(int r3) {
	char bytes[16] = {0xF8, 0x21, 0xFE, 0x31, 0x7C, 0x08, 0x02, 0xA6, 0xFB, 0xC1, 0x01, 0x78, 0xFB, 0x21, 0x01, 0x50};
	write_process1((void*)0x000A3BB0,bytes,16);
	int j = CL_CreateNewCommands(r3);
	PatchInJump(0x000A3BB0,(int)hkCL,false);
	return j;
}




int oviewangles[3];
//int cviewangles[3];

//vec3i viewangles[128];

void DoSilentAim(bool bAdjust)
{
    int cmd = getCmdAddr(getCurrentCmd(0));
	int oldcmd = getCmdAddr(getCurrentCmd(-1));

	vec3i *oldcmdangles = (vec3i*)(oldcmd+0x8);
	vec3i *cmdangles = (vec3i*)(cmd+0x8);

    if (bAdjust) // Do adjustment
    {

        /* Backup servertime */
        int stime = *(int*)oldcmd;

        /* Copy current command data to old command */
        memcpy((void*)oldcmd, (void*)cmd, 0x28);

        /* Restore old servertime but add 1 to activate l33t h4x */
		*(int*)oldcmd = stime+1;

        /* Backup old viewangles */
        memcpy(oviewangles,oldcmdangles,4*3);


		int x = ANGLE2SHORT(/*normalize90(*/AimAngles.x/*)*/);
		int y = ANGLE2SHORT(/*normalize180(*/AimAngles.y/*)*/);

        /* Not ideal (see below) */
        oldcmdangles->x = x;
        oldcmdangles->y = y;

		//cmdangles->x = /*cmdangles->x + */x;
        //cmdangles->y = /*cmdangles->y + */y;

		//*(int*)(oldcmd+0x4) &= ~(1<<0);//unshoot old cmd
		//if(autoShoot)
			//*(int*)(cmd+0x4) |= (1<<0);//shoot    oldcmd does not work
		//*(int*)(cmd+0x4) |= (1<<11);//aim
		//printf("Set Silent Aim Angles\n");
		/*for (int i = 0; i < 128; i++) {
			int cmd = getCmdAddr(getCurrentCmd(i));
			vec3i *angles = (vec3i*)(cmd+0x8);
			memcpy(angles,&(viewangles[i]),4*3);
			angles->x = x;
			angles->y = y;
		}*/
    }
    else // Fix angles
    {
        /* Restore old viewangles */
        memcpy(oldcmdangles,oviewangles, 4*3);
		//memcpy(cviewangles, cmdangles, 4*3);
		//*(int*)(cmd+0x4) &= ~(1<<0);//unshoot
		//*(int*)(cmd+0x4) &= ~(1<<11);//unaim
		/*for (int i = 0; i < 128; i++) {
			int cmd = getCmdAddr(getCurrentCmd(i));
			vec3i *angles = (vec3i*)(cmd+0x8);
			memcpy(&(viewangles[i]),angles,4*3);
		}*/
    }
} 
/*
bool AntiAimCmd(usercmd_t* c)
{
    c->buttons &= ~(1<<28);
    c->buttons &= ~(1<<11);
    return true;
}*/

int hkCL(int r3)
{
	int ret;
	//printf("%i\n",aimbothit);
	//int ret = clcnc_redo(r3);
	//aimbothit = true;
	if (aimbothit && silentAimbot && Aimbot) {

		DoSilentAim(1); // Adjust angles

		ret = wp_redo(r3);

		DoSilentAim(0); // Fix angles
	} else {
		ret = wp_redo(r3);
	}
	//return wp_redo(r3);
	return ret;
} 

/*
bool tmpautoshoot = false;
void Aimthread(uint64_t)
{
	for(;;) {
		if(AimbotEnabled && isInGameOrLoading()) {
			if (amIAiming() || !RequireAiming) {
				calcViewAngles();
				if (aimbothit && !silentAim) {
					setViewAngles(AimAngles.x,AimAngles.y,AimAngles.z);
					if(autoShoot) {
						tmpautoshoot = true;
						*(char*)0x00AE1820 = 1;
					}
				} else {
					if(autoShoot && tmpautoshoot) {
						tmpautoshoot = false;
						*(char*)0x00AE1820 = 0;
					}
				}
			} else {
				if(autoShoot && tmpautoshoot) {
					tmpautoshoot = false;
					*(char*)0x00AE1820 = 0;
				}
			}
		}
		sleep(1);
	}
}*/


void hook() {
	//vec3 myOriginn = getOrigin(GetIndex());
	//vec3 myOrigin = {myOriginn.x,myOriginn.y,myOriginn.z+getHeight(GetIndex())};
	//printf("My Z: %s\n",FtoS(myOrigin.z));
	if (*(bool*)0x01D17A8C) {
		//SV_GetUsercmd(GetIndex(),0x10070500);
		GetBonePos(GetIndex());
		vec3 me = {retttt.x,retttt.y,retttt.z};
		playerHeadCoords[GetIndex()].x = me.x;
		playerHeadCoords[GetIndex()].y = me.y;
		playerHeadCoords[GetIndex()].z = me.z-56;
		//if (Centity(0) != -1)
		 for (int i = 0; i < 18; i++) {
			 //if (killclients[i]) {
			 //	 killClientReal(i);
			 //	 killclients[i] = false;
			 //}
			 if(GetIndex() != i) {
				 if (isAlive(i)) {
					//dropHeightArray[i] = getDrop(i);
					//vec3 theirOriginn = getOrigin(i);
					//vec3 theirOrigin = {theirOriginn.x,theirOriginn.y,theirOriginn.z+getHeight(i)};
					GetBonePos(i);
					vec3 them = {retttt.x,retttt.y,retttt.z};
					playerHeadCoords[i].x = them.x;
					playerHeadCoords[i].y = them.y;
					playerHeadCoords[i].z = them.z-56;
					//isPlayerVisible(i);
					playerVisibilities[i] = Trace(0,me/*myOrigin*//*getOrigin(GetIndex())*/,them/*theirOrigin*//*getOrigin(i)*/);/*isPlayerVisible(i);*//*Visible::IsVisible(getOrigin(i),56);//56*/
					//vec3 me = getOrigin(GetIndex());
					//vec3 them = getOrigin(i);
					//startA[0] = me.x;
					//startA[1] = me.y;
					//startA[2] = me.z;
					//endA[0] = them.x;
					//endA[1] = them.y;
					//endA[2] = them.z;
					//float cv = FX_GetClientVisibility(0,startA,endA)*100;
					//int g = (int)cv;
					//printf("Client %i: %i percent visible\n",i,g);
				 }
			 }
		 }
		 //Aimthread(0);
	}
	//printf("\n");
}










//======================================//
//                                      //
//                                      //
//=============MAIN=THREAD==============//
//                                      //
//                                      //
//======================================//



void onPlayerSpawned(int clientIndex)
{
	sleep(1500);
	char buf[100];
	_toc::stdc_snprintf(buf, 100, "Welcome [ %s ]", (char*)(0x14E5408 + (clientIndex * 0x3700)));
	huds::SetMessage(clientIndex, buf, "PlautGaming's Black Ice v2 SPRX!", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
	_toc::SV_GameSendServerCommand(clientIndex, 0, "o \"mp_level_up\"");

	char dest[100];
	_toc::stdc_snprintf(dest, 100, "^5PlautGaming's Black Ice v1 SPRX! Press [{+actionslot 1}] For Menu");
	_toc::iPrintln(clientIndex, dest);
}

void Menu_Thread()
{
	sys_ppu_thread_t id2;
	sys_ppu_thread_create(&id2,Aimthread,0,0x5AA,0x8000,0,"Aimbot");
	for(;;)
	{
		for(int i = 0; i < 18; i++)
		{
			if(_toc::Dvar_GetBool("cl_ingame") == 1)
			{
				if(!ModLoaded[i])
				{
					*(int*)0x131689C = 1;//to use precached shaders
					host = getHostClientNumber();
					if(i == host)
					{
						if(StatusForever == false)
						{
							char buf[100];
							_toc::stdc_snprintf(buf, 100, "Host: [ %s ]", (char*)(0x14E5408 + (i * 0x3700)));
							huds::SetMessage(i, buf, "Host Set Successfully", 2048, 50, 7000, 1500, 255, 255, 255, 255, 17, 255, 255, 255);
							*(char*)0x007EDCA4 = 0x0;//non host end game fix
							
							AccessLevel[i] = Host;
							StatusForever = true;
						}
						//aimbot stuff
					}
					
					if(isAllowed(i))
					{
						_toc::stdc_snprintf(mapBuf, 100, "Q %s", Dvar_GetString("ui_mapname"));//get map name assigng to mapbuf
						_toc::SV_GameSendServerCommand(i, 1, "v loc_warnings \"0\"");
						_toc::SV_GameSendServerCommand(i, 1, "v loc_warningsAsErrors \"0\"");
						onPlayerSpawned(i);
						freezeControls(i, false);
						MenuHuds(i);
						Close(i);
						ModLoaded[i] = true;
					}
				}
				else
				{
					if(!MenuOpened[i])
					{
						if(ButtonPressed(i, "+actionslot 1"))
						{
							sleep(200);
							Open(i);
							loadMenu(i,"main","Exit");
							MenuOpened[i] = true;
						}
					}
					else
					{
						if(ButtonPressed(i, "+actionslot 1"))
						{
							sleep(120);
							Scroll[i] --;
							if (Scroll[i] < 1)
							{
									Scroll[i] = MaxScroll[i];
							}
							Scrolling(i);
						}
						else if(ButtonPressed(i, "+actionslot 2"))
						{
							sleep(120);
							Scroll[i] ++;
							if (Scroll[i] > MaxScroll[i])
							{
									Scroll[i] = 1;
							}
							Scrolling(i);
						}
						else if(ButtonPressed(i, "+gostand"))
						{
							sleep(150);
							getMenuFuncList(i);
							Scrolling(i);
						}
						else if(ButtonPressed(i, "+usereload"))
						{
							sleep(150);
							if(ParentMenu[i]=="Exit")
							{
								Close(i);
								MenuOpened[i] = false;
							}
							else if(CurrentMenu[i]=="Players Menu")
							{
								loadMenu(i, "main", "Exit");
							}
							else if(CurrentMenu[i]=="client")
							{
								loadMenu(i, "Players Menu", "main");
							}
							else
							{
								loadMenu(i,ParentMenu[i],"Exit");
							}
						}
					}
				}
				if(!isAlive(i) && isAllowed(i))
				{
					sys_timer_sleep(1);
					onPlayerSpawned(i);
				}
			}
			else
			{
				StatusForever = false;
				huds::Destroy;
				ModLoaded[i] = false;
				removeAccess(i, None);
			}
		}
	}
}