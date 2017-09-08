/* Por Prof. Daniel Batista <batista@ime.usp.br>
 * Em 9/8/2017
 * 
 * Um código simples (não é o código ideal, mas é o suficiente para o
 * EP) de um servidor de eco a ser usado como base para o EP1. Ele
 * recebe uma linha de um cliente e devolve a mesma linha. Teste ele
 * assim depois de compilar:
 * 
 * ./servidor 8000
 * 
 * Com este comando o servidor ficará escutando por conexões na porta
 * 8000 TCP (Se você quiser fazer o servidor escutar em uma porta
 * menor que 1024 você precisa ser root).
 *
 * Depois conecte no servidor via telnet. Rode em outro terminal:
 * 
 * telnet 127.0.0.1 8000
 * 
 * Escreva sequências de caracteres seguidas de ENTER. Você verá que
 * o telnet exibe a mesma linha em seguida. Esta repetição da linha é
 * enviada pelo servidor. O servidor também exibe no terminal onde ele
 * estiver rodando as linhas enviadas pelos clientes.
 * 
 * Obs.: Você pode conectar no servidor remotamente também. Basta saber o
 * endereço IP remoto da máquina onde o servidor está rodando e não
 * pode haver nenhum firewall no meio do caminho bloqueando conexões na
 * porta escolhida.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

char* try_to_login(char *user, char *password) {
    if (strcmp(user, "\"romao@test\"") == 0) {
        if (strcmp(password, "\"220294\"\r\n") == 0) {
            return " OK LOGIN completed";
        } else {
            return " NO LOGIN failure";
        }
    } else if (strcmp(user, "\"cesar@test\"") == 0) {
        if (strcmp(password, "\"211292\"\r\n") == 0) {
            return " OK LOGIN completed";
        }
    }
    return " NO LOGIN failure";
}

int main (int argc, char **argv) {
   /* Os sockets. Um que será o socket que vai escutar pelas conexões
    * e o outro que vai ser o socket específico de cada conexão */
	int listenfd, connfd;
   /* Informações sobre o socket (endereço e porta) ficam nesta struct */
	struct sockaddr_in servaddr;
   /* Retorno da função fork para saber quem é o processo filho e quem
    * é o processo pai */
   pid_t childpid;
   /* Armazena linhas recebidas do cliente */
	char	recvline[MAXLINE + 1];
   /* Armazena o tamanho da string lida do cliente */
   ssize_t  n;

   /* variavel para devolver as respostas ao cliente */
   char sendline[MAXLINE + 1];
   
	if (argc != 2) {
      fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
      fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
		exit(1);
	}

   /* Criação de um socket. Eh como se fosse um descritor de arquivo. Eh
    * possivel fazer operacoes como read, write e close. Neste
    * caso o socket criado eh um socket IPv4 (por causa do AF_INET),
    * que vai usar TCP (por causa do SOCK_STREAM), já que o IMAP
    * funciona sobre TCP, e será usado para uma aplicação convencional sobre
    * a Internet (por causa do número 0) */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket :(\n");
		exit(2);
	}

   /* Agora é necessário informar os endereços associados a este
    * socket. É necessário informar o endereço / interface e a porta,
    * pois mais adiante o socket ficará esperando conexões nesta porta
    * e neste(s) endereços. Para isso é necessário preencher a struct
    * servaddr. É necessário colocar lá o tipo de socket (No nosso
    * caso AF_INET porque é IPv4), em qual endereço / interface serão
    * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
    * qual a porta. Neste caso será a porta que foi passada como
    * argumento no shell (atoi(argv[1]))
    */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind :(\n");
		exit(3);
	}

   /* Como este código é o código de um servidor, o socket será um
    * socket passivo. Para isto é necessário chamar a função listen
    * que define que este é um socket de servidor que ficará esperando
    * por conexões nos endereços definidos na função bind. */
	if (listen(listenfd, LISTENQ) == -1) {
		perror("listen :(\n");
		exit(4);
	}

   printf("[Servidor no ar. Aguardando conexoes na porta %s]\n",argv[1]);
   printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
   /* O servidor no final das contas é um loop infinito de espera por
    * conexões e processamento de cada uma individualmente */
	for (;;) {
      /* O socket inicial que foi criado é o socket que vai aguardar
       * pela conexão na porta especificada. Mas pode ser que existam
       * diversos clientes conectando no servidor. Por isso deve-se
       * utilizar a função accept. Esta função vai retirar uma conexão
       * da fila de conexões que foram aceitas no socket listenfd e
       * vai criar um socket específico para esta conexão. O descritor
       * deste novo socket é o retorno da função accept. */
		if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
			perror("accept :(\n");
			exit(5);
		}
      
      /* Agora o servidor precisa tratar este cliente de forma
       * separada. Para isto é criado um processo filho usando a
       * função fork. O processo vai ser uma cópia deste. Depois da
       * função fork, os dois processos (pai e filho) estarão no mesmo
       * ponto do código, mas cada um terá um PID diferente. Assim é
       * possível diferenciar o que cada processo terá que fazer. O
       * filho tem que processar a requisição do cliente. O pai tem
       * que voltar no loop para continuar aceitando novas conexões */
      /* Se o retorno da função fork for zero, é porque está no
       * processo filho. */
      if ( (childpid = fork()) == 0) {
         /**** PROCESSO FILHO ****/
         printf("[Uma conexao aberta]\n");
         /* Já que está no processo filho, não precisa mais do socket
          * listenfd. Só o processo pai precisa deste socket. */
         close(listenfd);
         
         /* Agora pode ler do socket e escrever no socket. Isto tem
          * que ser feito em sincronia com o cliente. Não faz sentido
          * ler sem ter o que ler. Ou seja, neste caso está sendo
          * considerado que o cliente vai enviar algo para o servidor.
          * O servidor vai processar o que tiver sido enviado e vai
          * enviar uma resposta para o cliente (Que precisará estar
          * esperando por esta resposta) 
          */
         strcpy(sendline, "* OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE NAMESPACE STARTTLS AUTH=PLAIN AUTH=LOGIN] Server ready.");
         strcat(sendline, "\r\n");
         write(connfd, sendline, strlen(sendline));
         /* ========================================================= */
         /* ========================================================= */
         /*                         EP1 INÍCIO                        */
         /* ========================================================= */
         /* ========================================================= */
         /* TODO: É esta parte do código que terá que ser modificada
          * para que este servidor consiga interpretar comandos IMAP  */
         while ((n=read(connfd, recvline, MAXLINE)) > 0) {
            recvline[n]=0;
            printf("[Cliente conectado no processo filho %d enviou:] ",getpid());
            if ((fputs(recvline,stdout)) == EOF) {
               perror("fputs :( \n");
               exit(6);
            }
            char command[MAXLINE + 1];
            char* tag;
            const char delimiter[2] = " ";
            strcpy(command, recvline);
            char *token = strtok(command, delimiter);
            tag = token;
            int logged = 0;
            while (token != NULL) {
                if (strcmp("login", token) == 0) {
                    char* user = strtok(NULL, delimiter);
                    char* password = strtok(NULL, delimiter);
                    printf("tentando logar como %s com senha %s\n\n", user, password);
                    printf("%s\n\n", try_to_login(user, password));
                    strcpy(sendline, tag);
                    strcat(sendline, try_to_login(user, password));
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("CAPABILITY\r\n", token) == 0) {
                    strcpy(sendline, "* OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE NAMESPACE STARTTLS AUTH=PLAIN AUTH=LOGIN] Server ready.");
                    strcat(sendline, "\r\n");
                    strcat(sendline, tag);
                    strcat(sendline, " OK CAPABILITY completed\r\n");
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("authenticate", token) == 0) {
                    strcpy(sendline, tag);
                    strcat(sendline, " NO [UNAVAILABLE]");
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline)
;                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("ID", token) == 0) {
                    strcpy(sendline, tag);
                    strcat(sendline, " ID (\"name\" \"Our IMAP server\")");
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline)
;                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("list", token) == 0) {
                    char* command = strtok(NULL, delimiter);
                    if (strcmp("(subscribed)", command) == 0) {
                      strcpy(sendline, tag);
                      strcat(sendline, " OK List completed");
                      strcat(sendline, "\r\n");
                    }
                    printf("mandando pro cliente: %s", sendline)
;                   write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("noop", token) == 0) {
                    char* command = strtok(NULL, delimiter);
                    if (strcmp("(subscribed)", command) == 0) {
                      strcpy(sendline, tag);
                      strcat(sendline, " OK NOOP completed");
                      strcat(sendline, "\r\n");
                    }
                    printf("mandando pro cliente: %s", sendline)
;                   write(connfd, sendline, strlen(sendline));
                    break;
                }
                token = strtok(NULL, delimiter);
            }

         }
         /* ========================================================= */
         /* ========================================================= */
         /*                         EP1 FIM                           */
         /* ========================================================= */
         /* ========================================================= */

         /* Após ter feito toda a troca de informação com o cliente,
          * pode finalizar o processo filho */
         printf("[Uma conexao fechada]\n");
         exit(0);
      }
      /**** PROCESSO PAI ****/
      /* Se for o pai, a única coisa a ser feita é fechar o socket
       * connfd (ele é o socket do cliente específico que será tratado
       * pelo processo filho) */
		close(connfd);
	}
	exit(0);
}
