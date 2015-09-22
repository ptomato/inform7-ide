/* Unix methods needed to run intest on Windows, notably a minimal pthreads implementation */

/* From the Windows SDK */

struct Win32_Critical_Section { void* v1; long v2; long v3; long v4; long v5; void* v6; };
struct Win32_Startup_Info { long v1; char* v2; char* v3; char* v4; long v5; long v6; long v7; long v8; long v9; long v10; long v11;

                            unsigned long flags; unsigned short showWindow; short v12; char* v13; long v14; long v15; long v16; };

struct Win32_Process_Info { unsigned long process; unsigned long thread; long v1; long v2; };


void __stdcall EnterCriticalSection(struct Win32_Critical_Section* cs);
void __stdcall LeaveCriticalSection(struct Win32_Critical_Section* cs);
unsigned long __stdcall CreateThread(void* attrs, unsigned long stack, void* func, void* param, unsigned long flags, unsigned long* id);

unsigned long __stdcall CreateProcessA(void* app, char* cmd, void* pa, void* ta, long inherit, unsigned long flags, void* env, void* dir,

                                       struct Win32_Startup_Info* start, struct Win32_Process_Info* process);

unsigned long __stdcall GetExitCodeProcess(unsigned long proc, unsigned long* code);

unsigned long __stdcall CloseHandle(unsigned long handle);

unsigned long __stdcall WaitForSingleObject(unsigned long handle, unsigned long ms);

void __stdcall Sleep(unsigned long ms);
unsigned long __stdcall GetCurrentDirectoryA(unsigned long len, char* buffer);

unsigned long __stdcall SHGetFolderPathA(unsigned long wnd, int folder, unsigned long token, unsigned long flags, char* path);


/* Internal definitions */

struct Win32_Thread_Attrs {};
struct Win32_Thread_Start { void *(*fn)(void *); void* arg; };

unsigned long __stdcall Win32_Thread_Func(unsigned long param)

{

  struct Win32_Thread_Start* start = (struct Win32_Thread_Start*)param;
  (start->fn)(start->arg);
  free(start);
  return 0;
}


char* Win32_getenv(const char *name)

{

  static char env[260];

  env[0] = 0;



  if (strcmp(name,"PWD") == 0)

  {

    if (GetCurrentDirectoryA(260,env) > 0)

      return env;

  }

  else if (strcmp(name,"HOME") == 0)

  {

    if (SHGetFolderPathA(0,5,0,0,env) == 0)

      return env;

  }

  return getenv(name);

}


int Win32_system(const char* cmd)

{

  if (strncmp(cmd,"md5 ",4) == 0)

    return 0;



  char cmdline[4096];

  sprintf(cmdline,"cmd /s /c \"%s\"",cmd);



  struct Win32_Startup_Info start = { sizeof (struct Win32_Startup_Info), 0 };

  start.flags = 1;

  start.showWindow = 0;

  struct Win32_Process_Info process;

  if (CreateProcessA(0,cmdline,0,0,0,0x8000000,0,0,&start,&process) == 0)

    return -1;

  CloseHandle(process.thread);

  if (WaitForSingleObject(process.process,-1) != 0)

  {

    CloseHandle(process.process);

    return -1;

  }



  unsigned long code = 10;

  GetExitCodeProcess(process.process,&code);

  CloseHandle(process.process);

  return code;

}


/* Implementation of needed functions */

typedef unsigned long pthread_t;
typedef struct Win32_Thread_Attrs pthread_attr_t;
typedef struct Win32_Critical_Section pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER { (void*)-1,-1,0,0,0,0 }

#define getenv(name) Win32_getenv(name)
#define system(cmd) Win32_system(cmd)

void pthread_mutex_lock(pthread_mutex_t* pm)
{
  EnterCriticalSection(pm);
}

void pthread_mutex_unlock(pthread_mutex_t* pm)
{
  LeaveCriticalSection(pm);
}

int pthread_create(pthread_t* pt, const pthread_attr_t* pa, void *(*fn)(void *), void* arg)
{
  struct Win32_Thread_Start* start = (struct Win32_Thread_Start*)malloc(sizeof (struct Win32_Thread_Start));
  start->fn = fn;
  start->arg = arg;
  unsigned long thread = CreateThread(0,0,Win32_Thread_Func,start,0,0);
  if (thread == 0)
  {
    free(start);
    return 1;
  }
  else
  {
    *pt = thread;
    return 0;
  }
}

int pthread_join(pthread_t pt, void** rv)
{
  return (WaitForSingleObject(pt,-1) == 0) ? 0 : 1;
}

void pthread_attr_init(pthread_attr_t* pa)
{
}

void pthread_attr_setstacksize(pthread_attr_t* pa, size_t sz)
{
}

void sleep(int ms)
{
  Sleep(ms);
}
