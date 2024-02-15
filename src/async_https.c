#include "async_https.h"
#include <stdio.h>
#include <pthread.h>
#include <curl/curl.h>
#include "async_https.h"
#include "endian.h"
#include "kms.h"
#include "helpers.h"
#include "output.h"
#include "shared_globals.h"

#define NUMT 1

struct pull_one_url_arg_struct
{
    char *url;
    char *apiKey;
    char *document;
};

int getDocument(char *buffer, size_t buffer_length,
                const char *clientGuid,
                const char *clientName,
                const char *productName,
                const char *clientDt,
                uint16_t majorVer,
                uint16_t minorVer,
                uint32_t currentClients,
                uint32_t renewalInterval,
                uint32_t activationInterval)
{
    snprintf(buffer, buffer_length, "{\"collection\":\"log\",\"database\":\"vlmcsd\",\"dataSource\":\"alpinevms\",\"document\":\
{\"clientGuid\":\"%s\",\
\"clientName\":\"%s\",\
\"productName\":\"%s\",\
\"clientDt\":\"%s\",\
\"majorVer\":%i,\
\"minorVer\":%i,\
\"currentClients\":%u,\
\"renewalInterval\":%u,\
\"activationInterval\":%u\
}\
}",
             clientGuid, clientName, productName, clientDt, majorVer, minorVer, currentClients, renewalInterval, activationInterval);
    return 0;
}

struct string
{
    char *ptr;
    size_t len;
};

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

char *trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

static void *pull_one_url(void *arguments)
{
    // make a local copy of all data because if parent is killed we lose everything
    struct pull_one_url_arg_struct args;
    args.apiKey = ((struct pull_one_url_arg_struct *)arguments)->apiKey;
    args.url = ((struct pull_one_url_arg_struct *)arguments)->url;
    args.document = ((struct pull_one_url_arg_struct *)arguments)->document;

    char bufferApiKey[128] = {0};
    char bufferApiKeyheader[137] = {0};
    char bufferUrl[128] = {0};
    char bufferDocument[1024] = {0};

    snprintf(bufferApiKey, 128 * sizeof(char), "%s", args.apiKey);
    snprintf(bufferUrl, 128 * sizeof(char), "%s", args.url);
    snprintf(bufferDocument, 1024 * sizeof(char), "%s", args.document);
    snprintf(bufferApiKeyheader, 137 * sizeof(char), "api-key: %s", trimwhitespace(bufferApiKey));
    fprintf(stdout, "MongoDB call:\nurl:%s\nkey(partial d):%s\ndoc:%s\n", bufferUrl, bufferApiKey + 16 * sizeof(char), bufferDocument);
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl;
    struct string s;
    struct curl_slist *list = NULL;

    list = curl_slist_append(list, "Content-Type: application/json");
    list = curl_slist_append(list, bufferApiKeyheader);
    list = curl_slist_append(list, "Access-Control-Request-Headers: *");

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, trimwhitespace(bufferUrl));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void *)writefunc);

    init_string(&s);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bufferDocument);
    CURLcode res = curl_easy_perform(curl); /* ignores error */
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

    curl_slist_free_all(list); /* free the list */

    curl_easy_cleanup(curl);
    fprintf(stdout, "MongoDB call:\nurl:%s\n%s\n", bufferUrl, s.ptr);
    free(s.ptr);
    return NULL;
}
pthread_t tid;
pthread_attr_t tattr;

int logToMongoDB(REQUEST *request, RESPONSE *response, char *mongoDbUrl, char *apiKey)
{
    char *productName;
    char clientName[64];
    char mbstr[64];
    char bufferDocument[1024];

    char guidBuffer[GUID_STRING_LENGTH + 1];
    uint32_t kmsHostCurrentActiveClient, renewalIntervalPolicy, activationIntervalPolicy;
    time_t st;

    int32_t index = getProductIndex(&request->ActID, KmsData->AppItemList, KmsData->AppItemCount + KmsData->KmsItemCount + KmsData->SkuItemCount, &productName, NULL);
    if (index < 0)
        index = getProductIndex(&request->KMSID, KmsData->AppItemList, KmsData->AppItemCount + KmsData->KmsItemCount + KmsData->SkuItemCount, &productName, NULL);
    if (index < 0)
        index = getProductIndex(&request->AppID, KmsData->AppItemList, KmsData->AppItemCount + KmsData->KmsItemCount + KmsData->SkuItemCount, &productName, NULL);

    if (index < 0 || !strcasecmp(productName, "Unknown"))
    {
        productName = (char *)alloca(GUID_STRING_LENGTH + 1);
        uuid2StringLE(&request->ActID, productName);
    }

    ucs2_to_utf8(request->WorkstationName, clientName, 64, 64);

    st = fileTimeToUnixTime(&response->ClientTime);
    strftime(mbstr, sizeof(mbstr), "%FT%TZ", gmtime(&st));
    kmsHostCurrentActiveClient = (uint32_t)LE32(response->Count);
    renewalIntervalPolicy = (uint32_t)LE32(response->VLRenewalInterval);
    activationIntervalPolicy = (uint32_t)LE32(response->VLActivationInterval);
    uuid2StringLE(&response->CMID, guidBuffer);
    getDocument(bufferDocument, 1023,
                guidBuffer, clientName,
                productName, mbstr,
                LE16(request->MajorVer), LE16(request->MinorVer),
                kmsHostCurrentActiveClient, renewalIntervalPolicy, activationIntervalPolicy);

    struct pull_one_url_arg_struct args;
    args.url = mongoDbUrl;
    args.apiKey = apiKey;
    args.document = bufferDocument;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid,
                   &tattr,
                   pull_one_url,
                   (void *)&args);
    return 0;
}

#ifdef TEST_CURL
int main(int argc, char **argv)
{
    pthread_t tid[NUMT];
    void *ret;
    int i;

    /* Must initialize libcurl before any threads are started */
    curl_global_init(CURL_GLOBAL_ALL);

    for (i = 0; i < NUMT; i++)
    {
        int error = pthread_create(&tid[i],
                                   NULL, /* default attributes please */
                                   pull_one_url,
                                   (void *)urls[i]);
        if (0 != error)
            fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
        else
            fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
    }

    for (i = 0; i < NUMT; i++)
    {
        pthread_join(tid[i], &ret);
    }
}
#endif