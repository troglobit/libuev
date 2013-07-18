/* Test libuev event library
 *
 * Copyright (c) 2012  Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013  Joachim Nilsson <troglobit()gmail!com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>             /* intptr_t */

#include "libuev.h"

static period;
static client_fd, server_fd, service_fd;
static struct LUETimerH *timeOut;
int readFd, writeFd;

static timeOutHdl(struct LUECtxt *ctxt, struct LUETimerH *handle, void *data)
{
   timeOut = NULL;
   printf("Timeout exceeded %p\n", data);
   lueRemFd(ctxt, readFd);
   lueTerminate(ctxt);
}

static appTimeout(struct LUECtxt *ctxt, struct LUETimerH *handle, void *data)
{
   printf("Lifetime exceeded %p\n", data);
   exit(1);
}

static int readCb(struct LUECtxt *ctxt, struct LUEFileEventH *handle, int fd, void *data)
{
   if (timeOut) {
      lueRemTimer(ctxt, timeOut);
      timeOut = lueAddTimer(ctxt, 950, timeOutHdl, NULL);
   }

   char msg[50];
   int cnt = read(readFd, msg, sizeof(msg));
   //printf("READ %.*s %d\n", cnt, msg, cnt);
   printf("%.*s.%d ", cnt, msg, cnt); fflush(stdout);

   return 0;
}

static writeThread(struct LUECtxt *ctxt, struct LUETimerH *handle, void *data)
{
   int cnt = (int)(intptr_t)data;
   char *msg = "TESTING";

   write(writeFd, msg, cnt);
   //printf("WRITE %.*s %d\n", cnt, msg, cnt);
   printf("%d ", cnt); fflush(stdout);
   period = cnt + 5;
   lueAddTimer(ctxt, period * 100,  writeThread, (void *)(intptr_t)(cnt + 1));
}

main()
{
   struct LUECtxt *ctxt = lueCtxtCreate();

   timeOut = lueAddTimer(ctxt, 950, timeOutHdl, (void *)1);
   lueAddTimer(ctxt, 5000, appTimeout, (void *)2);

   lueAddTimer(ctxt, 500,  writeThread, (void *) 1);

   int fd[2];
   pipe(fd);
   readFd = fd[0];
   writeFd = fd[1];
   lueAddInput(ctxt, readFd, readCb, NULL);

   lueRun(ctxt);
   lueCtxtDestroy(ctxt);

   printf("Period is %d must be 10: %s\n", period, 10 == period ? "OK" : "ERROR!");
   exit(10 == period ? 0 : 1);
}

// vim: set sw=3 sts=3 et:
