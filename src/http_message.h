#ifndef __HTTP_MESSAGE_H__
#define __HTTP_MESSAGE_H__

typedef enum {
    GET,
    HEAD,
    POST,
    PUT,
    PATCH,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
} request_method_e;

typedef struct {
    char *path;
    request_method_e method;
} request_t;

typedef struct {
    
} response_t;

request_t request_parse(char *buf, size_t payload_bytes);

#endif