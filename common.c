#include "common.h"
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <assert.h>

int read_len(int fd, void *buf, size_t length)
{
  size_t total = 0;
  while (total < length) {
    ssize_t read_sz = read(fd, buf + total, length - total);
    if (read_sz == -1) {
        if (errno == EINTR) {
            // if we get interrupted then we should try reading again
            continue;
        }
      // read error
      perror("read");
      return -1;
    } else if (read_sz == 0) {
      // eof
      return 0;
    }

    total += read_sz;
  }
#ifdef DEBUG_ON
  for (int i = 0; i < length; ++i) {
    fprintf(stderr, "%02X ", ((char *) buf)[i]);
  }
  fprintf(stderr, "\n");
#endif
  return total;
}

int write_len(const int fd, const void *buf, size_t length)
{
  size_t total = 0;
  while (total < length) {
    ssize_t write_sz = write(fd, buf + total, length - total);
    if (write_sz == -1) {
        if (errno == EINTR) {
            // if we get interrupted then we should try reading again
            continue;
        }
      // read error
      perror("write");
      return -1;
    }

    total += write_sz;
  }

#ifdef DEBUG_ON
  for (int i = 0; i < length; ++i) {
    fprintf(stderr, "%02X ", ((char *) buf)[i]);
  }
  fprintf(stderr, "\n");
#endif

  return total;
}

size_t msg_size(enum MSG_TYPES type)
{
        switch (type) {
            case MSG_REQUEST_TASK: return sizeof(struct msg_request_task);
            case MSG_TASK: return sizeof(struct msg_task);
            case MSG_SOLUTION: return sizeof(struct msg_solution);
            case MSG_VERIFICATION: return sizeof(struct msg_verification);
            default: assert(false && "Message size not known!");
        }
}

union msg_wrapper read_msg(int fd)
{
  union msg_wrapper msg;
  read_len(fd, &msg, sizeof(struct msg_header));
  void *payload_ptr = (char *)&msg + sizeof(struct msg_header);
  read_len(fd, payload_ptr, msg.header.msg_len - sizeof(struct msg_header));
  return msg;
}

int write_msg(int fd, const union msg_wrapper *msg)
{
  return write_len(fd, msg, msg->header.msg_len);
}

union msg_wrapper create_msg(enum MSG_TYPES type)
{
  union msg_wrapper wrapper;
  wrapper.header.msg_type = type;
  wrapper.header.msg_len = msg_size(type);
  return wrapper;
}