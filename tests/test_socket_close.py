#!/usr/bin/env python3
"""Exercise the actual close functions with controlled socket syscall outcomes."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'source/ftp.c').read_text()
start = source.index('static void\nftp_shrink_socket(')
end = source.index('/*! close command socket', start)
functions = source[start:end]
preamble = r'''
#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#define RED ""
#define RESET ""
#define YELLOW ""
static char calls[32];
static int call_count, fail_receive, fail_send;
static void record(char c) { calls[call_count++] = c; calls[call_count] = 0; }
static void diagnostic_event(const char* s, const void* p, int fd, long rc)
{ (void)s; (void)p; (void)fd; (void)rc; }
static void console_print(const char* f, ...) { (void)f; }
static int test_getpeername(int fd, struct sockaddr* a, socklen_t* n)
{ (void)fd; (void)a; (void)n; return -1; }
static int test_shutdown(int fd, int how)
{ (void)fd; assert(how == SHUT_WR); record('H'); return 0; }
static int test_poll(struct pollfd* fds, nfds_t n, int timeout)
{ (void)fds; assert(n == 1 && timeout == 250); record('P'); return 0; }
static int test_setsockopt(int fd, int level, int opt, const void* val, socklen_t n)
{
    (void)fd; assert(level == SOL_SOCKET);
    if (opt == SO_LINGER) {
        const struct linger* l = val;
        assert(l->l_onoff == 1 && l->l_linger == 0); record('L'); return 0;
    }
    assert(n == sizeof(int) && *(const int*)val == 1);
    if (opt == SO_RCVBUF) { record('R'); return fail_receive ? -1 : 0; }
    assert(opt == SO_SNDBUF); record('S'); return fail_send ? -1 : 0;
}
static int test_close(int fd) { (void)fd; record('C'); return 0; }
#define getpeername test_getpeername
#define shutdown test_shutdown
#define poll test_poll
#define setsockopt test_setsockopt
#define close test_close
'''
checks = r'''
int main(void)
{
    ftp_closesocket(7, true, true);
    assert(strcmp(calls, "HPLRSC") == 0);
    call_count = 0;
    ftp_closesocket(7, true, false);
    assert(strcmp(calls, "HPLC") == 0);
    call_count = 0;
    ftp_closesocket(7, false, true);
    assert(strcmp(calls, "LRSC") == 0);
    for (int failures = 1; failures <= 3; ++failures) {
        call_count = 0;
        fail_receive = failures & 1; fail_send = failures & 2;
        ftp_closesocket(7, true, true);
        assert(strcmp(calls, "HPLRSC") == 0);
    }
    puts("PASS close ordering, unchanged command sockets, passive sockets, close despite shrink failures");
}
'''
with tempfile.TemporaryDirectory() as directory:
    path = Path(directory)
    (path / 'test.c').write_text(preamble + functions + checks)
    subprocess.run(['cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                    str(path / 'test.c'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
