#include "iman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curl/curl.h>

static curl_socket_t opensocket(void *clientp, curlsocktype purpose, struct curl_sockaddr *address) {
    curl_socket_t sockfd = socket(address->family, address->socktype, address->protocol);
    if (sockfd == -1) {
        perror("socket");
    }
    return sockfd;
}

void parsehtml(const char *str, size_t len) {
    int in_tag = 0;
    int newline_flag = 0;

    for (size_t i = 0; i < len; i++) {
        if (str[i] == '<') {
            in_tag = 1;
        } else if (str[i] == '>') {
            in_tag = 0;
        } else if (!in_tag) {
            if (str[i] != '\n' && str[i] != '\r') {
                putchar(str[i]);
                newline_flag = 0;
            } else if (!newline_flag) {
                putchar('\n');
                newline_flag = 1;
            }
        }
    }
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    parsehtml(ptr, total_size);
    return total_size;
}

void fetchContent(const char *ip_address, const char *path) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing libcurl\n");
        return;
    }

    char url[1024];
    snprintf(url, sizeof(url), "http://%s%s", ip_address, path);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, opensocket);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Host: man.he.net");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
}

// ✅ DEFINE imanCommand here
void imanCommand(char *args[]) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: iMan <command>\n");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "/?topic=%s&section=all", args[1]);

    const char *ip = "104.18.191.12";  // hardcoded IP for man.he.net
    fetchContent(ip, path);
}
