/* call pipe and set flags on file descriptors.
 */
_RIBS_INLINE_ int pipe2(int fildes[2], int flags) {
  int i;
  if (0 > pipe(fildes))
    return -1;
  for (i = 0; i < 2 && fcntl(fildes[i], F_SETFL, (fcntl(fildes[i],F_GETFL)) | flags) >= 0; ++i);
  if (i < 2)
    return -1;
  return 0;
}

_RIBS_INLINE_ char *strchrnul(const char *s, int c) {
  for (; *s != c && *s; ++s);
  return (char *)s;
}

_RIBS_INLINE_ int accept4(int socket, struct sockaddr *addr_buf, socklen_t *addr_len, int flags) {
  int sfd = accept(socket, addr_buf, addr_len);
  if (flags & SOCK_CLOEXEC) {
    fcntl(sfd, F_SETFD, fcntl(sfd, F_GETFD) | FD_CLOEXEC);
  }
  if (flags & SOCK_NONBLOCK) {
    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
  }
  return sfd;
}

_RIBS_INLINE_ int socket_apple(int socket_family, int socket_type, int protocol) {
  int sfd = socket(socket_family, socket_type & ~SOCK_NONBLOCK, protocol);
  if (socket_type & SOCK_NONBLOCK) {
    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
  }
  return sfd;
}
