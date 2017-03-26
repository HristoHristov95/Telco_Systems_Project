 #include <sys/types.h>
       #include <sys/socket.h>
       #include <netdb.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <string.h>

       #define BUF_SIZE 500
       void encrypt_decrypt(char *sentence);
       int main(int argc, char *argv[])
       {
           struct addrinfo hints;
           struct addrinfo *result, *rp;
           int sfd, s, j;
           size_t len;
           ssize_t nread;
           char buf[BUF_SIZE];

           if (argc < 3) {
               fprintf(stderr, "Required: %s <hostname/IP> <port> <msg...>\n", argv[0]);
               exit(EXIT_FAILURE);
           }

           /* Взима адресите отговарящи на изискванията*/

           memset(&hints, 0, sizeof(struct addrinfo)); //слага 0-ли в паметта на мястото на hints
           hints.ai_family = AF_UNSPEC;    /* позволява IPv4 или IPv6 адреси */
           hints.ai_socktype = SOCK_DGRAM; /* оказва Datagram тип сокет който отговаря на UDP протокол */
           hints.ai_flags = 0; // без допълнителни опции
           hints.ai_protocol = 0;          /* всякакви протоколи */

           s = getaddrinfo(argv[1], argv[2], &hints, &result); // взима от терминала информацията и връща в result свързан списак от структури отговарящи на описанието в hints
           if (s != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
               exit(EXIT_FAILURE);
           }

           /* Пробваме всеки от адресите във върнатия списък докато някои
		   от тях успее да направи връзката със сървъра. Ако текущия адрес пропадне изтриваме сокета
		   и продължаваме със следващия адрес.*/

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (sfd == -1)
                   continue;

               if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
                   break;                  /* връзката със сървъра е направена */

			   close(sfd); /*затваряме сокета само при неуспех*/
           }

           if (rp == NULL) {               /* не е направена връзка със сървъра */
               fprintf(stderr, "Could not connect\n");
               exit(EXIT_FAILURE);
           }

           freeaddrinfo(result);           /* повече не ни трябва списъка */

           /* изпращаме съобщение към сървъра и след това чакаме той да ни го върне */

           for (j = 3; j < argc; j++) {
               len = strlen(argv[j]) + 1;
                       /* +1 за терминиращия бит */

               if (len + 1 > BUF_SIZE) {
                   fprintf(stderr,"Ignoring long message in argument %d\n", j);
                   continue;
               }
			   encrypt_decrypt(argv[j]);
			   printf("Here is the encrypted message : %s \n", argv[j]);
               if (write(sfd, argv[j], len) != len) {
                   fprintf(stderr, "failed write\n");
                   exit(EXIT_FAILURE);
               }

               nread = read(sfd, buf, BUF_SIZE);
               if (nread == -1) {
                   perror("read");
                   exit(EXIT_FAILURE);
               }
			   encrypt_decrypt(buf);
               printf("Received %zd bytes: %s\n", nread, buf);
           }

           exit(EXIT_SUCCESS);
       }
	   // XOR chipher algorith - за криптиране на информация 
	   // при второ прилагане на криптирането информацията се декриптира
	   void encrypt_decrypt(char *sentence)
	   {
		   char key[10] = "ABCDEFGHIJ"; //ключ за криптиране
		   int l = strlen(sentence);
		   int counter = 0;
		   for (int i = 0; i<l; i++)
		   {
			   sentence[i] = sentence[i] ^ key[counter++];
			   if (counter == 9)
			   {
				   counter = 0;
			   }
		   }
	   }

