#pragma once

#include <vector>
#include <string>
#include <chrono>

using namespace std;

extern int tests_total;
extern int tests_fail;
#define TEST(EX) do { ++tests_total; if(!(EX)) {\
    fprintf(stderr, "failed: [%s] testing [%s] in %s, line %d\n", __FUNCTION__, #EX, __FILE__, __LINE__); \
    ++tests_fail; }} while (0)



const float TEST_EPS=0.005;
inline int within_eps(float a, float b) { return a-TEST_EPS<b && b<a+TEST_EPS; }



class __ProfileScopeClass
{
unsigned long long start;
const std::string fname;
const std::string name;
int level;
int line;

static unsigned long long get_cpu_time_raw_int(void)
  {
  //struct timespec t1;
  //clock_gettime(CLOCK_REALTIME,&t1);
  //return (unsigned long long)t1.tv_sec*(unsigned long long)1000000000+(unsigned long long)t1.tv_nsec;
  long long c1=std::chrono::high_resolution_clock::now().time_since_epoch().count();
  return c1;
  }


public:


__ProfileScopeClass(int line,const std::string& fname,int level,const std::string& name="")
  :fname(fname), name(name), level(level), line(line)
  {
  start=get_cpu_time_raw_int();
  }

~__ProfileScopeClass()
  {
  if(level==1)printf("%30s(%4d) :  %lf ms\n",(name+"@"+fname).c_str(),line,double(get_cpu_time_raw_int()-start)/1e6);
  if(level==2)printf("%30s(%4d) :  %lf us\n",(name+"@"+fname).c_str(),line,double(get_cpu_time_raw_int()-start)/1e3);
  if(level==3)printf("%30s(%4d) :  %lf ns\n",(name+"@"+fname).c_str(),line,double(get_cpu_time_raw_int()-start));
  }

};

#define COMBINE1(X,Y) X##Y
#define COMBINE(X,Y) COMBINE1(X,Y)


#define TIME(...) __ProfileScopeClass COMBINE(__t,__LINE__) (__LINE__,__FUNCTION__,__VA_ARGS__);


#define NOT_IMPLEMENTED() do{static bool done=false;if(!done)fprintf(stderr,"Function \"%s\"  in file \"%s\" line %d not implemented yet!!!\n",__FUNCTION__, __FILE__, __LINE__);done=true;}while(0)
//#define NOT_IMPLEMENTED() 


#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include <climits>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <semaphore.h>
#include <errno.h>
#include <dirent.h>
#include <sched.h>
#include <getopt.h>
#include <netdb.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/inotify.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/futex.h>






class OMPCLASS4
  {
  public:
  int n;
  std::vector < std::thread > th;
  
  std::atomic < int > m,w,count;
  
  
  std::function<void(int,int)>*fun;
  
  void threadfun(int threadnum,int localm)
    {
    while(1)
      {
      syscall(SYS_futex,&m,FUTEX_WAIT | FUTEX_PRIVATE_FLAG,localm,NULL,NULL,0);
      localm=m;
      if(localm==8)if(threadnum>=n)break;
      
      if(localm<8)(*fun)(threadnum,n);
      
      if((--count)==0)
        {
        w=1;
        syscall(SYS_futex,&w,FUTEX_WAKE | FUTEX_PRIVATE_FLAG,INT_MAX,NULL,NULL,0);
        }
      }
    } 
  
  OMPCLASS4(){n=0;m=0;}
  ~OMPCLASS4(){decrease(0);}
  
  void set(int nt)
    {
    if(n==1)return;
    if(nt<0)nt=0;
    if(n<nt)increase(nt);
    if(n>nt)decrease(nt);
    }
  
  void increase(int nt)
    {
    for(;n<nt;n++)th.push_back(std::thread(&OMPCLASS4::threadfun,this,n,(int)m));
    }
    
  
  void decrease(int nt)
    {
    count=nt;
    n=nt;
    m=8;
    w=(n==0);
    syscall(SYS_futex,&m,FUTEX_WAKE | FUTEX_PRIVATE_FLAG,INT_MAX,NULL,NULL,0);
    for(size_t q1=n;q1<th.size();q1++)th[q1].join();
    th.resize(n);
    syscall(SYS_futex,&w,FUTEX_WAIT | FUTEX_PRIVATE_FLAG,0,NULL,NULL,0);
    }
  
  void run(std::function<void(int,int)> &a)
    {
    if(!n)return;
    
    fun=&a; 
    
    if(n==1){(*fun)(0,1);return;}
    
    w=0;
    m=(m+1)%8;
    count=n;
    
    syscall(SYS_futex,&m,FUTEX_WAKE | FUTEX_PRIVATE_FLAG,INT_MAX,NULL,NULL,0);
    syscall(SYS_futex,&w,FUTEX_WAIT | FUTEX_PRIVATE_FLAG,0,NULL,NULL,0);
    
    }
  }; 
  
  
  
  
  
    
  
#define OMP4(NTHREADS,CODE)\
do{\
  auto lambda=[&](int threadnum,int nthreads){CODE;};\
  std::function<void(int,int)> fun(lambda);\
  thread_local OMPCLASS4 OMPDATA;\
  OMPDATA.set(int(NTHREADS));\
  OMPDATA.run(fun);\
  }while(0)
    
#define OMP4for(NTHREADS,START,LENGTH,CODE)\
  OMP4((NTHREADS),{int A=int(START)+int(LENGTH)*(threadnum+0)/nthreads;int B=int(START)+int(LENGTH)*(threadnum+1)/nthreads;{CODE;}})

#define OMP4for2(NTHREADS,START,LENGTH,VAR,CODE)\
  OMP4((NTHREADS),{int __A=int(START)+int(LENGTH)*(threadnum+0)/nthreads;int __B=int(START)+int(LENGTH)*(threadnum+1)/nthreads;for(int VAR=__A;VAR<__B;VAR++){CODE;}})


    
#define OMP4forll(NTHREADS,START,LENGTH,CODE)\
  OMP4((NTHREADS),{typedef long long __I;__I A=__I(START)+__I(LENGTH)*(threadnum+0)/nthreads;__I B=__I(START)+__I(LENGTH)*(threadnum+1)/nthreads;{CODE;}})

#define OMP4for2ll(NTHREADS,START,LENGTH,VAR,CODE)\
  OMP4((NTHREADS),{typedef long long __I;__I __A=__I(START)+__I(LENGTH)*(threadnum+0)/nthreads;__I __B=__I(START)+__I(LENGTH)*(threadnum+1)/nthreads;for(__I VAR=__A;VAR<__B;VAR++){CODE;}})




