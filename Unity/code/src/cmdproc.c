#include <stdio.h>
#include "cmdproc.h"

static unsigned char cmdLen = 0; 
static char cmdString[10];
volatile int SOF_C = -1;
volatile int EOF_C = -1;

/** \brief Funçao para adicionar char introduzido pelo user num buffer 
 * ******************************** 
 * adiciona char á cmdString 	
 * \return 	0: if success 			         		        
 * \return -1: if cmd string full 	    
 * ******************************** 
 * */
int CmdChar(unsigned char newChar) {
	/* If cmd string not full add char to it */
	if (cmdLen < MAX_CMDSTRING_SIZE) {
		cmdString[cmdLen] = newChar;
		cmdLen ++;
		return EXIT_SUCCESS;
	}
	/* If cmd string full return error */
	return STRING_FULL;
}

/** \brief Funçao de reset do buffer de comandos
 * 
 * Resseta o buufer onde estao guardados o comandos bem como o valor de 
 * start of frame e end of frame
 * 
*/
void resetcmd(void) {
	cmdLen = 0;
	SOF_C = -1;
	EOF_C = -1;		
}

/** \brief Processa o cmdString que foi inserido para verificar comando 
 *                                                      
 * \return  0: comando valido e executado                         	    
 * \return	-1: string vazia ou comando incompleto                      
 * \return	-2: comando invalido encontrado                             
 * \return	-3: formato da string incorreto                             
 * ************************************************************ */
int cmdProcess(void){
	int i;
	/*encontrar simbolo inicial de comando e final de comando*/
	/* posiçao do primeiro #*/
	for(i=0;i<cmdLen;i++){
		if(cmdString[i] == '#'){
			SOF_C = i;
			break;
		}
	}
	/*identificar posicao do primeiro !*/
	for(i=0;i<cmdLen;i++){
		if(cmdString[i] == '!'){
			EOF_C = i;
			break;
		}
	}
	/* se encontrou primeiro ! do que #*/
	if(EOF_C < SOF_C){
		SOF_C = -1;
		EOF_C = -1;
	    return WRONG_STR_FORMAT;
	}
	/* se encontrou um # entre o primeiro # e !*/
	for(i=SOF_C+1;i<EOF_C;i++){
		if(cmdString[i] == '#'){
			return WRONG_STR_FORMAT;
		}
	}

	if(SOF_C == -1){
		return WRONG_STR_FORMAT;
	}
	if(EOF_C == -1){
		return WRONG_STR_FORMAT;
	}

	/* Detect empty cmd string */
	if(cmdLen == 0) {
		return EMPTY_STRING;
    } 
	/* ate aqui confirmo que tenho um comando do tipo #...! */
	/* detetar comandos*/
	/* comando para altera tempo(ms) das threads*/
	if(cmdString[SOF_C+1] == 'T' || cmdString[SOF_C+1] == 't'){
		if(cmdLen<8){
			return WRONG_STR_FORMAT;
		}
		for(i = SOF_C+3;i<EOF_C;i++){
			if(cmdString[i] < '0' || cmdString[i] > '9'){
				return CMD_NOT_FOUND;
			}
		}
		/* mudar periodo da thread dos leds*/
		if(cmdString[SOF_C+2] == 'L' || cmdString[SOF_C+2] == 'l'){
			if( (EOF_C - (SOF_C+3)) == 4){
				return EXIT_SUCCESS;
			}
			else{
				return CMD_NOT_FOUND;
			}
			return EXIT_SUCCESS;
		}
		/* mudar periodo da thread dos butoes*/
		else if(cmdString[SOF_C+2] == 'B' || cmdString[SOF_C+2] == 'b'){
			if( (EOF_C - (SOF_C+3)) == 4){
				return EXIT_SUCCESS;
			}
			else{
				return CMD_NOT_FOUND;
			}
			return EXIT_SUCCESS;
		}
		/* mudar periodo da thread da temp*/
		else if(cmdString[SOF_C+2] == 'T' || cmdString[SOF_C+2] == 't'){
			if( (EOF_C - (SOF_C+3)) == 4){
				return EXIT_SUCCESS;
			}
			else{
				return CMD_NOT_FOUND;
			}
		}
		return EXIT_SUCCESS;
	}
	/* verificar se é commando para alterar estado dos leds*/
	else if(cmdString[SOF_C+1] == 'L' || cmdString[SOF_C+1] == 'l'){
		/* tamanho deste tipo de comando é 5*/
		if((EOF_C - SOF_C) > 4 ){
			return CMD_NOT_FOUND;
		}
		else if(cmdString[SOF_C+2] < '0' || cmdString[SOF_C+2] > '3'){
			return CMD_NOT_FOUND;
		} 
		else if(cmdString[SOF_C+3] < '0' || cmdString[SOF_C+3] > '1'){
			return CMD_NOT_FOUND;
		} 
		else{
			if(cmdString[SOF_C+2] == '0'){
				return EXIT_SUCCESS;
			}
			else if(cmdString[SOF_C+2] == '1'){
				return EXIT_SUCCESS;
			}
			else if(cmdString[SOF_C+2] == '2'){
				return EXIT_SUCCESS;
			}
			else if(cmdString[SOF_C+2] == '3'){
				return EXIT_SUCCESS;
			}	
		}
		return EXIT_SUCCESS;
	}
	else{
		return CMD_NOT_FOUND;
	}
	return WRONG_STR_FORMAT;
}
