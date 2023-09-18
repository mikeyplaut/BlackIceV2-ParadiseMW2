//======================================//
//                                      //
//                                      //
//=============VERIFICATION=============//
//                                      //
//                                      //
//======================================//

typedef enum Verification
{
None,
Verified,
Vip,
Admin,
Host
};
Verification AccessLevel[18];

Verification getAccessLevel(int clientIndex)
{
	return AccessLevel[clientIndex];
}
bool isAllowed(int clientIndex, Verification MinLevel = Verified)
{
	if(AccessLevel[clientIndex] >= MinLevel)
		return true;

	return false;
}
const char* getAccessLevelStr(int clientIndex)
{
	if (isAllowed(clientIndex, Host)) return "^1Host^7";
	if (isAllowed(clientIndex, Admin)) return "^5Admin^7";
	if (isAllowed(clientIndex, Vip)) return "^3Vip^7";
	if (isAllowed(clientIndex, Verified)) return "^2Ver^7";
	else return "";
}