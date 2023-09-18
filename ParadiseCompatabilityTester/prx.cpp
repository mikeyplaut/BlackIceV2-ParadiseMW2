/*
Property of gopro_2027 and http://paradisesprx.com thanks!
This source is free for anyone to use and put in their own menus.
Please give some credit, because it is nice, not because I am forcing you to.

Menu by gopro_2027 with a bunch of code from places about the internet.
This was my first non-host that improved upon the aimbot that was previously available to the public in free menus (snow engine in this case).
Credits:
Nick b haxing & Milky for the og stuff
gopro_2027 for some more aimbot stuff
unknowncheats.com and it's members for having pc mods to convert to ps3
*/


#include "stdafx.h"

#include <cellstatus.h>
#include <sys/prx.h>

#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <cell/cell_fs.h>
#include <sys/stat.h>
#include <cellstatus.h>
#include <sys/prx.h>
#include <sys/ppu_thread.h>
#include <string.h>
#include <sys/memory.h>
#include <sys/timer.h>
#include <sys/process.h>
#include <ppu_intrinsics.h>
#include <stdarg.h>
#include <stdio.h>
#include <sysutil\sysutil_msgdialog.h>
#include <sysutil\sysutil_oskdialog.h>
#include <sysutil\sysutil_oskdialog_ext.h>
#include <cstdlib>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <cell/cell_fs.h>
#include <sys/stat.h>
/*
typedef struct {
    int            x, y, Width, Height;
    float        fov_x, fov_y;
    float       Total_fov;

    vec3        vieworg;

    char        z_crap[4];

    vec3        viewaxis[3];
    //0x60 is some sort of position
    char        z_crap_2[17044];

    vec3        refdefViewAngles;
} refdef_t;//size:0x50(80)
refdef_t *refdef = (refdef_t*)0x337E6020;//0x4358 is origin and 0x434C is angles?*/

//refdef = 0x535E0+*(int*)0x00A95CD8 = 0x337E6020

float StoF(const char* s){
  float rez = 0, fact = 1;
  if (*s == '-'){
    s++;
    fact = -1;
  }
  for (int point_seen = 0; *s; s++){
    if (*s == '.'){
      point_seen = 1;
      continue;
    }
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) fact /= 10.0f;
      rez = rez * 10.0f + (float)d;
    }
  }
  return rez * fact;
}
char* FtoS(float input)
{
	char returnvalue[64];
	int wholenumber = (int)input;
	input -= wholenumber;
	while (input-(int)input > 0) {
		input*=10;
	}
	sprintf(returnvalue, "%i.%i", wholenumber, (int)input);
	return returnvalue;
}


void * _sys_malloc(size_t size);

void _sys_free(void *ptr);


void readFile(char *file, char buf[], int size) {
	int fd;
	int ret;
	uint64_t pos;
	uint64_t nread;
	ret = cellFsOpen(file, 0, &fd, NULL, 0);
	if (!ret)
	{
		cellFsLseek(fd, 0, CELL_FS_SEEK_SET, &pos);
		ret = cellFsRead(fd, buf, size, &nread);
		if (!ret)
		{
			cellFsClose(fd);
		}
	}
}

void writeToFile(char *file, char buf[] = NULL, int size = 0) {
	int err;
 int fd;
 uint64_t nrw;
 int ret;
 /* Open file */
 err = cellFsOpen(file,CELL_FS_O_RDWR|CELL_FS_O_CREAT, &fd, NULL, 0);
 /* Clear file */
 cellFsTruncate(file,0);
 /* Write to file */
 if (size != 0)
 err = cellFsWrite(fd, (const void *)buf, (uint64_t)size, &nrw);
 /* Close file */
 err = cellFsClose(fd);
}
void createFile(char *loc) {
	int fd;
	int ret;
	ret = cellFsOpen(loc,CELL_FS_O_CREAT|CELL_FS_O_RDWR|CELL_FS_O_TRUNC, &fd, NULL, 0);
    cellFsClose(fd);
	if (!ret)
	{
		cellFsClose(fd);
	}
}
int doesFolderExist(const char *Path)
{
	int i, err;	CellFsStat status;
	for (i = 0; i < 15; i++) 
	{
		err = cellFsStat(Path, &status);
		if (err == CELL_FS_SUCCEEDED) 
		{
			return 1;
		}
	}
	return 0;
}
void my_dialog2(int button, void *userdata)
{
    switch(button) {
        case CELL_MSGDIALOG_BUTTON_OK:

        case CELL_MSGDIALOG_BUTTON_NONE:

		case CELL_MSGDIALOG_BUTTON_ESCAPE:
            //dialog_action = 1;
            break;
        default:
		    break;
    }
}





bool isNumericChar(char x) {
	return (x >= '0' && x <= '9') ? true : false;
}
int StoI2( const char *c ) {
	int value = 0;
    int sign = 1;
    if( *c == '+' || *c == '-' ) {
       if( *c == '-' ) sign = -1;
       c++;
    }
    while ( isNumericChar( *c ) ) {
        value *= 10;
        value += (int) (*c-'0');
        c++;
    }
    return value * sign;
}




void * neww (std::size_t sz){
	void *p;

	if (sz == 0)
	sz = 1;
	
	p = _sys_malloc(sz);
	return p;
}


	void deletee (void *ptr) {
		_sys_free(ptr);
	}
void move(char buf[], int start, int end) {
	int placeholder = 0;
	for (int i = start; i < end; i++) {
		buf[placeholder] = buf[i];
		placeholder++;
	}
}


void reverse(char arr[], int count)
{
   char temp;
   for (int i = 0; i < count/2; ++i)
   {
      temp = arr[i];
      arr[i] = arr[count-i-1];
      arr[count-i-1] = temp;
   }
}

// Implementation of itoa()
char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}
char *ItoS(int num)
{
	char buf[30];
	return itoa(num,buf,10);
}




int StoI( const char *c ) {
    int value = 0;
    int sign = 1;
    if( *c == '+' || *c == '-' ) {
       if( *c == '-' ) sign = -1;
       c++;
    }
    while ( isNumericChar( *c ) ) {
        value *= 10;
        value += (int) (*c-'0');
        c++;
    }
    return value * sign;
}
char *getSpotInSpaceArray(char *str, int spot = 0) {
	char *bf = str;
	while (spot > 0) {
		bf = strstr(bf," ")+1;
		spot--;
	}
	char *space = strstr(bf," ");
	char buff[30];
	for (int i = 0; i < 30; i++)
		buff[i] = 0;
	strncpy(buff,bf,space-bf);
	return buff;
}
int getSpotInSpaceArrayAsInt(char *str, int spot = 0) {
	int i = StoI(getSpotInSpaceArray(str,spot));
	return i;
}

#include "Main.h"

void init() {
	PatchInJump(0x38EDE8,(int)hook,false);
	PatchInJump(0x000A1A88,(int)hkCL,false);//silent aim hook write packet
	//PatchInJump(0x000A3BB0,(int)hkCL,false);//silent aim hook client command
	Menu_Thread();//stops the whole process
}

void start() {
		cellMsgDialogOpen(1,"Welcome to Black Ice V2!\nBy PlautGaming",my_dialog2,(void*) 0x0000aaab, NULL);
		init();
}
void Thread(uint64_t nothing) {
	sleep(10000);
	start();
}


SYS_MODULE_INFO("Black Ice SPRX", 0, 1, 1);
SYS_MODULE_START( _ParadiseCompatabilityTester_prx_entry );

SYS_LIB_DECLARE_WITH_STUB( LIBNAME, SYS_LIB_AUTO_EXPORT, STUBNAME );
SYS_LIB_EXPORT( _ParadiseCompatabilityTester_export_function, LIBNAME );

// An exported function is needed to generate the project's PRX stub export library
extern "C" int _ParadiseCompatabilityTester_export_function(void)
{
    return CELL_OK;
}

extern "C" int _ParadiseCompatabilityTester_prx_entry(void)
{
	create_thread(Thread, 0x4AA, 0x7000, "Loading: BlackIceV2", ThreadModuleID);
	create_thread(Calls_thread, 0x4AB, 0x7000, "Loading: Call Thread", thrd_Calls);

    return SYS_PRX_RESIDENT;
}
