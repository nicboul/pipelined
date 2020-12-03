
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

int main()
{

	struct sockaddr_un addr;
	int optval = 1;

	int sck;

	sck = socket(PF_UNIX, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	addr.sun_path[0] = 0;
	addr.sun_path[1] = 'b';

	connect(sck, (struct sockaddr*)&addr, sizeof(addr));
	printf("%s\n", strerror(errno));

	for(;;) {
	sleep(2);
	write(sck, "salut\0", 6);
	printf("wrote\n");
	}

}
