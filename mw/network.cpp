#include <wesock.h>
#include <iostream>

#include "defines.h"
#include "msgtype.h"


using namespace std;

void message_parser(struct wsock *clnt, char *buff)
{
}

void *network_table(void *obj)
{
	struct wsock_table *table = (struct wsock_table *) obj;

	cout<<"execute network-table."<<endl;
	wsock_table_run(table);
	cout<<"network-table is terminated."<<endl;

	exit(1);
	return NULL;
}

#if 1
void f_disconn (struct wsock_table *table, struct wsock *clnt)
{
	printf("%s:%d is disconnected.\n", clnt->addr_info.ch_ip, clnt->addr_info.h_port);
}

void f_recv (struct wsock_table *table, struct wsock *clnt, unsigned char *buff, int read_len, int *offset)
{
	printf("%s:%d receive data(%d bytes).\n", clnt->addr_info.ch_ip, clnt->addr_info.h_port, read_len);
	int tot_len;
f_recv_loop:
	tot_len = *offset + read_len;
	if(tot_len < sizeof(msg_header_t)) {
		*offset = tot_len;
		return;
	}

	msg_header_t *msg = (msg_header_t *) buff;
	if(msg->len > tot_len) {
		*offset = tot_len;
		return;
	}

	// TODO: A message processing
	message_parser(clnt, (char *) buff);
	// end TODO

	if(msg->len < tot_len) {
		memcpy(buff, buff+msg->len, tot_len - msg->len);
		*offset = 0;
		read_len = tot_len - msg->len;
		goto f_recv_loop;
	}
}
#endif

void *network_procedure(void *obj)
{
	struct wsock_table *table = (struct wsock_table *) obj;
	struct wsock *wsock;

	char *buff = new char[255];
	if(buff == NULL) {
		cout<<"malloc is failed."<<endl;
		exit(1);
		return NULL;
	}

	msg_header_t *msg = (msg_header_t *) buff;
	char *data = (char *) &msg[1];
	while(1)
	{
		sleep(1);
		if(table->element_count_current == 0) {
			// connect to server
			//cout<<"try connect to server."<<endl;
#if 1
			if((wsock = wsock_add_new_tcp_client(
					table,
					(char *) "127.0.0.1",
					48583,
					0,
					5,
					NULL,
					&f_recv,
					&f_disconn)) == NULL) {
				printf("wsock_add_new_tcp_client() is failed.\n");
				exit(1);
				return NULL;
			}
#endif
		}
		msg->len = sizeof(msg_header_t);
		strcpy(data, "Hello, world!");
		msg->len += strlen(data) + 1;
		wsock_send(wsock, msg, msg->len);
	}

	exit(1);
	return NULL;
}

int create_network_procedure()
{
	pthread_t pid;
	struct wsock_table *table = NULL;

	if((table = new struct wsock_table) == NULL) {
		cout<<"malloc() failed. (wsock_table)"<<endl;
		return -1;
	}

	if(wsock_create_tcp_table(table, 1, BUFF_MAX, 60) < 0) {
		cout<<"wsock_create_tcp_table() is failed."<<endl;
		return -1;
	}

	if(pthread_create(&pid, NULL, network_table, (void *) table) != 0) {
		cout<<"Create a thread is failed. (network_table)"<<endl;
		return -1;
	}
	if(pthread_create(&pid, NULL, network_procedure, (void *) table) != 0) {
		cout<<"Create a thread is failed. (network_procedure)"<<endl;
		return -1;
	}


	return 0;
}
