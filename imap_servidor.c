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
#include <sys/stat.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>


#define TRUE 1
#define FALSE 0

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

char* try_to_login(char *user, char *password) {
    if (strcmp(user, "romao@test") == 0) {
        if (strcmp(password, "\"220294\"\r\n") == 0) {
            return " OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE SORT SORT=DISPLAY THREAD=REFERENCES THREAD=REFS THREAD=ORDEREDSUBJECT MULTIAPPEND URL-PARTIAL CATENATE UNSELECT CHILDREN NAMESPACE UIDPLUS LIST-EXTENDED I18NLEVEL=1 CONDSTORE QRESYNC ESEARCH ESORT SEARCHRES WITHIN CONTEXT=SEARCH LIST-STATUS BINARY MOVE NAMESPACE NOTIFY COMPRESS=DEFLATE QUOTA] Logged in";
        } else {
            return " NO LOGIN failure";
        }
    } else if (strcmp(user, "cesar@test") == 0) {
        if (strcmp(password, "\"211292\"\r\n") == 0) {
            return " OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE SORT SORT=DISPLAY THREAD=REFERENCES THREAD=REFS THREAD=ORDEREDSUBJECT MULTIAPPEND URL-PARTIAL CATENATE UNSELECT CHILDREN NAMESPACE UIDPLUS LIST-EXTENDED I18NLEVEL=1 CONDSTORE QRESYNC ESEARCH ESORT SEARCHRES WITHIN CONTEXT=SEARCH LIST-STATUS BINARY MOVE NAMESPACE NOTIFY COMPRESS=DEFLATE QUOTA] Logged in";
        }
    }
    return " NO LOGIN failure";
}

char * file_name_from_uid(char *uid, char *user) {
  FILE *fp;
  char fileLocation[100];
  strcpy(fileLocation, user);
  strcat(fileLocation, "/Maildir/uidmap");
  fp = fopen(fileLocation, "r");
  char line[256];
  const char delimiter[2] = " ";
  char *fileName = malloc(sizeof (char) * 100); 
  while (fgets(line, sizeof(line), fp)) {
      char *uidFromLine = strtok(line, delimiter);
      if (strcmp(uid, uidFromLine) == 0) {
        strcpy(fileName, strtok(NULL, delimiter));
        break;
      }
  }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */
  fclose(fp);
  return fileName;

}


int get_filesize_from(char *uid, char *user) {
    char file_path[100];
    struct stat st;
    strcpy(file_path, user);
    strcat(file_path, "/Maildir/cur/");
    char *fileName = file_name_from_uid(uid, user);
    strcat(file_path, fileName);
    // free(fileName);
    if (stat(file_path, &st) == 0) 
        return st.st_size;
    return -1;
}

char *get_body_from_uid(char *uid, char*user) {
  FILE *fp;
  char fileLocation[100];
  strcpy(fileLocation, user);
  strcat(fileLocation, "/Maildir/cur/");
  char *fileName = file_name_from_uid(uid, user);
  strcat(fileLocation, fileName);
  // free(fileName);
  fp = fopen(fileLocation, "r");

  char *body;
  // int fileSize = get_filesize_from(uid, user);
  body = malloc(sizeof (char) * 10000);
  char line[256];
  strcpy(body, "");
  int i = 1;
  while (fgets(line, sizeof(line), fp)) {
      i++;
      strcat(body, line);
  }  
  fclose(fp);
  return body;

}

char * get_header_from_uid(char *uid, char *user) {
    char from[100];
    char to[100];
    char subject[100];
    char date[100];
    char message_id[100];
    char content_type[100];
    char read_part[50];
    char aux_from[100];
    int read_content_type = FALSE;
    
    char file_path[50];
    strcpy(file_path, user);
    strcat(file_path, "/Maildir/cur/");
    char *fileName = file_name_from_uid(uid, user);
    strcat(file_path, fileName);
    // free(fileName);

    char *header = malloc(sizeof (char) * 2000);

    FILE *email;
    email = fopen(file_path, "r");
    while (fscanf(email, "%s", read_part) != EOF) {
        if (strcmp(read_part, "From:") == 0) {
            strcpy(aux_from, read_part);
            fgets(aux_from + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "To:") == 0) {
            strcpy(to, read_part);
            fgets(to + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Subject:") == 0) {
            strcpy(subject, read_part);
            fgets(subject + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Date:") == 0) {
            strcpy(date, read_part);
            fgets(date + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Message-id:") == 0 || strcmp(read_part, "Message-ID:") == 0) {
            strcpy(message_id, read_part);
            fgets(message_id + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Content-Type:") == 0) { 
            if (read_content_type == FALSE) {
                strcpy(content_type, read_part);
                fgets(content_type + strlen(read_part), 100 - strlen(read_part), email);
                read_content_type = TRUE;
            }
        }
    }
    fclose(email);
    strncpy(from, aux_from, strlen(aux_from));
    strcpy(header, "");
    strcat(header, from);
    strcat(header, content_type);
    strcat(header, subject);
    strcat(header, message_id);
    strcat(header, date);
    strcat(header, to);
    strcat(header, "\r\n");

    return header;
}

void update_uidmap(char *oldName, char *newName, char* user) {
  FILE *fp;
  FILE *fp2;
  char fileLocation[100];
  strcpy(fileLocation, user);
  strcat(fileLocation, "/Maildir/uidmap");
  fp = fopen(fileLocation, "r");

  char newFileLocation[100];
  strcpy(newFileLocation, fileLocation);
  strcat(newFileLocation, "new");
  fp2 = fopen(newFileLocation, "w");

  char line[256];
  const char delimiter[2] = " ";
  char fileName[100];
  char holder[30];
  char newLine[256];

  while (fgets(line, sizeof(line), fp)) {
      char *uidFromLine = strtok(line, delimiter);
      strcpy(fileName, strtok(NULL, delimiter));
      strcpy(holder, strtok(NULL, delimiter));
      strcpy(newLine, "");
      strcat(newLine, uidFromLine);
      strcat(newLine, " ");
      if (strcmp(fileName, oldName) == 0){
        strcat(newLine, newName);
      } else {
        strcat(newLine, fileName);
      }
      strcat(newLine, " ");
      strcat(newLine, holder);
      fprintf(fp2, "%s", newLine);
  }  
  remove(fileLocation);
  rename(newFileLocation, fileLocation);
  fclose(fp);
  fclose(fp2);

}

void mark_as_unread(char *messageName, char *user) {   

   char location[100];
   char newName[100];
   char oldName[100];
   strncpy(newName, messageName, strlen(messageName) - 1);

   strcpy(location, user);
   strcat(location, "/Maildir/cur/");

   strcpy(oldName, location);
   strcat(oldName, messageName);

   strcat(location, newName);

   rename(oldName, location);
   update_uidmap(messageName, newName, user);

}

void mark_as_trashed(char *messageName, char *user) {
   char location[100];
   strcpy(location, user);
   strcat(location, "/Maildir/cur/");
   char newName[100];
   char oldName[100];
   strcpy(newName, messageName);
   strcat(location, newName);
   strcpy(oldName, location);
   strcat(location, "T");
   rename(oldName, location);
   strcpy(newName, messageName);
   strcat(newName, "T");
   update_uidmap(messageName, newName, user);
}

void mark_as_read(char *messageName, char *user) {
   char location[100];
   strcpy(location, user);
   strcat(location, "/Maildir/cur/");
   char newName[100];
   char oldName[100];
   strcpy(newName, messageName);
   strcat(location, newName);
   strcpy(oldName, location);
   strcat(location, "S");
   rename(oldName, location);
   strcpy(newName, messageName);
   strcat(newName, "S");
   update_uidmap(messageName, newName, user);
}


char * get_flags_from_uid(char *uid, char *user) {
  char *flags = malloc(sizeof (char) * 50);
  char* fileName = file_name_from_uid(uid, user);
  char* tags;
  char* tags2;
  strcpy(flags, "FLAGS ");
  const char delimiter[2] = ",";
  strcat(flags, "(");
  tags = strtok(fileName, delimiter);
  tags = strtok(NULL, delimiter);
  if (tags) {
    if (strcmp("S", tags) == 0) {
      strcat(flags, "\\Seen");
    }  else {
        strcat(flags, "\\Deleted");
    }
  }
  strcat(flags, ")"); 
  // free(fileName);
  return flags;
}

char *fetch_infos_for(char *uid, char *user, char *arguments) {
    char *response = malloc(sizeof(char) * 2000);
    char argument_list[500];
    int i;
    char delimiter[2] = " ";
    for (i = 1; arguments[i] != ')'; i++) {
        argument_list[i - 1] = arguments[i];
    }
    argument_list[i - 1] = '\0';
    strcpy(response, "(");
    strcat(response, "UID ");
    strcat(response, uid);
    strcat(response, " ");
    if (strstr(argument_list, "RFC822.SIZE") != NULL) {
        char buffer[30];
        sprintf(buffer, "RFC822.SIZE %d ", get_filesize_from(uid, user));
        strcat(response, buffer);
    }
    if (strstr(argument_list, "FLAGS") != NULL) {
        fprintf(stdout, "pediram flags tambem\n");
        char *flags = get_flags_from_uid(uid, user);
        strcat(response, flags);
        strcat(response, " ");
        // free(flags);
    }
    if (strstr(argument_list, "BODY.PEEK") != NULL) {
        if (strstr(argument_list, "HEADER.FIELDS") != NULL) {
            strcat(response, "BODY[HEADER.FIELDS (FROM TO CC BCC SUBJECT DATE MESSAGE-ID PRIORITY X-PRIORITY REFERENCES NEWSGROUP IN-REPLY-TO CONTENT-TYPE REPLY-TO)] {");
            char *header = get_header_from_uid(uid, user);
            char numb[15];
            sprintf(numb, "%ld", strlen(header));
            strcat(response, numb);
            strcat(response, "}\r\n");
            strcat(response, header);
            // free(header);
        }
        if (strstr(argument_list, "BODY.PEEK[]") != NULL) {
            strcat(response, "BODY.PEEK[] ");
            char *body = get_body_from_uid(uid, user);
            strcat(response, body);
            // free(body);
            strcat(response, "\r\n");
        }
    } 
    return response;
}


char * fetch_for(char *uids, char *user, char *params) {
  FILE *fp;
  char fileLocation[100];
  strcpy(fileLocation, user);
  strcat(fileLocation, "/Maildir/uidmap");
  fp = fopen(fileLocation, "r");
  char line[256];
  int i = 1;
  const char delimiter[2] = " ";
  char *ans = malloc(sizeof (char) * MAXLINE + 1);
  char numb[15];
  if (strcmp("1:*", uids) == 0) {
    fputs(params, stdout);
    fputs(user, stdout);
    fputs("\n", stdout);
    fflush(stdout);
    strcpy(ans, "");
    while (fgets(line, sizeof(line), fp)) {
        strcat(ans, "* ");
        sprintf(numb, "%d", i);
        char *uidFromLine = strtok(line, delimiter);
        strcat(ans, numb);
        strcat(ans, " FETCH ");
        char *infos = fetch_infos_for(uidFromLine, user, params);
        strcat(ans, infos);
        strcat(ans, ")\r\n");
        // free(infos);
        i++;
    }
  } else if (strlen(uids) > 1) {
    strcpy(ans, "");
    char *ptr;
    long start = strtol(uids, &ptr, 10);
    long end = strtol(uids + 2, &ptr, 10);
    long int j = start;
    while (fgets(line, sizeof(line), fp)) {
        if (j > end) break;
        char *uidFromLine = strtok(line, delimiter);
        long int uidLong = strtol(uidFromLine, &ptr, 10);
        if (j == uidLong) {
        strcat(ans, "* ");
        sprintf(numb, "%d", i);
        strcat(ans, numb);
        strcat(ans, " FETCH ");
        char *infos = fetch_infos_for(uidFromLine, user, params);
        strcat(ans, infos);
        strcat(ans, ")\r\n");
        // free(infos);
        i++;
        j++;
      }
    }
  } else {
    strcpy(ans, "");
    while (fgets(line, sizeof(line), fp)) {
        char *uidFromLine = strtok(line, delimiter);
        if (strcmp(uids, uidFromLine) == 0) {
        strcat(ans, "* 1");
        strcat(ans, " FETCH ");
        char *infos = fetch_infos_for(uidFromLine, user, params);
        strcat(ans, infos);
        strcat(ans, ")\r\n");
        // free(infos);
        break;
      }
    }
  }
  printf("olha lah o ans: %s\n", ans);
  fclose(fp);
  return ans;

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
   char *bodySendline;
   
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
        /* variavel para guardar o usuario */
         char user[50];
     
         /* Agora pode ler do socket e escrever no socket. Isto tem
          * que ser feito em sincronia com o cliente. Não faz sentido
          * ler sem ter o que ler. Ou seja, neste caso está sendo
          * considerado que o cliente vai enviar algo para o servidor.
          * O servidor vai processar o que tiver sido enviado e vai
          * enviar uma resposta para o cliente (Que precisará estar
          * esperando por esta resposta) 
          */
         strcpy(sendline, "* OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE NAMESPACE STARTTLS AUTH=PLAIN AUTH=LOGIN] Server ready.");
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
                    char* user_login = strtok(NULL, delimiter);
                    char* password = strtok(NULL, delimiter);
                    strncpy(user, 1 + user_login, strlen(user_login) - 2);
                    printf("%s\n\n", try_to_login(user, password));
                    strcpy(sendline, tag);
                    strcat(sendline, try_to_login(user, password));
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("CAPABILITY\r\n", token) == 0) {
                    strcpy(sendline, "* OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE NAMESPACE STARTTLS AUTH=PLAIN AUTH=LOGIN] Server ready.");
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
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("ID", token) == 0) {
                    strcpy(sendline, tag);
                    strcat(sendline, " ID (\"name\" \"Our IMAP server\")");
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("list", token) == 0) {
                    char* command = strtok(NULL, delimiter);
                    if (strcmp("(subscribed)", command) == 0) {
                      strcpy(sendline, tag);
                      strcat(sendline, " OK List completed");
                      strcat(sendline, "\r\n");
                    } else {
                        if (strcmp("\"\"", command) == 0) {
                            char* dir = strtok(NULL, delimiter);
                            if (strcmp("\"*\"\r\n", dir) == 0) {
                                strcpy(sendline, "* LIST (\\HasNoChildren) \".\" INBOX\r\n");
                                strcat(sendline, tag);
                                strcat(sendline, " OK LIST completed");
                                strcat(sendline, "\r\n");
                            }
                        }
                    }
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("uid", token) == 0 || strcmp("UID", token) == 0 ) {
                    char* command = strtok(NULL, delimiter);
                    char* uid = strtok(NULL, delimiter);
                    fflush(stdout);
                    if (strcmp("store", command) == 0) {
                      if (strlen(uid) > 1) {
                        strcpy(sendline, "NO [UNAVAILABLE]\r\n");
                        write(connfd, sendline, strlen(sendline));
                      }
                      char* action = strtok(NULL, delimiter);
                      char* flags = strtok(NULL, delimiter);
                      if (strcmp("(\\Seen)\r\n", flags) == 0) {
                        if (strcmp("+Flags", action) == 0) {
                          char *fileName = file_name_from_uid(uid, user);
                          mark_as_read(fileName, user);
                          // free(fileName);
                        } else if (strcmp("-Flags", action) == 0) {
                          char *fileName = file_name_from_uid(uid, user);
                          mark_as_unread(fileName, user);
                          // free(fileName);
                        }
                      } else if (strcmp("(\\Seen", flags) == 0) {
                        char *fileName = file_name_from_uid(uid, user);
                        mark_as_trashed(fileName, user);
                        // free(fileName);
                      }
                      strcpy(sendline, fetch_for(uid, user, "(FLAGS)\r\n"));
                      strcat(sendline, tag);
                      strcat(sendline, " OK Store complete\r\n");   
                    }
                    else if (strcmp("fetch", command) == 0) {
                        char arguments[100]; 
                        char *argument  =  strtok(NULL, delimiter);
                        strcpy(arguments, argument);
                        while (argument != NULL) {
                            argument = strtok(NULL, delimiter);
                            if (argument != NULL) {
                                strcat(arguments, " ");
                                strcat(arguments, argument);
                            }
                        }
                        strcpy(sendline, fetch_for(uid, user, arguments));
                        strcat(sendline, tag);
                        strcat(sendline, " OK Fetch complete\r\n");   
                      }
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("noop\r\n", token) == 0) {
                    char* command = strtok(NULL, delimiter);
                    strcpy(sendline, tag);
                    strcat(sendline, " OK NOOP completed");
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline);                   
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("namespace\r\n", token) == 0) {
                  strcpy(sendline, "* NAMESPACE ((\"INBOX.\" \".\")) NIL NIL\r\n");
                  strcat(sendline, tag);
                  strcat(sendline, " OK Namespace completed");
                  strcat(sendline, "\r\n");               
                  write(connfd, sendline, strlen(sendline));
                  break;

                }
                if (strcmp("logout\r\n", token) == 0 || strcmp("close\r\n", token) == 0) {
                    strcpy(sendline, "");
                    strcat(sendline, "* OK Logout completed");
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline);                   
                    write(connfd, sendline, strlen(sendline));
                    printf("[Uma conexao fechada]\n");
                    exit(0);
                }
                if (strcmp("getquotaroot", token) == 0) {
                    char* selected = strtok(NULL, delimiter);
                    if (strcmp("\"INBOX\"\r\n", selected) == 0) {
                      strcpy(sendline, "");
                      strcat(sendline, "* QUOTAROOT INBOX Mailbox \"cPanel Account\"\r\n");
                      strcat(sendline, "* QUOTA Mailbox (STORAGE 1279 51200)\r\n");
                      strcat(sendline, "* QUOTA \"cPanel Account\" (STORAGE 0 1024000000)\r\n");
                      strcat(sendline, tag);
                      strcat(sendline, " OK Getquotaroot completed");
                      strcat(sendline, "\r\n");
                    }
                    printf("mandando pro cliente: %s", sendline);                   
                    write(connfd, sendline, strlen(sendline));
                    break;
                }
                if (strcmp("select", token) == 0) {
                    strcpy(sendline, "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)\r\n");
                    strcat(sendline, "* OK [PERMANENTFLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft \\*)] Flags permitted.\r\n");
                    char directory[50];
                    if (strcmp(user, "romao@test") == 0) {
                        strcpy(directory, "romao@test/Maildir");
                    } else {
                        strcpy(directory, "cesar@test/Maildir");
                    }
                    DIR *cur, *new;
                    char cur_dir[100];
                    char new_dir[100];
                    char buffer[20];
                    int exists = 0;
                    struct dirent * entry;
                    strcpy(cur_dir, directory);
                    strcpy(new_dir, directory);
                    strcat(cur_dir, "/cur");
                    strcat(new_dir, "/new");
                    new = opendir(new_dir);
                    /* conta quantos emails sao novos*/
                    while ((entry = readdir(new)) != NULL) {
                        if (entry->d_type == DT_REG) {
                            char new_filename[256];
                            char old_filename[256];
                            strcpy(old_filename, new_dir);
                            strcat(old_filename, "/");
                            strcat(old_filename, entry->d_name);
                            strcpy(new_filename, cur_dir);
                            strcat(new_filename, "/");
                            strcat(new_filename, entry->d_name);
                            strcat(new_filename, ":2,");
                            rename(old_filename, new_filename);
                        }
                    }
                    closedir(new);
                    cur = opendir(cur_dir);
                    /* conta quantos emails estao em cur, ou seja, ja foram lidos, mas nao apagados */
                    while ((entry = readdir(cur)) != NULL) {
                        if (entry->d_type == DT_REG) {
                            exists++;
                        }
                    }
                    closedir(cur);
                    //manda a mensagem com quantos existem no total
                    sprintf(buffer, "* %d EXISTS\r\n", exists);
                    strcat(sendline, buffer);
                    strcat(sendline, "* 0 RECENT\r\n");
                    // strcat(sendline, "* OK [UNSEEN 4] Message 4 is first unseen\r\n");
                    strcat(sendline, "* OK [UIDVALIDITY 3857529045] UIDs valid\r\n");
                    strcat(sendline, "* OK [UIDNEXT 4] Predicted next UID\r\n");
                    strcat(sendline, tag);
                    strcat(sendline, " OK [READ-WRITE] SELECT completed");
                    strcat(sendline, "\r\n");
                    printf("mandando pro cliente: %s", sendline);
                    write(connfd, sendline, strlen(sendline));
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
