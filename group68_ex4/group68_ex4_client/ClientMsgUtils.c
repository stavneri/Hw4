#include "ClientMsgUtils.h"
#include "ClientHeader.h" 
#include "SocketClient.h"


/*Initializes the msg to be filled later*/
void MsgInit(Msg_t *msg)
{
	msg->NumOfParams = 0;
	msg->MsgType = ERROR_TYPE;
	for (int i = 0; i < MAX_PARAMS; i++)
	{
		strcpy(msg->MsgParams[i], "");
	}
}

/*Gets msg to append and arguments and fills the msg*/
void FillMsg(Msg_t *msg, int type, char **params, int NumOfParams)
{
	MsgInit(msg);
	msg->MsgType = type;
	if (params != NULL) {
		for (int i = 0; i < NumOfParams; i++)
		{
			strcpy(msg->MsgParams[i], params[i]);
		}
	}
}

/*Gets a msg and str and makes the appropriate msg type. return ERROR_RETURN for fail. 1 for success*/
int ServerMsgDecode(char *MsgStr, Msg_t *msg)
{
	int RetVal = 1;
	int type, i, j, flag = 0;
	char MsgType[MAX_TYPE_LEN];
	char **params;

	params = (char**)malloc((MAX_PARAMS + MAX_PARAM_LEN) * sizeof(char));
	if (NULL == params)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	for (j = 0; j < MAX_PARAMS; j++)
	{
		params[j] = (char*)malloc((MAX_PARAM_LEN) * sizeof(char));
		if (NULL == params[j])
		{
			printf("Error in malloc, closing thread.\n");
			return ERROR_RETURN;
		}
	}

	for (i = 0; MsgStr[i] != ':'; i++)
	{
		MsgType[i] = MsgStr[i];
		if (MsgStr[i + 1]== '\n')
		{
			flag = 1;
			break;
		}
	}
	MsgType[i + flag] = '\0';


	if (STRINGS_ARE_EQUAL("SERVER_MAIN_MENU", MsgType))
	{
		type = SERVER_MAIN_MENU;
		RetVal = 0;
	}
	else if (STRINGS_ARE_EQUAL("SERVER_APPROVED", MsgType))
	{
		type = SERVER_APPROVED;
		RetVal = 0;
	}
	else if (STRINGS_ARE_EQUAL("SERVER_DENIED", MsgType))
	{
		type = SERVER_DENIED;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("SERVER_INVITE", MsgType))
	{
		type = SERVER_INVITE;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("SERVER_PLAYER_MOVE_REQUEST", MsgType))
	{
		type = SERVER_PLAYER_MOVE_REQUEST;
		RetVal = 0;
	}
	else if (STRINGS_ARE_EQUAL("SERVER_GAME_RESULTS", MsgType))
	{
		type = SERVER_GAME_RESULTS;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("SERVER_GAME_OVER_MENU", MsgType))
	{
		type = SERVER_GAME_OVER_MENU;
		RetVal = 0;
	}
	else if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", MsgType))
	{
		type = SERVER_OPPONENT_QUIT;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("SERVER_NO_OPPONENTS", MsgType))
	{
		type = SERVER_NO_OPPONENTS;
		RetVal = 0;
	}
	else if (STRINGS_ARE_EQUAL("SERVER_LEADERBOARD", MsgType))
	{
		type = SERVER_OPPONENT_QUIT;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("SERVER_LEADERBOARD_MENU", MsgType))
	{
		type = SERVER_NO_OPPONENTS;
		RetVal = 0;
	}
	else
	{
		return ERROR_TYPE;
	}
	if (RetVal != ERROR_RETURN)
	{
		FillMsg(msg, type, params, RetVal);
		free(params);
		return 1;
	}
	else
	{
		free(params);
		return RetVal;
	}
}

/*Gets msg string and an array to be appended. return ERROR_RETURN if fails and 'number of params' for success*/
int GetParams(char **params, char *MsgStr)
{
	char TempStr[MAX_MSG_SIZE], *token;
	int i, j = 0;
	for (i = 0; MsgStr[i] != ':'; i++); //advance i to the beginig of params in string
	i++;
	while (MsgStr[i] != '\n')
	{
		TempStr[j] = MsgStr[i];
		i++;
		j++;
	}
	TempStr[j] = '\0';
	token = strtok(TempStr, ";");
	strcpy(params[0], token);
	i = 1;
	while (token != NULL)
	{
		token = strtok(NULL, ";");
		if (token != NULL)
		{
			strcpy(params[i], token);
			i++;
		}
	}
	return i;
}