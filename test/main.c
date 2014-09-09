#include "common.h"
#include "sysutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"

void print_conf();

int main(int argc, const char *argv[])
{
    if(getuid())
    {
        //服务器端运行时没有权限给出提示信息
        fprintf(stderr, "FtpServer must be started by root!\n");
        exit(EXIT_FAILURE);
    }

    //处理僵尸进程
    if(signal(SIGCHLD, SIG_IGN) == SIG_ERR)
        ERR_EXIT("signal");

    parseconf_load_file("ftpserver.conf");
    print_conf();

    //创建一个监听
    int listenfd = tcp_server(tunable_listen_address, tunable_listen_port);
    //int listenfd = tcp_server(NULL, 9981);

    pid_t pid;
    session_t ses;
    session_init(&ses);
    while(1)
    {
        //int peerfd = accept_timeout(listenfd, NULL, 10);
        int peerfd = accept_timeout(listenfd, NULL, tunable_accept_timeout);
        if(peerfd == -1 && errno == ETIMEDOUT)
            continue;
        else if(peerfd == -1)
            ERR_EXIT("accept_timeout");

        if((pid = fork()) == -1)
            ERR_EXIT("fork");
        else if(pid == 0)
        {
            close(listenfd);

            ses.peer_fd = peerfd;
            session_begin(&ses);

            //这里保证每次成功执行后退出循环
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(peerfd);
        }
    }
}

void print_conf()
{
    printf("tunable_pasv_enable=%d\n", tunable_pasv_enable);
    printf("tunable_port_enable=%d\n", tunable_port_enable);

    printf("tunable_listen_port=%u\n", tunable_listen_port);
    printf("tunable_max_clients=%u\n", tunable_max_clients);
    printf("tunable_max_per_ip=%u\n", tunable_max_per_ip);
    printf("tunable_accept_timeout=%u\n", tunable_accept_timeout);
    printf("tunable_connect_timeout=%u\n", tunable_connect_timeout);
    printf("tunable_idle_session_timeout=%u\n", tunable_idle_session_timeout);
    printf("tunable_data_connection_timeout=%u\n", tunable_data_connection_timeout);
    printf("tunable_local_umask=0%o\n", tunable_local_umask);
    printf("tunable_upload_max_rate=%u\n", tunable_upload_max_rate);
    printf("tunable_download_max_rate=%u\n", tunable_download_max_rate);

    if (tunable_listen_address == NULL)
        printf("tunable_listen_address=NULL\n");
    else
        printf("tunable_listen_address=%s\n", tunable_listen_address);
}
