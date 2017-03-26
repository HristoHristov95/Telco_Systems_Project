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

           /* ����� �������� ���������� �� ������������*/

           memset(&hints, 0, sizeof(struct addrinfo)); //����� 0-�� � ������� �� ������� �� hints
           hints.ai_family = AF_UNSPEC;    /* ��������� IPv4 ��� IPv6 ������ */
           hints.ai_socktype = SOCK_DGRAM; /* ������ Datagram ��� ����� ����� �������� �� UDP �������� */
           hints.ai_flags = 0; // ��� ������������ �����
           hints.ai_protocol = 0;          /* �������� ��������� */

           s = getaddrinfo(argv[1], argv[2], &hints, &result); // ����� �� ��������� ������������ � ����� � result ������� ������ �� ��������� ���������� �� ���������� � hints
           if (s != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
               exit(EXIT_FAILURE);
           }

           /* �������� ����� �� �������� ��� �������� ������ ������ �����
		   �� ��� ����� �� ������� �������� ��� �������. ��� ������� ����� �������� ��������� ������
		   � ������������ ��� ��������� �����.*/

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (sfd == -1)
                   continue;

               if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
                   break;                  /* �������� ��� ������� � ��������� */

			   close(sfd); /*��������� ������ ���� ��� �������*/
           }

           if (rp == NULL) {               /* �� � ��������� ������ ��� ������� */
               fprintf(stderr, "Could not connect\n");
               exit(EXIT_FAILURE);
           }

           freeaddrinfo(result);           /* ������ �� �� ������ ������� */

           /* ��������� ��������� ��� ������� � ���� ���� ������ ��� �� �� �� ����� */

           for (j = 3; j < argc; j++) {
               len = strlen(argv[j]) + 1;
                       /* +1 �� ������������ ��� */

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
	   // XOR chipher algorith - �� ���������� �� ���������� 
	   // ��� ����� ��������� �� ������������ ������������ �� ����������
	   void encrypt_decrypt(char *sentence)
	   {
		   char key[10] = "ABCDEFGHIJ"; //���� �� ����������
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

