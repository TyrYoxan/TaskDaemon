//
// Created by clement on 29/04/23.
//

#ifndef TASKDAEMON_MESSAGE_H
#define TASKDAEMON_MESSAGE_H

int send_string(int fd, char *str);

char *recv_string(int fd);

int send_argv(int fd, char *argv[]);

char **recv_argv(int fd);
#endif //TASKDAEMON_MESSAGE_H
