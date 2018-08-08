/*****************************************************************************
 *   Copyright (C) 2008-2013 Ana-Maria Visan, Kapil Arya, and Gene Cooperman *
 *   amvisan@cs.neu.edu, kapil@cs.neu.edu, and gene@ccs.neu.edu              *
 *                                                                           *
 *  This file is part of the PTRACE plugin of DMTCP (DMTCP:plugin/ptrace).   *
 *                                                                           *
 *  DMTCP is free software: you can redistribute it and/or                   *
 *  modify it under the terms of the GNU Lesser General Public License as    *
 *  published by the Free Software Foundation, either version 3 of the       *
 *  License, or (at your option) any later version.                          *
 *                                                                           *
 *  DMTCP is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU Lesser General Public         *
 *  License along with DMTCP:dmtcp/src.  If not, see                         *
 *  <http://www.gnu.org/licenses/>.                                          *
 *****************************************************************************/

#include "ptrace.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "jalloc.h"
#include "jassert.h"
#include "dmtcp.h"
#include "ptraceinfo.h"
#include "util.h"

using namespace dmtcp;

static int originalStartup = 1;

EXTERNC int
dmtcp_ptrace_enabled()
{
  return 1;
}

void
ptraceInit()
{
  PtraceInfo::instance().createSharedFile();
  PtraceInfo::instance().mapSharedFile();
}

void
ptraceWaitForSuspendMsg(DmtcpEventData_t* data)
{
  PtraceInfo::instance().markAsCkptThread();

  if (!originalStartup) {
    PtraceInfo::instance().waitForSuperiorAttach();
  } else {
    originalStartup = 0;
  }
}

void
ptraceProcessResumeUserThread(DmtcpEventData_t* data)
{
  printf("\n*** Before ResumeUserThread  ***\n");
  printf("\n*** Value of  data: %d  ***\n",
         data->resumeUserThreadInfo.isRestart);
  ptrace_process_resume_user_thread(data->resumeUserThreadInfo.isRestart);
  printf("\n*** After ResumeUserThread  ***\n");
}

static void
dmtcp_ptrace_event_hook(DmtcpEvent_t event, DmtcpEventData_t* data)
{
  switch (event) {
    case DMTCP_EVENT_INIT:
      ptraceInit();
      printf("\n*** HOOK The init event has been called   ***\n");
      break;

    case DMTCP_EVENT_WAIT_FOR_CKPT:
      printf("\n*** HOOK The waitforcheckpoint event  called   ***\n");
      ptraceWaitForSuspendMsg(data);
      break;

    case DMTCP_EVENT_PRE_SUSPEND_USER_THREAD:
      printf("\n*** HOOK  pre_suspend user thread  called ***\n");
      ptrace_process_pre_suspend_user_thread();

      break;

    case DMTCP_EVENT_RESUME_USER_THREAD:
      printf("\n*** HOOK The event resume user thread has been called   ***\n");
      ptraceProcessResumeUserThread(data);
      break;

    case DMTCP_EVENT_ATFORK_CHILD:
      originalStartup = 1;
      printf("\n*** HOOK The event atfork event has been called   ***\n");
      break;

    default:
      break;
  }
}
static void
checkpoint()
{
  printf("\n*** BARRIER The plugin is  called before checkpointing. ***\n");
}

static void
resume()
{
  printf("*** BARRIER The application has now been checkpointed. ***\n");
}

static void
restart()
{
  printf("*** BARRIER The application is now restarting from a checkpoint.\n");
}

static DmtcpBarrier barriers[] = {
  { DMTCP_GLOBAL_BARRIER_PRE_CKPT, checkpoint, "checkpoint" },
  { DMTCP_GLOBAL_BARRIER_RESUME, resume, "resume" },
  { DMTCP_GLOBAL_BARRIER_RESTART, restart, "restart" }
};
DmtcpPluginDescriptor_t ptrace_plugin = { DMTCP_PLUGIN_API_VERSION,
                                          DMTCP_PACKAGE_VERSION,
                                          "ptrace",
                                          "DMTCP",
                                          "dmtcp@ccs.neu.edu",
                                          "Ptrace plugin",
                                          DMTCP_DECL_BARRIERS(barriers),
                                          dmtcp_ptrace_event_hook };

DMTCP_DECL_PLUGIN(ptrace_plugin);
