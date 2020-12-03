#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "pipelined.h"

#define MUXCOM_PORT 2104
#define MUXCOM_BACKLOG 256


struct muxcom_peer {
	struct muxcom_peer *next;
	int sck;
	struct sockaddr_un addr;
};

static struct muxcom_peer *muxcom_peers = NULL;
int muxcom_sck;

/* Reading and writing the sig_atomic_t datatype is
 * guaranteed to happen in a single instruction, so
 * there's no way for a handler to run "in the middle"
 * of an access.
 */
sig_atomic_t peers_waiting = 0;

void add_peer(struct muxcom_peer *peer)
{
	struct muxcom_peer *i;

	if (muxcom_peers != NULL) {

        i = muxcom_peers;
		while (i->next != NULL) {
			i = i->next;
		}

		i->next = peer;
    }

	else {

		muxcom_peers = peer;
	}
}

void del_peer(int sck)
{

    sigset_t block_sigio;
    sigemptyset(&block_sigio);
    sigaddset(&block_sigio, SIGIO);

    struct muxcom_peer *peer;
    int del = 0;

    peer = muxcom_peers;

    if (!muxcom_peers->sck == sck) {

        while (peer->next != NULL && !del) {

            if (peer->next->sck == sck) {

                /* Blocking a signal means telling the
                 * operating system to hold it and deliver
                 * it later. The peers list is shared
                 * between the process and the signal handler,
                 * to prevent race condition, we must keep
                 * this part atomic.
                 */
                sigprocmask(SIG_BLOCK, &block_sigio, NULL);
                peer->next = peer->next->next;
                sigprocmask(SIG_UNBLOCK, &block_sigio, NULL);

                free(peer);

                del = 1;
            }

            peer = peer->next;
        }
    }

    else {

        sigprocmask(SIG_BLOCK, &block_sigio, NULL);
        muxcom_peers = muxcom_peers->next;
        sigprocmask(SIG_UNBLOCK, &block_sigio, NULL);

        free(peer);
    }
}

int poll_sck(int sck)
{
	fd_set rfds;
	struct timeval tv = {0, 0};
	int retval;

	FD_ZERO(&rfds);
	FD_SET(sck, &rfds);

	retval = select(sck + 1, &rfds, NULL, NULL, &tv);

	return retval;
}

/*
 * When a handler functions is invoked on a signal,
 * that signal is automatically blocked.
 */
void muxcom_sigio(int signum)
{
	int sck;
	struct sockaddr_un addr;
	socklen_t addr_len;
	struct muxcom_peer *peer;

	int retval;

	printf("muxcom_sigio\n");

	retval = poll_sck(muxcom_sck);
	if (retval) {

		sck = accept(muxcom_sck, (struct sockaddr*)&addr, &addr_len);

		fcntl(sck, F_SETFL, O_NONBLOCK | O_ASYNC);
		fcntl(sck, F_SETOWN, getpid());

		peer = malloc(sizeof(struct muxcom_peer));
		peer->sck = sck;
		peer->addr = addr;
        peer->next = NULL;

		add_peer(peer);
	}

    else if (muxcom_peers && !peers_waiting) {
		peers_waiting = 1;
	}
}

void serve_peers()
{

    char buf[256];
	struct muxcom_peer *peer;
	int retval;

    memset(buf, 0, 256);

	peer = muxcom_peers;
	while (peer != NULL) {

		retval = poll_sck(peer->sck);
		if (retval) {

			printf("muxcom_rcv()\n");
            // FIXME: read the data into a pre-allocated area
            read(peer->sck, buf, 256);
            printf("block: %s\n", buf);

		}
/* FIXME
        else {
            del_peer(peer->sck);
            // remove peer from list
            // race condition ?!
        }
*/

		peer = peer->next;
	}

	peers_waiting = 0;
}

void muxcom_sched()
{
	for (;;) {
		pause();
		if (peers_waiting) {
			serve_peers();
		}
	}
}

int muxcom_init()
{
	struct sigaction handler;

	/* An alternative to select() is to let the kernel inform the application
	 * about events via a SIGIO signal. For that the O_ASYNC flag must be set
	 * on a socket file descriptor via fcntl(2) and valid signal handler for
	 * SIGIO must be installed via sigaction(2); socket(7)
	 */
	handler.sa_handler = muxcom_sigio;
	sigaction(SIGIO, &handler, 0);

	muxcom_sck_init(MUXCOM_PORT, MUXCOM_BACKLOG);
}

int muxcom_sck_init(int port, int backlog)
{
	struct sockaddr_un addr;
	int optval = 1;

	/* The PF_UNIX socket family is used to communicate between processes on
	 * the same machine efficiently.
	 */
	if ((muxcom_sck = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
		printf("socket\n");
		return -1;
	}

	/* A TCP local socket address that has been bound is unavailable for some
	 * time after closing, unless the SO_REUSEADDR flag has been set. Care should
	 * be taken when using this flag as it makes TCP less reliable; ip(7)
	 */
	setsockopt(muxcom_sck, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	addr.sun_path[0] = 0;
	addr.sun_path[1] = 'b';

	if (bind(muxcom_sck, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("bind\n");
		return -1;
	}

	/* It is possible to do non-blocking I/O on sockets by setting the O_NONBLOCK
	 * flag on a socket file descriptor. Then all operations that would block will
	 * (usually) return EAGAIN (operation should be retired later). The user can
	 * then wait for various events via select(2); socket(7)
	 *
	 * If you set the O_ASYNC status flag on a file descriptor, a SIGIO signal is
	 * sent whenever input or output becomes possible on that file descriptor; fcntl(2)
	 *
	 * You can set the socket to deliver SIGIO when activity occurs on a socket; socket(7)
	 */
	fcntl(muxcom_sck, F_SETFL, O_NONBLOCK | O_ASYNC);
	fcntl(muxcom_sck, F_SETOWN, getpid());

	/* The backlog parameter defines the maximum length the queue of pending
	 * connections may grow to. If a connection request arrives with the queue
	 * full the client may receive an error with an indication of ECONNREFUSED,
	 * or, if the underlying protocol supports retransmission, the request may
	 * be ignored so that retries may succeed.
	 */
	if (listen(muxcom_sck, backlog) == -1) {
		printf("listen\n");
		return -1;
	}

	printf("muxcom initialized\n");
	return 0;
}
