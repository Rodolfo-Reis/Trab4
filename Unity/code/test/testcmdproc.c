/* Includes */
#include <unity.h>
#include <cmdproc.h>

void setUp(void){
    return;
}
void tearDown(void){
    return;
}

/**
 * @brief Inserçao de comando correto para altera periodo de uma tarefa 
 * 
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ThreadPeriod(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('T'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('t'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('2'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(0,cmdProcess());
}

/**
 * @brief Inserçao de comando correto para alterar estado do led

 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ledstate(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('l'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('1'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(0,cmdProcess());
}

/**
 * @brief Inserçao de comando com estrutura correta e conteudo errado
 * 
 * Estrutura correta para altera o periodo da tarefa mas tem caracteres errados
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ThreadPeriod_er1(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('L'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('t'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('2'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(-2,cmdProcess());
}


/**
 * @brief Inserçao de comando com estrutura correta e conteudo errado
 * 
 * Estrutura correta para altera o periodo da tarefa mas tem caracteres errados
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ThreadPeriod_er2(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('T'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('b'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('a'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(-2,cmdProcess());
}

/**
 * @brief Inserçao de comando com estrutura correta e conteudo errado
 * 
 * Estrutura correta para altera o estado do led mas tem caracteres errados
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ledstate_er1(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('b'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('1'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(-2,cmdProcess());
}

/**
 * @brief Inserçao de comando com estrutura correta e conteudo errado
 * 
 * Estrutura correta para altera o estado do led mas tem um estado errado
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ledstate_er2(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('l'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('3'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('4'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(-2,cmdProcess());
}

/**
 * @brief Inserçao de comando com estrutura correta e conteudo errado
 * 
 * Estrutura correta para altera o estado do led mas tem um led selecionado errado
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_ledstate_er3(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('l'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('6'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(-2,cmdProcess());
}

/**
 * @brief Inserçao de comando com estrutura correta mas nao tem caracter finalizador
 * 
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_NotFinish(void){
    TEST_ASSERT_EQUAL_INT(0,CmdChar('#'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('l'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('6'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(-3,cmdProcess());
}


/**
 * @brief Inserçao de comando com estrutura correta mas nao tem caracter inicializador
 * 
 * 
*/
/* Inserçao de comando aleatorio*/
void test_cmdproc_NotInit(void){
    
    TEST_ASSERT_EQUAL_INT(0,CmdChar('l'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('6'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('0'));
    TEST_ASSERT_EQUAL_INT(0,CmdChar('!'));
    TEST_ASSERT_EQUAL_INT(-3,cmdProcess());
}

/**
 * @brief Main
 * 
 * Dá Run de todos os unity testes
 * 
 * 
*/
int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_cmdproc_ThreadPeriod);
    resetcmd();
    RUN_TEST(test_cmdproc_ledstate);
    resetcmd();
    RUN_TEST(test_cmdproc_ThreadPeriod_er1);
    resetcmd();
    RUN_TEST(test_cmdproc_ThreadPeriod_er2);
    resetcmd();
    RUN_TEST(test_cmdproc_ledstate_er1);
    resetcmd();
    RUN_TEST(test_cmdproc_ledstate_er2);
    resetcmd();
    RUN_TEST(test_cmdproc_ledstate_er3);
    resetcmd();
    RUN_TEST(test_cmdproc_NotFinish);
    resetcmd();
    RUN_TEST(test_cmdproc_NotInit);
    resetcmd();
    return UNITY_END();
}