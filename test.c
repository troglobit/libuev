/**
 * @file
 * Micro event library libuevent.
 * Test & showcase of libuevent
 *
 * (C) Copyright 2012 flemming.madsen at madsensoft.dk. See libuevent.h
 */
#include <stdlib.h>
#include <stdio.h>
#include "libuevent.h"

static period;
static client_fd, server_fd, service_fd;
static struct LUETimerH *timeOut;
int readFd, writeFd;

static timeOutHdl(struct LUECtxt *ctxt, struct LUETimerH *handle, void *data)
{
   timeOut = NULL;
   printf("Timeout is exceeded %p\n", data);
   lueRemFd(ctxt, readFd);
   lueExit(ctxt);
}

static appTimeout(struct LUECtxt *ctxt, struct LUETimerH *handle, void *data)
{
   printf("Lifetime is exceeded %p\n", data);
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
   int cnt = (int) data;
   char *msg = "TESTING";

   write(writeFd, msg, cnt);
   //printf("WRITE %.*s %d\n", cnt, msg, cnt);
   printf("%d ", cnt); fflush(stdout);
   period = cnt + 5;
   lueAddTimer(ctxt, period * 100,  writeThread, (void *) (cnt + 1));
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
