#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095
#define FROM "******@qq.com"

char buf[MAX_SIZE+1];

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";

    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name

    const unsigned short port = 25; // SMTP server port

    const char* user = "******@qq.com"; // TODO: Specify the user
    const char* pass = "******"; // TODO: Specify the password
    const char* from = FROM; // TODO: Specify the mail address of the sender

    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    // START
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(dest_ip);
    if (connect(s_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    // END

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // Send EHLO command and print server response
    const char* EHLO = "EHLO qq.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);

    // TODO: Print server response to EHLO command
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Authentication. Server response should be printed out.
    // START
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    char* user_base64 = encode_str(user);
    size_t len = strlen(user_base64) + 3;
    char* USER = (char*)malloc(len);
    memset(USER, 0, len);
    memcpy(USER, user_base64, len - 3);
    memcpy(USER + len - 3, "\r\n", 2);
    send(s_fd, USER, len - 1, 0);
    free(user_base64);
    free(USER);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    char* pass_base64 = encode_str(pass);
    len = strlen(pass_base64) + 3;
    char* PASS = (char*)malloc(len);
    memset(PASS, 0, len);
    memcpy(PASS, pass_base64, len - 3);
    memcpy(PASS + len - 3, "\r\n", 2);
    send(s_fd, PASS, len - 1, 0);
    free(pass_base64);
    free(PASS);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);
    //END

    // TODO: Send MAIL FROM command and print server response
    const char* SENDER = "MAIL FROM:<" FROM ">\r\n";
    send(s_fd, SENDER, strlen(SENDER), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);
    
    // TODO: Send RCPT TO command and print server response
    len = strlen(receiver) + 13;
    char* RECEIVER = (char*)malloc(len);
    memset(RECEIVER, 0, len);
    memcpy(RECEIVER, "RCPT TO:<", 9);
    memcpy(RECEIVER + 9, receiver, len - 13);
    memcpy(RECEIVER + len - 4, ">\r\n", 3);
    send(s_fd, RECEIVER, len - 1, 0);
    free(RECEIVER);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);
    
    // TODO: Send DATA command and print server response
    const char* DATA = "data\r\n";
    send(s_fd, DATA, strlen(DATA), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send message data
    // START
    char message[10004];
    memset(message, 0, sizeof(message));
    FILE* msg_file = fopen(msg, "r");
    if(msg_file == NULL)
        memcpy(message, msg, strlen(msg));
    else
    {
        char c = 0;
        int j = 0;
        for( ; (c = fgetc(msg_file)) != EOF && j < 9999; ++j)
            message[j] = c;
        fclose(msg_file);
        memcpy(message + j, "\r\n\r\n", 4);
    }

    FILE* att_file = fopen(att_path, "r");
    FILE* att_base64file = fopen("att_base64file", "w");
    if(att_file != NULL && att_base64file != NULL)
        encode_file(att_file, att_base64file);
    if(att_file != NULL)
        fclose(att_file);
    if(att_base64file != NULL)
        fclose(att_base64file);

    char attachment[100004];
    memset(attachment, 0, sizeof(attachment));
    FILE* att_base64_file = fopen("att_base64file", "r");
    if(att_base64_file != NULL)
    {
        char c = 0;
        int j = 0;
        for( ; (c = fgetc(att_base64_file)) != EOF && j < 99999; ++j)
            attachment[j] = c;
        fclose(att_base64_file);
        memcpy(attachment + j, "\r\n\r\n", 4);
    }

    const char* FROM1 = "From: " FROM "\r\n";
    send(s_fd, FROM1, strlen(FROM1), 0); 

    len = strlen(receiver) + 7;
    char* TO2 = (char*)malloc(len);
    memset(TO2, 0, len);
    memcpy(TO2, "To: ", 4);
    memcpy(TO2 + 4, receiver, len - 7);
    memcpy(TO2 + len - 3, "\r\n", 2);
    send(s_fd, TO2, len - 1, 0);
    free(TO2);

    const char* MIME_Version = "MIME-Version: 1.0\r\n";
    send(s_fd, MIME_Version, strlen(MIME_Version), 0);

    const char* Content_Type = "Content-Type: multipart/mixed; boundary=xxxxxxxxx\r\n";
    send(s_fd, Content_Type, strlen(Content_Type), 0);

    len = strlen(subject) + 14;
    char* SUBJECT = (char*)malloc(len);
    memset(SUBJECT, 0, len);
    memcpy(SUBJECT, "Subject: ", 9);
    memcpy(SUBJECT + 9, subject, len - 14);
    memcpy(SUBJECT + len - 5, "\r\n\r\n", 4);
    send(s_fd, SUBJECT, len - 1, 0);
    free(SUBJECT);

    const char* boundary = "--xxxxxxxxx\r\n";
    send(s_fd, boundary, strlen(boundary), 0);

    const char* encoding = "Content-Transfer-Encoding: base64\r\n";
    send(s_fd, encoding, strlen(encoding), 0);

    const char* app_header = "Content-Type: application/zip; name=";
    size_t len1 = strlen(app_header);
    len = len1 + strlen(att_path) + 5;
    char* APP_HEADER = (char*)malloc(len);
    memset(APP_HEADER, 0, len);
    memcpy(APP_HEADER, app_header, len1);
    memcpy(APP_HEADER + len1, att_path, len - len1 - 5);
    memcpy(APP_HEADER + len - 5, "\r\n\r\n", 4);
    send(s_fd, APP_HEADER, len - 1, 0);
    free(APP_HEADER);

    send(s_fd, attachment, strlen(attachment), 0);

    send(s_fd, boundary, strlen(boundary), 0);

    const char * TEXT_HEADER = "Content-Type: text/plain\r\n\r\n";
    send(s_fd, TEXT_HEADER, strlen(TEXT_HEADER), 0);

    send(s_fd, message, strlen(message), 0);

    send(s_fd, boundary, strlen(boundary), 0);
    // END


    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send QUIT command and print server response
    const char* QUIT = "quit\r\n";
    send(s_fd, QUIT, strlen(QUIT), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
