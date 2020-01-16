#include "ServerMsgUtils.h"
#include "ServerHeader.h" 
#include "SocketServer.h"


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
void FillMsg(Msg_t *msg, int type, char **params,int NumOfParams)
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
int ClientMsgDecode(char *MsgStr, Msg_t *msg)
{
	int RetVal = 1;
	int type, i, j;
	char MsgType[MAX_TYPE_LEN];
	char **params;

	params = (char*)malloc((MAX_PARAMS+MAX_PARAM_LEN) * sizeof(char));
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
	}
	MsgType[i] = '\0';


	if (STRINGS_ARE_EQUAL("CLIENT_REQUEST", MsgType))
	{
		type = CLIENT_REQUEST;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_MAIN_MENU", MsgType))
	{
		type = CLIENT_MAIN_MENU;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_CPU", MsgType))
	{
		type = CLIENT_CPU;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_VERSUS", MsgType))
	{
		type = CLIENT_VERSUS;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_LEADERBOARD", MsgType))
	{
		type = CLIENT_LEADERBOARD;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_PLAYER_MOVE", MsgType))
	{
		type = CLIENT_PLAYER_MOVE;
		RetVal = GetParams(params, MsgStr);
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_REPLAY", MsgType))
	{
		type = CLIENT_REPLAY;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_REFRESH", MsgType))
	{
		type = CLIENT_REFRESH;
	}
	else if (STRINGS_ARE_EQUAL("CLIENT_DISCONNECT", MsgType))
	{
		type = CLIENT_DISCONNECT;
	}
	else
	{
		return ERROR_TYPE;
	}
	
	if (RetVal != ERROR_RETURN)
	{
		FillMsg(msg, type, params,RetVal);
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
int GetParams(char **params,char *MsgStr)
{
	char TempStr[MAX_MSG_SIZE], *token;
	int i, j=0;
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
	i=1;
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