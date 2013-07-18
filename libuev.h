/* libuev - Asynchronous event loop library
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

#ifndef LIBUEV_H
#define LIBUEV_H

struct LUETimerH;
struct LUEFileEventH;
struct LUECtxt;

/**
 * Typedef file handler callbacks
 */
typedef int (*LUEFileHandler)(struct LUECtxt *ctxt,
                              struct LUEFileEventH *handle,
                              int fd,
                              void *data);
/**
 * Typedef timer handler callbacks
 */
typedef int (*LUETimerHandler)(struct LUECtxt *ctxt,
                              struct LUETimerH *handle,
                              void *data);

struct LUEFileEventH *lueAddInput(struct LUECtxt *ctxt,
                                  int fd, LUEFileHandler handler, void *data);
int lueRemInput(struct LUECtxt *ctxt, struct LUEFileEventH *hdl);
struct LUEFileEventH *lueAddOutput(struct LUECtxt *ctxt,
                                  int fd, LUEFileHandler handler, void *data);
int lueRemOutput(struct LUECtxt *ctxt, struct LUEFileEventH *hdl);
int lueRemFd(struct LUECtxt *ctxt, int fd);

struct LUETimerH *lueAddTimer(struct LUECtxt *ctxt, int periodMs,
                              LUETimerHandler handler, void *data);
int lueRemTimer(struct LUECtxt *ctxt, struct LUETimerH *hdl);

struct LUECtxt *lueCtxtCreate();
void lueCtxtDestroy(struct LUECtxt *lue);
void lueTimeAdvance(struct LUECtxt *ctxt, int mSecs);
void lueRun(struct LUECtxt *ctxt);
int lueProcessPending(struct LUECtxt *ctxt);
void lueTerminate(struct LUECtxt *ctxt);

#endif /* LIBUEV_H */

// vim: set ts=8 sw=3 sts=3 et:
