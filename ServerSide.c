
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

int main(int argc, char *argv[])
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	ssize_t nread;
	char buf[BUF_SIZE];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Позволява IPv4 и IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram тип на сокет отговарящ на UDP протокол */
	hints.ai_flags = AI_PASSIVE;    /* За Wild card IP адреси */
	hints.ai_protocol = 0;          /* Не конкретизиран протокол за връзка */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(NULL, argv[1], &hints, &result); /* връща списък от структури в result с адресите.  ЗАБЕЛЕЖКА: когато
														сме оказали AI_PASSIVE флаг трябва задължително IP-то да е NULL понеже се изисква
														за да не се пренебрегне флага*/
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* Тук минаваме през списъка от адреси като търсим за клиент с който да се bind()-ним.
	Ако текущия адрес пропадне минаваме към следващия като стария сокет го затваряме.*/

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
			rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;                  /* Успешно bind()-ване с клиент */

		close(sfd);
	}

	if (rp == NULL) {               /* Не сме успели да сме свържем */
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* не ни трябва вече адресите */

	/* Приемаме пакета и го връщаме обратно */

	for (;;) {
		peer_addr_len = sizeof(struct sockaddr_storage);
		nread = recvfrom(sfd, buf, BUF_SIZE, 0,(struct sockaddr *) &peer_addr, &peer_addr_len);
		if (nread == -1)
			continue;               /* Игнорираме провалена заявка*/

		char host[NI_MAXHOST], service[NI_MAXSERV];

		s = getnameinfo((struct sockaddr *) &peer_addr,peer_addr_len, host, NI_MAXHOST,service, NI_MAXSERV, NI_NUMERICSERV); // обратната функция на getaddrinfo тоест тази функция извлича информация за потребителско име и адрес от структурата в която се пази
																															 // цялата информация
		if (s == 0){
			printf("Received %zd bytes from %s:%s\n", nread, host, service);
			printf("The message that was received is : %s \n", buf);
		}
		else
		{
			fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
		}
		if (sendto(sfd, buf, nread, 0,(struct sockaddr *) &peer_addr, peer_addr_len) != nread)
			fprintf(stderr, "Error sending response\n");
	}
}
