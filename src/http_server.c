/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2012,2013,2014 Adap.tv, Inc.

    RIBS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, version 2.1 of the License.

    RIBS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with RIBS.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "http_server.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/sendfile.h>
#include "mime_types.h"
#include "logger.h"
#define HTTP_DEF_STR(var,str)                   \
    const char var[]=str
#include "http_defs.h"

#ifdef RIBS2_SSL
#include "ribs_ssl.h"
#endif

#define ACCEPTOR_STACK_SIZE 8192
#define MIN_HTTP_REQ_SIZE 5 // method(3) + space(1) + URI(1) + optional VER...
#define DEFAULT_MAX_REQ_SIZE 1024*1024*1024
#define DEFAULT_NUM_STACKS 64

/* methods */
SSTRL(HEAD, "HEAD " );
SSTRL(GET,  "GET "  );
SSTRL(POST, "POST " );
SSTRL(PUT,  "PUT "  );
/* misc */
SSTRL(HTTP_SERVER_VER, "HTTP/1.1");
SSTRL(HTTP_SERVER_NAME, "ribs2.0");
SSTRL(CRLFCRLF, "\r\n\r\n");
SSTRL(CRLF, "\r\n");
SSTRL(CONNECTION, "\r\nConnection: ");
SSTRL(CONNECTION_CLOSE, "close");
SSTRL(CONNECTION_KEEPALIVE, "Keep-Alive");
SSTRL(CONTENT_LENGTH, "\r\nContent-Length: ");
SSTRL(SET_COOKIE, "\r\nSet-Cookie: ");
SSTRL(COOKIE_VERSION, "Version=\"1\"");
SSTRL(HTTP_LOCATION, "\r\nLocation: ");
SSTR(CACHE_CONTROL, "\r\nCache-Control: ");
SSTR(PRAGMA, "\r\nPragma: ");
SSTR(EXPIRES, "\r\nExpires: ");
/* 1xx */
SSTRL(HTTP_STATUS_100, "100 Continue");
SSTRL(EXPECT_100, "\r\nExpect: 100");

static int accept_reserved_fd = -1;
static inline void http_server_yield(void);

static int _http_server_read(struct http_server_context *ctx) {
    ssize_t res;
    ssize_t wavail;
    while (0 < (res = read(ctx->fd, vmbuf_wloc(&ctx->request), wavail = vmbuf_wavail(&ctx->request)))) {
        if (0 > vmbuf_wseek(&ctx->request, res))
            return -1;
        if (res < wavail)
            return errno=0, 1;
    }
    if (res < 0)
        return (EAGAIN == errno ? 1 : -1);
    return 0; // remote side closed connection
}

static int _http_server_write(struct http_server_context *ctx) {
    struct iovec iovec[2] = {
        { vmbuf_data(&ctx->header), vmbuf_wlocpos(&ctx->header)},
        { vmbuf_data(&ctx->payload), vmbuf_wlocpos(&ctx->payload)}
    };
    ssize_t num_write;
    for (;;http_server_yield()) {
        num_write = writev(ctx->fd, iovec, iovec[1].iov_len ? 2 : 1);
        if (0 > num_write) {
            if (EAGAIN == errno) {
                continue;
            } else {
                ctx->persistent = 0;
                return -1;
            }
        } else {
            if (num_write >= (ssize_t)iovec[0].iov_len) {
                num_write -= iovec[0].iov_len;
                iovec[0].iov_len = iovec[1].iov_len - num_write;
                if (iovec[0].iov_len == 0)
                    break;
                iovec[0].iov_base = iovec[1].iov_base + num_write;
                iovec[1].iov_len = 0;
            } else {
                iovec[0].iov_len -= num_write;
                iovec[0].iov_base += num_write;
            }
        }
    }
    return 0;
}

static int _http_server_sendfile(struct http_server_context *ctx, int ffd, ssize_t size) {
    off_t ofs = 0;
    int fd = ctx->fd;
    for (;;http_server_yield()) {
        if (0 > sendfile(fd, ffd, &ofs, size - ofs) && EAGAIN != errno)
            return ctx->persistent = 0, -1;
        if (ofs >= size) break;
    }
    return 0;
}

#ifdef RIBS2_SSL
static int _http_server_read_ssl(struct http_server_context *ctx) {
    ssize_t res;
    ssize_t wavail;
    SSL *ssl = ribs_ssl_get(ctx->fd);
    while (0 < (res = SSL_read(ssl, vmbuf_wloc(&ctx->request), wavail = vmbuf_wavail(&ctx->request)))) {
        if (0 > vmbuf_wseek(&ctx->request, res))
            return -1;
        if (res < wavail)
            return errno=0, 1;
    }
    if (res < 0)
        return ribs_ssl_want_io(ssl, res) ? 1 : -1;
    return 0; // remote side closed connection
}

static int _http_server_write_ssl(struct http_server_context *ctx) {
    SSL *ssl = ribs_ssl_get(ctx->fd);
    int _write(struct vmbuf *vmb) {
        int res;
        size_t rav;
        while ((rav = vmbuf_ravail(vmb)) > 0) {
            res = SSL_write(ssl, vmbuf_rloc(vmb), rav);
            if (res > 0) {
                vmbuf_rseek(vmb, res);
                continue;
            }
            if (ribs_ssl_want_io(ssl, res)) {
                http_server_yield();
                continue;
            }
            ctx->persistent = 0;
            return errno = ENODATA, -1; // error, can not be zero
        }
        return 1; // reached the end
    }

    if ( 0 > _write(&ctx->header) ||
         0 > _write(&ctx->payload))
        return -1;
    return 0;
}

static int _http_server_sendfile_ssl(struct http_server_context *ctx, int ffd, ssize_t size) {
    off_t ofs = 0;
    SSL *ssl = ribs_ssl_get(ctx->fd);
    do {
        ssize_t chunk_size = 1024*1024;
        off_t chunk_ofs = 0;
        if (ofs + chunk_size > size)
            chunk_size = size - ofs;
        void *mem = mmap(NULL, 1024*1024, PROT_READ, MAP_SHARED, ffd, ofs);
        for (;;http_server_yield()) {
            int res = SSL_write(ssl, mem + chunk_ofs, chunk_size - chunk_ofs);
            if (res > 0) {
                chunk_ofs += res;
                if (chunk_ofs == chunk_size)
                    break;
            } else {
                if (res < 0) {
                    if (ribs_ssl_want_io(ssl, res))
                                       continue;
                }
                ctx->persistent = 0;
                munmap(mem, 1024*1024);
                return errno = ENODATA, -1;
            }
        }
        ofs += chunk_size;
        munmap(mem, 1024*1024);
    } while (ofs < size);
    return 0;
}
#endif

static void http_server_process_request(char *uri, char *headers);
static void http_server_accept_connections(void);

static void http_server_fiber_main_wrapper(void) {
    http_server_fiber_main();
    struct http_server_context *ctx = http_server_get_context();
    ctx_pool_put(&ctx->server->ctx_pool, current_ctx);
}

static void http_server_idle_handler(void) {
    struct http_server **server_ref = (struct http_server **)current_ctx->reserved;
    struct http_server *server = *server_ref;
    for (;;) {
        if (last_epollev.events == EPOLLOUT)
            yield();
        else {
            struct ribs_context *new_ctx = ctx_pool_get(&server->ctx_pool);
            ribs_makecontext(new_ctx, event_loop_ctx, http_server_fiber_main_wrapper);
            int fd = last_epollev.data.fd;
            struct epoll_worker_fd_data *fd_data = epoll_worker_fd_map + fd;
            fd_data->ctx = new_ctx;
            struct http_server_context *ctx = (struct http_server_context *)new_ctx->reserved;
            ctx->fd = fd;
            ctx->server = server;
            TIMEOUT_HANDLER_REMOVE_FD_DATA(fd_data);
            ribs_swapcurcontext(new_ctx);
        }
    }
}

int http_server_init(struct http_server *server) {
    server->bind_addr = htonl(INADDR_ANY);
#ifdef RIBS2_SSL
    server->use_ssl = 0;
#endif
    return http_server_init2(server);
}

#ifdef RIBS2_SSL
int http_server_init_ssl(struct http_server *server) {
    server->bind_addr = htonl(INADDR_ANY);
    server->use_ssl = 1;
    return http_server_init2(server);
}
#endif

int http_server_init2(struct http_server *server) {
    /*
     * one time global initializers
     */
    if (0 > mime_types_init())
        return LOGGER_ERROR("failed to initialize mime types"), -1;
    if (0 > http_headers_init())
        return LOGGER_ERROR("failed to initialize http headers"), -1;

    if (-1 == accept_reserved_fd) {
        accept_reserved_fd = open("/dev/null", 0);
        if (0 > accept_reserved_fd)
            return LOGGER_PERROR("open"), -1;
    }

#ifdef RIBS2_SSL
    if (server->use_ssl) {

        server->http_server_read = _http_server_read_ssl;
        server->http_server_write = _http_server_write_ssl;
        server->http_server_sendfile = _http_server_sendfile_ssl;

        /* init */
        SSL_library_init();
        server->ssl_ctx = SSL_CTX_new(SSLv23_server_method());

        if (0 != ribs_ssl_set_options(server->ssl_ctx, server->cipher_list, server->dhparam_file))
            return -1;

        /* Chain file must start with the server's certificate, appended by the authority certs */
        if (0 == SSL_CTX_use_certificate_chain_file(server->ssl_ctx, server->certificate_chain_file))
            return LOGGER_ERROR("failed to initialize SSL:chain_file"), -1;
        if (0 == SSL_CTX_use_PrivateKey_file(server->ssl_ctx, server->privatekey_file, SSL_FILETYPE_PEM))
            return LOGGER_ERROR("failed to initialize SSL:load privatekey_file"), -1;
        if (0 == SSL_CTX_check_private_key(server->ssl_ctx))
            return LOGGER_ERROR("failed to initialize SSL:check private_key"), -1;
    } else
#endif
    {
        server->http_server_read = _http_server_read;
        server->http_server_write = _http_server_write;
        server->http_server_sendfile = _http_server_sendfile;
    }
    /*
     * idle connection handler
     */
    server->idle_ctx = ribs_context_create(SMALL_STACK_SIZE, sizeof(struct http_server *), http_server_idle_handler);
    struct http_server **server_ref = (struct http_server **)server->idle_ctx->reserved;
    *server_ref = server;
    /*
     * context pool
     */
    if (0 == server->num_stacks)
        server->num_stacks = DEFAULT_NUM_STACKS;
    struct rlimit rlim;
    if (0 > getrlimit(RLIMIT_STACK, &rlim))
        return LOGGER_PERROR("getrlimit(RLIMIT_STACK)"), -1;
    server->stack_size = server->stack_size > rlim.rlim_cur ? server->stack_size : rlim.rlim_cur;
    LOGGER_INFO("http server pool: initial=%zu, grow=%zu, stack_size=%zu", server->num_stacks, server->num_stacks, server->stack_size);
    ctx_pool_init(&server->ctx_pool, server->num_stacks, server->num_stacks, server->stack_size, sizeof(struct http_server_context) + server->context_size);
    /*
     * listen socket
     */
    const int LISTEN_BACKLOG = 32768;
    int lfd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (0 > lfd)
        return -1;

    int rc;
    const int option = 1;
    rc = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (0 > rc)
        return LOGGER_PERROR("setsockopt, SO_REUSEADDR"), rc;

    rc = setsockopt(lfd, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(option));
    if (0 > rc)
        return LOGGER_PERROR("setsockopt, TCP_NODELAY"), rc;

    struct linger ls;
    ls.l_onoff = 0;
    ls.l_linger = 0;
    rc = setsockopt(lfd, SOL_SOCKET, SO_LINGER, (void *)&ls, sizeof(ls));
    if (0 > rc)
        return LOGGER_PERROR("setsockopt, SO_LINGER"), rc;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);
    addr.sin_addr.s_addr = server->bind_addr;
    if (0 > bind(lfd, (struct sockaddr *)&addr, sizeof(addr)))
        return close(lfd), LOGGER_PERROR("bind"), -1;

    if (0 == server->port) {
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        if (0 > getsockname(lfd, (struct sockaddr *)&addr, &addrlen))
            return close(lfd), LOGGER_PERROR("getsockname"), -1;
        server->port = ntohs(addr.sin_port);
    }
    LOGGER_INFO("listening on port: %d, backlog: %d, protocol: http%s", server->port, LISTEN_BACKLOG,
#ifdef RIBS2_SSL
                server->use_ssl ? "s" :
#endif
                "");

    if (0 > listen(lfd, LISTEN_BACKLOG))
        return LOGGER_PERROR("listen"), -1;

    server->accept_ctx = ribs_context_create(ACCEPTOR_STACK_SIZE, sizeof(struct http_server *), http_server_accept_connections);
    server->fd = lfd;
    server_ref = (struct http_server **)server->accept_ctx->reserved;
    *server_ref = server;

    if (server->max_req_size == 0)
        server->max_req_size = DEFAULT_MAX_REQ_SIZE;
    return 0;
}

int http_server_init_acceptor(struct http_server *server) {
    if (0 > ribs_epoll_add(server->fd, EPOLLIN, server->accept_ctx))
        return -1;
    return timeout_handler_init(&server->timeout_handler);
}

static void http_server_accept_connections(void) {
    struct http_server **server_ref = (struct http_server **)current_ctx->reserved;
    struct http_server *server = *server_ref;
    for (;; yield()) {
        struct sockaddr_in new_addr;
        socklen_t new_addr_size = sizeof(struct sockaddr_in);
        int fd = accept4(server->fd, (struct sockaddr *)&new_addr, &new_addr_size, SOCK_CLOEXEC | SOCK_NONBLOCK);
        if (0 > fd) {
            if (EAGAIN == errno)
                continue;
            if (EMFILE == errno || ENFILE == errno) {
                /*
                 * If we run out of fds then we must be swamped with requests. So any new connections won't be serviced
                 * in a timely manner. By the time we service them, they'll probably time out anyways. So instead of making
                 * them wait to find out that we're overloaded, let them know immediately.
                 */
                LOGGER_PERROR("Not accepting connection on %s:%hu", inet_ntoa((struct in_addr){server->bind_addr}), server->port);
                close(accept_reserved_fd);
                fd = accept4(server->fd, (struct sockaddr *)&new_addr, &new_addr_size, SOCK_CLOEXEC | SOCK_NONBLOCK);
                if (0 > fd) {
                    if (EAGAIN != errno)
                        LOGGER_PERROR("Accept on %s:%hu", inet_ntoa((struct in_addr){server->bind_addr}), server->port);
                }
                else
                    close(fd);
                accept_reserved_fd = open("/dev/null", 0);
                if (0 > accept_reserved_fd)
                    LOGGER_PERROR("open");
            } else {
                LOGGER_PERROR("Accept on %s:%hu", inet_ntoa((struct in_addr){server->bind_addr}), server->port);
            }
            continue;
        }
        if (0 > ribs_epoll_add(fd, EPOLLIN | EPOLLOUT | EPOLLET, server->idle_ctx)) {
            ribs_close(fd);
            continue;
        }
        timeout_handler_add_fd_data(&server->timeout_handler, epoll_worker_fd_map + fd);
    }
}

static int check_persistent(char *p) {
    char *conn = strstr(p, CONNECTION);
    char *h1_1 = strstr(p, " HTTP/1.1");
    // HTTP/1.1
    if ((NULL != h1_1 &&
         (NULL == conn ||
          0 != SSTRNCMPI(CONNECTION_CLOSE, conn + SSTRLEN(CONNECTION)))) ||
        // HTTP/1.0
        (NULL == h1_1 &&
         NULL != conn &&
         0 == SSTRNCMPI(CONNECTION_KEEPALIVE, conn + SSTRLEN(CONNECTION))))
        return 1;
    else
        return 0;
}


void http_server_header_start(const char *status, const char *content_type) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_sprintf(&ctx->header, "%s %s\r\nServer: %s\r\nContent-Type: %s%s%s", HTTP_SERVER_VER, status, HTTP_SERVER_NAME, content_type, CONNECTION, ctx->persistent ? CONNECTION_KEEPALIVE : CONNECTION_CLOSE);
}

void http_server_header_start_no_body(const char *status) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_sprintf(&ctx->header, "%s %s\r\nServer: %s%s%s", HTTP_SERVER_VER, status, HTTP_SERVER_NAME, CONNECTION, ctx->persistent ? CONNECTION_KEEPALIVE : CONNECTION_CLOSE);
}

void http_server_header_close(void) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_strcpy(&ctx->header, CRLFCRLF);
}

void http_server_header_no_cache(){
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_sprintf(&ctx->header, "%s%s", CACHE_CONTROL, "no-cache, no-store, must-revalidate");
    vmbuf_sprintf(&ctx->header, "%s%s", PRAGMA, "no-cache");
    vmbuf_sprintf(&ctx->header, "%s%s", EXPIRES, "0");
}


void http_server_set_cookie(const char *name, const char *value, uint32_t max_age, const char *path, const char *domain) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_sprintf(&ctx->header, "%s%s=\"%s\"", SET_COOKIE, name, value);
    if (path) vmbuf_sprintf(&ctx->header, ";Path=%s", path);
    if (max_age) vmbuf_sprintf(&ctx->header, ";Max-Age=%u", max_age);
    if (domain) vmbuf_sprintf(&ctx->header, ";Domain=%s", domain);
    vmbuf_sprintf(&ctx->header, ";%s", COOKIE_VERSION);
}

void http_server_set_session_cookie(const char *name, const char *value, const char *path) {
    http_server_set_cookie(name, value, 0, path, NULL);
}

struct vmbuf *http_server_begin_cookie(const char *name) {
    struct vmbuf *buf = &http_server_get_context()->header;
    vmbuf_sprintf(buf, "\r\nSet-Cookie: %s=\"", name);
    return buf;
}

struct vmbuf *http_server_end_cookie(time_t expires, const char *domain, const char *path) {
    struct vmbuf *buf = &http_server_get_context()->header;
    struct tm tm;
    gmtime_r(&expires, &tm);
    vmbuf_sprintf(buf, "\";Path=%s;Domain=%s;Expires=", path, domain);
    vmbuf_strftime(buf, "%a, %d-%b-%Y %H:%M:%S %Z", &tm);
    return buf;
}

void http_server_response(const char *status, const char *content_type) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_reset(&ctx->header);
    http_server_header_start(status, content_type);
    http_server_header_content_length();
    http_server_header_close();
}

void http_server_response_sprintf(const char *status, const char *content_type, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    http_server_response_vsprintf(status, content_type, format, ap);
    va_end(ap);
}

void http_server_response_vsprintf(const char *status, const char *content_type, const char *format, va_list ap) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_reset(&ctx->header);
    vmbuf_reset(&ctx->payload);
    http_server_header_start(status, content_type);
    vmbuf_vsprintf(&ctx->payload, format, ap);
    http_server_header_content_length();
    http_server_header_close();
}

void http_server_header_redirect(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    http_server_header_vredirect(format, ap);
    va_end(ap);
}

void http_server_header_vredirect(const char *format, va_list ap) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_strcpy(&ctx->header, HTTP_LOCATION);
    vmbuf_vsprintf(&ctx->header, format, ap);
}

void http_server_redirect(const char *status, const char *content_type, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    http_server_vredirect(status, content_type, format, ap);
    va_end(ap);
}

void http_server_vredirect(const char *status, const char *content_type, const char *format, va_list ap) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_reset(&ctx->header);
    http_server_header_start(status, content_type);
    http_server_header_vredirect(format, ap);
    http_server_header_content_length();
    http_server_header_close();
}

void http_server_header_content_length(void) {
    struct http_server_context *ctx = http_server_get_context();
    vmbuf_sprintf(&ctx->header, "%s%zu", CONTENT_LENGTH, vmbuf_wlocpos(&ctx->payload));
}

#define READ_FROM_SOCKET()                                              \
    res = server->http_server_read(ctx);                                \
    if (0 >= res) {                                                     \
        ribs_close(fd); /* remote side closed or other error occured */ \
        return;                                                         \
    }                                                                   \
    if (vmbuf_wlocpos(&ctx->request) > max_req_size) {                  \
        ctx->persistent = 0;                                            \
        http_server_response(HTTP_STATUS_413, HTTP_CONTENT_TYPE_TEXT_PLAIN); \
        server->http_server_write(ctx);                                 \
        ribs_close(fd);                                                 \
        return;                                                         \
    }


static inline void http_server_yield(void) {
    struct http_server_context *ctx = http_server_get_context();
    struct epoll_worker_fd_data *fd_data = epoll_worker_fd_map + ctx->fd;
    timeout_handler_add_fd_data(&ctx->server->timeout_handler, fd_data);
    yield();
    TIMEOUT_HANDLER_REMOVE_FD_DATA(fd_data);
}

void http_server_fiber_main(void) {
    struct http_server_context *ctx = http_server_get_context();
    struct http_server *server = ctx->server;
    int fd = ctx->fd;

    char *URI;
    char *headers;
    char *content;
    size_t content_length;
    int res;
    ctx->persistent = 0;

    vmbuf_init(&ctx->request, server->init_request_size);
    vmbuf_init(&ctx->header, server->init_header_size);
    vmbuf_init(&ctx->payload, server->init_payload_size);
    size_t max_req_size = server->max_req_size;

#ifdef RIBS2_SSL
    if (server->use_ssl && NULL == ribs_ssl_get(fd)) {
        SSL *ssl = ribs_ssl_alloc(fd, server->ssl_ctx);
        if (NULL == ssl) {
            ribs_close(fd);
            return;
        }
        for (;;http_server_yield()) {
            int ret = SSL_accept(ssl);
            if (1 == ret) break;
            if (0 == ret) {
                ribs_close(fd);
                return;
            }
            if (!ribs_ssl_want_io(ssl, ret)) {
                ribs_close(fd);
                return;
            }
        }
    }
#endif

    for (;; http_server_yield()) {
        READ_FROM_SOCKET();
        if (vmbuf_wlocpos(&ctx->request) > MIN_HTTP_REQ_SIZE)
            break;
    }
    do {
        if (0 == SSTRNCMP(GET, vmbuf_data(&ctx->request)) || 0 == SSTRNCMP(HEAD, vmbuf_data(&ctx->request))) {
            /* GET or HEAD */
            while (0 != SSTRNCMP(CRLFCRLF,  vmbuf_wloc(&ctx->request) - SSTRLEN(CRLFCRLF))) {
                http_server_yield();
                READ_FROM_SOCKET();
            }
            /* make sure the string is \0 terminated */
            /* this will overwrite the first CR */
            *(vmbuf_wloc(&ctx->request) - SSTRLEN(CRLFCRLF)) = 0;
            char *p = vmbuf_data(&ctx->request);
            ctx->persistent = check_persistent(p);
            URI = strchrnul(p, ' '); /* can't be NULL GET and HEAD constants have space at the end */
            *URI = 0;
            ++URI; // skip the space
            p = strchrnul(URI, '\r'); /* HTTP/1.0 */
            headers = p;
            if (0 != *headers) /* are headers present? */
                headers += SSTRLEN(CRLF); /* skip the new line */
            *p = 0;
            p = strchrnul(URI, ' '); /* truncate the version part */
            *p = 0; /* \0 at the end of URI */

            ctx->content = NULL;
            ctx->content_len = 0;

            /* minimal parsing and call user function */
            http_server_process_request(URI, headers);
        } else if (0 == SSTRNCMP(POST, vmbuf_data(&ctx->request)) || 0 == SSTRNCMP(PUT, vmbuf_data(&ctx->request))) {
            /* POST or PUT */
            for (;;) {
                *vmbuf_wloc(&ctx->request) = 0;
                /* wait until we have the header */
                if (NULL != (content = strstr(vmbuf_data(&ctx->request), CRLFCRLF)))
                    break;
                http_server_yield();
                READ_FROM_SOCKET();
            }
            *content = 0; /* terminate at the first CR like in GET */
            content += SSTRLEN(CRLFCRLF);
            size_t content_ofs = content - vmbuf_data(&ctx->request);

            if (strstr(vmbuf_data(&ctx->request), EXPECT_100)) {
                vmbuf_sprintf(&ctx->header, "%s %s\r\n\r\n", HTTP_SERVER_VER, HTTP_STATUS_100);
                if (0 > server->http_server_write(ctx)) {
                    ribs_close(fd);
                    return;
                }
                vmbuf_reset(&ctx->header);
            }
            ctx->persistent = check_persistent(vmbuf_data(&ctx->request));

            /* parse the content length */
            char *p = strcasestr(vmbuf_data(&ctx->request), CONTENT_LENGTH);
            if (NULL == p) {
                http_server_response(HTTP_STATUS_411, HTTP_CONTENT_TYPE_TEXT_PLAIN);
                break;
            }

            p += SSTRLEN(CONTENT_LENGTH);
            content_length = atoi(p);
            for (;;) {
                if (content_ofs + content_length <= vmbuf_wlocpos(&ctx->request))
                    break;
                http_server_yield();
                READ_FROM_SOCKET();
            }
            p = vmbuf_data(&ctx->request);
            URI = strchrnul(p, ' '); /* can't be NULL PUT and POST constants have space at the end */
            *URI = 0;
            ++URI; /* skip the space */
            p = strchrnul(URI, '\r'); /* HTTP/1.0 */
            headers = p;
            if (0 != *headers) /* are headers present? */
                headers += SSTRLEN(CRLF); /* skip the new line */
            *p = 0;
            p = strchrnul(URI, ' '); /* truncate http version */
            *p = 0; /* \0 at the end of URI */
            ctx->content = vmbuf_data_ofs(&ctx->request, content_ofs);
            *(ctx->content + content_length) = 0;
            ctx->content_len = content_length;

            /* minimal parsing and call user function */
            http_server_process_request(URI, headers);
        } else {
            http_server_response(HTTP_STATUS_501, HTTP_CONTENT_TYPE_TEXT_PLAIN);
            break;
        }
    } while(0);

    if (vmbuf_wlocpos(&ctx->header) > 0) {
        epoll_worker_resume_events(fd);
        server->http_server_write(ctx);
    }

    if (ctx->persistent) {
        struct epoll_worker_fd_data *fd_data = epoll_worker_fd_map + fd;
        fd_data->ctx = server->idle_ctx;
        timeout_handler_add_fd_data(&server->timeout_handler, fd_data);
    } else
        ribs_close(fd);
}

static void http_server_process_request(char *uri, char *headers) {
    struct http_server_context *ctx = http_server_get_context();
    ctx->headers = headers;
    char *query = strchrnul(uri, '?');
    if (*query)
        *query++ = 0;
    ctx->query = query;
    static const char HTTP[] = "http://";
    if (0 == SSTRNCMP(HTTP, uri)) {
        uri += SSTRLEN(HTTP);
        uri = strchrnul(uri, '/');
    }
    ctx->uri = uri;
    epoll_worker_ignore_events(ctx->fd);
    ctx->server->user_func();
}

int http_server_sendfile(const char *filename) {
    return http_server_sendfile2(filename, NULL, NULL);
}

int http_server_sendfile2(const char *filename, const char *additional_headers, const char *ext) {
    if (0 == *filename)
        filename = ".";
    struct http_server_context *ctx = http_server_get_context();
    int ffd = open(filename, O_RDONLY);
    if (ffd < 0)
        return HTTP_SERVER_NOT_FOUND;
    struct stat st;
    if (0 > fstat(ffd, &st)) {
        LOGGER_PERROR("%s", filename);
        close(ffd);
        return HTTP_SERVER_NOT_FOUND;
    }
    if (S_ISDIR(st.st_mode)) {
        close(ffd);
        return 1;
    }

    vmbuf_reset(&ctx->header);

    if (NULL != ext)
        http_server_header_start(HTTP_STATUS_200, mime_types_by_ext(ext));
    else
        http_server_header_start(HTTP_STATUS_200, mime_types_by_filename(filename));
    vmbuf_sprintf(&ctx->header, "%s%jd", CONTENT_LENGTH, (intmax_t)st.st_size);
    if (additional_headers)
        vmbuf_strcpy(&ctx->header, additional_headers);

    http_server_header_close();
    int res = http_server_sendfile_payload(ffd, st.st_size);
    close(ffd);
    if (0 > res)
        LOGGER_PERROR("%s", filename);
    return res;
}

int http_server_sendfile_payload(int ffd, off_t size) {
    struct http_server_context *ctx = http_server_get_context();
    int fd = ctx->fd;
    int option = 1;
    if (0 > setsockopt(fd, IPPROTO_TCP, TCP_CORK, &option, sizeof(option)))
        LOGGER_PERROR("TCP_CORK set");
    epoll_worker_resume_events(ctx->fd);
    ctx->server->http_server_write(ctx);
    vmbuf_reset(&ctx->header);
    int res = ctx->server->http_server_sendfile(ctx, ffd, size);
    if (0 == res) {
        option = 0;
        if (0 > setsockopt(fd, IPPROTO_TCP, TCP_CORK, &option, sizeof(option)))
            LOGGER_PERROR("TCP_CORK release");
    }
    return res;
}

int http_server_generate_dir_list(const char *URI) {
    struct http_server_context *ctx = http_server_get_context();
    struct vmbuf *payload = &ctx->payload;
    const char *dir = URI;
    if (*dir == '/') ++dir;
    if (0 == *dir)
        dir = ".";
    vmbuf_sprintf(payload, "<html><head><title>Index of %s</title></head>", dir);
    vmbuf_strcpy(payload, "<body>");
    vmbuf_sprintf(payload, "<h1>Index of %s</h1><hr>", dir);

    vmbuf_sprintf(payload, "<a href=\"..\">../</a><br><br>");
    vmbuf_sprintf(payload, "<table width=\"100%%\" border=\"0\">");
    DIR *d = opendir(dir);
    int error = 0;
    if (d) {
        struct dirent *de = NULL;
        while ((de = readdir(d))) {
            if (de->d_name[0] == '.')
                continue;
            struct stat st;
            if (0 > fstatat(dirfd(d), de->d_name, &st, 0)) {
                vmbuf_sprintf(payload, "<tr><td>ERROR: %s</td><td>N/A</td></tr>", de->d_name);
                continue;
            }
            const char *slash = (S_ISDIR(st.st_mode) ? "/" : "");
            struct tm t_res, *t;
            t = localtime_r(&st.st_mtime, &t_res);

            vmbuf_strcpy(payload, "<tr>");
            vmbuf_sprintf(payload, "<td><a href=\"%s%s\">%s%s</a></td>", de->d_name, slash, de->d_name, slash);
            vmbuf_strcpy(payload, "<td>");
            if (t)
                vmbuf_strftime(payload, "%F %T", t);
            vmbuf_strcpy(payload, "</td>");
            vmbuf_sprintf(payload, "<td>%jd</td>", (intmax_t)st.st_size);
            vmbuf_strcpy(payload, "</tr>");
        }
        closedir(d);
    }
    vmbuf_strcpy(payload, "<tr><td colspan=3><hr></td></tr></table>");
    vmbuf_sprintf(payload, "<address>RIBS 2.0 Port %hu</address></body>", ctx->server->port);
    vmbuf_strcpy(payload, "</html>");
    return error;
}


void http_server_close(struct http_server *server) {
    ribs_close(server->fd);
}
