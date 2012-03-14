/**
 * @file
 * Micro event library libuevent.
 * Asynchronous event / mainloop library.
 * Not thread safe!
 * The ctxt structure needs mutex protection in case of multithreaded access
 * $Id:$
 *
 * (C) Copyright 2012 flemming.madsen at madsensoft.dk. See libuevent.h
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/times.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include "llist.h"
#include "libuevent.h"

#ifdef DEBUG
#define Debug(f, args...) printf(f "\n", ##args)
#else
#define Debug(f, args...) 
#endif

static clock_t clock_tick = 0;

/// Enumerate file handle direction
enum LUEFhDirectionT {LUE_FHIN = 0, LUE_FHOUT = 1};

/// File event handle
struct LUEFileEventH
{
   LListField(struct LUEFileEventH); ///< Elements for linked list

   int            fd;              ///< File descriptor
   enum LUEFhDirectionT direction; ///< Direction: in or out
   int            pollIx;          ///< This file handle index into list of file handles to poll
   LUEFileHandler handler;         ///< File handler callback
   void          *data;            ///< File handler callback parameter
};

/// Timer context object
struct LUETimerH
{
   LListField(struct LUETimerH);   ///< Linked list elements

   int            dueTime;  ///< Epoch timestamp
   LUETimerHandler handler; ///< Timer callback
   void          *data;     ///< Timer callback parameter
};

/**
 * @cond doxygen does not like this construction
 */
typedef LList(struct LUETimerH) LUETimerHandlerDL;
typedef LList(struct LUEFileEventH) LUEFileEventDL;
/**
 * @endcond
 */

/// Main lue context object
struct LUECtxt
{
   LUEFileEventDL fileHandlers;       ///< File handlers
   LUETimerHandlerDL timeoutHandlers; ///< Timer handlers

   LUEFileEventDL fileRem;            ///< List of file handles to be garbage collected
   LUETimerHandlerDL timeRem;         ///< List of timer handles to be garbage collected

   clock_t        baseTime;           ///< Time at last timer tick
   clock_t        setTime;            ///< Next timer tick epoch
   int            timeSet;            ///< True if next timer epoch is set
   int            exiting;            ///< True if the application is terminating
};

static
struct LUEFileEventH *lueAddIO(struct LUECtxt * ctxt,
                                  int fd, LUEFileHandler handler, void *data,
                                  enum LUEFhDirectionT direction)
{
   struct LUEFileEventH *ent;

   ent = (struct LUEFileEventH *) malloc(sizeof(*ent));
   assert(ent);
   ent->fd = fd;
   ent->direction = direction;
   ent->handler = handler;
   ent->data = data;
   ent->pollIx = -1;
   lListAppend(&ctxt->fileHandlers, ent);

   return ent;
}

/**
 * Add an input file handler
 */
struct LUEFileEventH *lueAddInput(struct LUECtxt *ctxt,
                                  int fd, LUEFileHandler handler, void *data)
{
    return lueAddIO(ctxt, fd, handler, data, LUE_FHIN);
}

/**
 * Add an output file handler
 */
struct LUEFileEventH *lueAddOutput(struct LUECtxt *ctxt,
                                  int fd, LUEFileHandler handler, void *data)
{
    return lueAddIO(ctxt, fd, handler, data, LUE_FHOUT);
}

static
int lueRemIO(struct LUECtxt *ctxt, struct LUEFileEventH *hdl)
{
   struct LUEFileEventH *ent;

   lListForeachIn (ent, &ctxt->fileRem)
      if (ent == hdl)
         return -1; // Already removed

   hdl->handler = NULL;
   lListAppend(&ctxt->fileRem, hdl);
   return 0;
}

/**
 * Remove all handlers for an fd
 */
int lueRemFd(struct LUECtxt *ctxt, int fd)
{
   struct LUEFileEventH *ent, *rem;

   lListForeachIn(ent, &ctxt->fileHandlers)
   {
      if (ent->fd != fd)
         continue;
      lListForeachIn(rem, &ctxt->fileRem) if (rem == ent) break; // Already removed
      if (rem) {
         ent->handler = NULL;
         lListAppend(&ctxt->fileRem, ent);
      }
   }
   return 0;
}

/**
 * Remove an input file handler
 */
int lueRemInput(struct LUECtxt *ctxt, struct LUEFileEventH *hdl)
{
   return lueRemIO(ctxt, hdl);
}

/**
 * Remove an output file handler
 */
int lueRemOutput(struct LUECtxt *ctxt, struct LUEFileEventH *hdl)
{
   return lueRemIO(ctxt, hdl);
}


/**
 * Remove a timer event
 */
int lueRemTimer(struct LUECtxt *ctxt, struct LUETimerH *hdl)
{
   if (hdl)
   {
      struct LUETimerH *ent;

      lListForeachIn (ent, &ctxt->timeRem)
         if (ent == hdl)
            return -1; // Already removed

      lListRemove(&ctxt->timeoutHandlers, hdl);
      lListAppend(&ctxt->timeRem, hdl); // Remember to free him (at a safe time)
   }
   return 0;
}

/**
 * Add a timer event
 */
struct LUETimerH *lueAddTimer(struct LUECtxt * ctxt, int periodMs,
                                  LUETimerHandler handler, void *data)
{
   int            timeDiff;
   clock_t        curTime;
   struct LUETimerH *nent,
                 *ret,
                 *ent;

   nent = (struct LUETimerH *) malloc(sizeof(*ent));
   assert(nent);
   nent->dueTime = periodMs;
   nent->handler = handler;
   nent->data = data;

   if (0 == periodMs)
   {
      // Zero delay is a special case: A oneshot workproc: Must go first
      lListInsertFirst(&ctxt->timeoutHandlers, nent);
      return nent;

   }

   // Adjust timers.
   // Jiffie wraparound is OK since we only care about diff time
   curTime = ctxt->timeSet ? ctxt->setTime : times(NULL);
   timeDiff = (curTime - ctxt->baseTime) / clock_tick * 1000
             + ((curTime - ctxt->baseTime) % clock_tick) * 1000 / clock_tick;

   ret = nent;
   lListForeachIn(ent, &ctxt->timeoutHandlers)
   {
      if (ent->dueTime > timeDiff)
         ent->dueTime -= timeDiff;
      else
         ent->dueTime = 0;
      if (nent && ent->dueTime > periodMs)
      {
         Debug("Insert %d before %d", periodMs, ent->dueTime);
         lListInsert(&ctxt->timeoutHandlers, nent, ent);
         nent = NULL;
      }
   }
   if (nent) 
   {
      Debug("Append %d at end", periodMs);
      lListAppend(&ctxt->timeoutHandlers, nent);
   }
   ctxt->baseTime = curTime;

   return ret;
}

/**
 * Create an application context
 */
struct LUECtxt *lueCtxtCreate()
{
   struct LUECtxt *lue;

   lue = (struct LUECtxt *) malloc(sizeof(*lue));
   assert(lue);
   lListInit(&lue->fileHandlers);
   lListInit(&lue->timeoutHandlers);
   lListInit(&lue->fileRem);
   lListInit(&lue->timeRem);

   lue->baseTime = times(NULL);
   lue->setTime = 0;
   lue->timeSet = false;
   lue->exiting = false;
   
   if (0 == clock_tick)
      clock_tick = sysconf(_SC_CLK_TCK);
   
   return lue;
}

/**
 * Destroy an application context
 */
void lueCtxtDestroy(struct LUECtxt *lue)
{
   lListPurge(&lue->fileHandlers);
   lListPurge(&lue->timeoutHandlers);
   free(lue);
}

/**
 * Drive the timeouts ourself (from now on until an increment of -1)
 * This is intended for use in testing frameworks
 * Caveat emptor: Currently, interactions with the scheduler (agent) API is undefined.
 *                Not for use in lues that use the agent scheduler
 * @param ctxt LUElication context object
 * @param mSecs Ms to advance. Use -1 to return to realtime
 */
void lueTimeAdvance(struct LUECtxt *ctxt, int mSecs)
{
   // Obviously we are executing a callback right now (arent we always)
   // The time leap is taken into account before falling back into poll() below
   if (mSecs >= 0)
   {
      if (!ctxt->timeSet)
      {
         ctxt->timeSet = true;
         ctxt->setTime = times(NULL) + mSecs;
      }
      else
      {
         ctxt->setTime += mSecs;
      }
   }
   else if (mSecs == -1)
   {
      // Go back to realtime
      ctxt->timeSet = false;
      ctxt->baseTime += times(NULL) - ctxt->setTime;
      ctxt->setTime = 0;
   }
}

/**
 * Process pending events
 */
static int _lueProcessPending(struct LUECtxt *ctxt, int immediate)
{
   struct LUETimerH     *timer;
   int            pollRet;
   clock_t        curTime;
   int            curPeriod = 0;

   // Do due timers
   if (lListHead(&ctxt->timeoutHandlers))
   {
      curTime = ctxt->timeSet ? ctxt->setTime : times(NULL);
      curPeriod = (curTime - ctxt->baseTime) / clock_tick * 1000
               + ((curTime - ctxt->baseTime) % clock_tick) * 1000 / clock_tick;
      Debug("Period for timer execution %d", curPeriod);
      while((timer = lListHead(&ctxt->timeoutHandlers)) &&
            timer->dueTime <= curPeriod)
      {
         timer->handler(ctxt, timer, timer->data);
         lueRemTimer(ctxt, timer);

         // baseTime changes for every lueAddTimer
         curPeriod = (curTime - ctxt->baseTime) / clock_tick * 1000
             + ((curTime - ctxt->baseTime) % clock_tick) * 1000 / clock_tick;
      }
      lListPurge(&ctxt->timeRem); // Reap timer zombies
   }

   // Calculate waiting time
   if (ctxt->exiting)
      curPeriod = 0;
   else if ((timer = lListHead(&ctxt->timeoutHandlers)) &&
            timer->dueTime > curPeriod)
   {
      Debug("Period from timer head %d - %d -> %d",
            timer->dueTime, curPeriod, timer->dueTime - curPeriod);
      curPeriod = timer->dueTime - curPeriod;
   }
   else if (NULL != timer)
      curPeriod = 0;
   else
      curPeriod = -1;

   // Do pipe handlers
   if (lListHead(&ctxt->fileHandlers) && !ctxt->exiting)
   {
      struct pollfd  polls[lListLength(&ctxt->fileHandlers)];
      struct LUEFileEventH *ent;

      int ix = 0;
      lListForeachIn(ent, &ctxt->fileHandlers)
      {
         ent->pollIx = ix;
         polls[ix].fd = ent->fd;
         polls[ix].events = LUE_FHIN == ent->direction ? POLLIN : POLLOUT;
         polls[ix].revents = 0;
         ix++;
      }
      Debug("Polling %d fds in %d ms", ix, curPeriod);
      pollRet = poll(polls, ix, immediate ? 0 : curPeriod);
      if (pollRet < 0)
      {
         if (EINTR == errno)
            return 0;
         else if (EBADF == errno)
         {
            lListForeachIn(ent, &ctxt->fileHandlers)
            {
               if (fcntl(ent->fd, F_GETFD) == -1 && EBADF == errno)
               {
                  ent->handler(ctxt, ent, ent->fd, ent->data);
               }
            }
         }
         else
         {
            // Error in poll. Cannot continue
            assert(false);
            return;
         }
      }
      else if (pollRet > 0)
      {
         lListForeachIn(ent, &ctxt->fileHandlers)
         {
            if (ent->handler && ent->pollIx >= 0 && polls[ent->pollIx].revents)
            {
               ent->handler(ctxt, ent, ent->fd, ent->data);
            }
         }
      }
      if (lListHead(&ctxt->fileRem))
      {
         struct LUEFileEventH *ent;

         while (NULL != (ent = lListHead(&ctxt->fileRem)))
         {
            lListRemove(&ctxt->fileHandlers, ent);
            free(ent);
         }
      }
   }
   else if (curPeriod > 0)
   {
      if (!immediate)
         poll(NULL, 0, curPeriod);
   }
   else if (curPeriod == -1)
      curPeriod = -2; // Nothing more to do

   return curPeriod;
}

/**
 * Process pending events
 */
int lueProcessPending(struct LUECtxt *ctxt)
{
   return _lueProcessPending(ctxt, 1);
}

/**
 * Run the application
 */
void lueRun(struct LUECtxt *ctxt)
{
   int ret;

   do {
      ret = _lueProcessPending(ctxt, 0);
   } while (!ctxt->exiting && ret != -2);
   ctxt->exiting = false;
   Debug("LUE exiting - bye");
}

/**
 * Terminate the application
 */
void lueTerminate(struct LUECtxt *ctxt)
{
   ctxt->exiting = true;
}

// vim: set ts=8 sw=3 sts=3 et:
