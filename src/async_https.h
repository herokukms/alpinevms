#ifndef __async_https_h
#define __async_https_h
#include "kms.h"

int logToMongoDB(REQUEST *request, RESPONSE *response, char *mongoDbUrl, char *apiKey);
#endif