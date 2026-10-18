# Close-time buffer shrink hardware comparison

Test console: HOS 22.5.0. The test does not establish HOS 23.x compatibility.

Baseline commit: 7956beb. Workaround commit: f303df5, derived from
[cathery/sys-ftpd PR #34](https://github.com/cathery/sys-ftpd/pull/34).
Heap capacity (5,505,024 bytes), startup socket buffers, and transfer-time
buffers were identical. The added behavior shrinks passive-listener and data
socket buffers immediately before close. Command sockets remain unchanged.

## Baseline

PASV socket creation failed at accepted session 14 with errno 105 (ENOBUFS).
There was one active session and 13 destroyed sessions. The process accepted
another session about 17 seconds later. The host observed a PASV 451 and
subsequent connection resets.

## Workaround build, 2026-10-18

Startup confirmed `variant=shrink-before-close`. Empty and 1,048,613-byte
binary upload/download matched exactly. Rename, read after rename, deletion,
and empty-directory listing passed.

The rapid reconnection sequence passed four requests, then request five timed
out connecting. The intended 64-request run and following 16 repeated transfers
were therefore not completed. The log records accept() failing with errno 113
(ECONNABORTED), with zero active sessions and 15 destroyed sessions, followed
by listener cleanup/restart. The service later recovered without a console
reboot. No PASV ENOBUFS was recorded in the captured modified-build run.

The captured log through subsequent investigation contains 56 successful
shrink setsockopt calls and no shrink failures. Thus the workaround is executing,
but this test does not establish resolution of rapid connection failures.

A subsequent NOOP/HEAD connection check also encountered an accept failure;
it was not isolated from the prior load and is not an independent clean-start
command-only stress result. The command socket's unchanged large startup buffers
are a candidate for the next controlled comparison, not an established cause.

All uniquely named remote test files and the test directory were removed after
service recovery. The workaround build and diagnostics marker remain installed.
Sleep/wake remains untested. Raw logs/results are retained locally under the
ignored `deployments/shrink-retest-20261018-211159/` directory.
