#ifndef __CMD_PROC_H_
#define __CMD_PROC_H_

/* Some defines */
/* Other defines should be return codes of the functions */
/* E.g. #define CMD_EMPTY_STRING -1                      */
/** definir estrutura de comandos e saidas possiveis */
#define MAX_CMDSTRING_SIZE 10   /**< tamanho maximo do comando */ 
#define SOF_SYM '#'             /**< Simbolo de inicio de comando */
#define EOF_SYM '!'             /**< Simbolo de fim de comando */
#define EXIT_SUCCESS    0;      /**< Successfull exit */
#define EMPTY_STRING   -1;      /**< String is empty */
#define STRING_FULL    -1;      /**< String is full */
#define CMD_NOT_FOUND  -2;      /**< Invalid CMD */
#define WRONG_STR_FORMAT -3;    /**< Wrong string format*/


/* Function prototypes */
int CmdChar(unsigned char newChar);
int cmdProcess(void);
void resetcmd(void);

#endif
