#include <bits/stdc++.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<time.h>
#include<random>
#include<algorithm>
#include<string>
#define PRINTING_STATION 4
#define BINDING_STATION 2
#define MAX_READER 2
#define AVAILABLE -1
#define WAITING 0
#define PRINTING 1
#define FINISHED 2
int time_for_reading;
int time_for_binding;
int time_for_printing;


using namespace std;
string file_ans="";

class student
{
    int id;
    int group_no;
    int group_size;
    bool isLeader;
    int state;
    
    public:
    student(int id,int group_size)
    {
        this->id=id;
        this->group_size=group_size;
        this->group_no=(int)ceil((id/(group_size*1.00)));
        if(id%group_size==0)
            isLeader=true;
        else
            isLeader=false;
        state=AVAILABLE;

    }
    void print_info()
    {
        cout<<"Student "<<id<<" is in group "<<group_no<<endl;
        if(isLeader)
            cout<<"Student "<<id<<" is the leader of group "<<group_no<<endl;
    }
    int get_group_no()
    {
        return group_no;
    }
    bool get_isLeader()
    {
        return isLeader;
    }
    int getid()
    {
        return id;
    }
    void setsate(int state)
    {
        this->state=state;
        
    }
    int getstate()
    {
        return state;
    }
  };

int generateRandomNumber(int min_val,int max_val)
{
    random_device rd;
    mt19937 gen(rd());
    poisson_distribution<int> dis((min_val-max_val)/2);
    return dis(gen)+min_val;
}
//normal vars
int total_std;
int* binder;
int student_per_group;
int no_of_groups;
//pthreads
pthread_t* student_thread;
pthread_t* groupleader_thread;
pthread_t library_thread;
time_t start_time,curtime;
//semaphores

//mutex
pthread_mutex_t printing_station[PRINTING_STATION];
pthread_mutex_t printing;
pthread_mutex_t creating;
pthread_mutex_t waitingforprinting[PRINTING_STATION];
pthread_mutex_t waitingforbinding;
pthread_mutex_t binding_station[BINDING_STATION];
pthread_mutex_t preparingforbinding;
pthread_mutex_t finishedbinding;
pthread_mutex_t entrybook;//the lock on the entrybook
//semaphores
sem_t readers_sem;//the entrybook semaphore
//condition variable
pthread_cond_t condition1[PRINTING_STATION];
pthread_cond_t *condition2;
pthread_cond_t condition3;
pthread_cond_t condition4;
//noral vars
int printing_signal[PRINTING_STATION]={-1,-1,-1,-1};
vector<int> printing_station_que[PRINTING_STATION];

int printing_station_queue[PRINTING_STATION];
int binding_station_state[BINDING_STATION]={-1,-1};
int total_submission=0;
//-1 means free and 1 means busy
int readers_count=0;//to keep the count of the readers
//--------------------------------------------------------------------------------------
int getTime()
{
    time(&curtime);
    double total_time=double(curtime-start_time);
    return (int)total_time;
    
}
bool removefromqueue(int q,int id)
{
   //this function removes a student from waiting queue
   //q is the PS number
   //id student id
    int index=0;
    while(printing_station_que[q].size()>=index)
    {
        if(printing_station_que[q].at(index)==id)
        {
            printing_station_que[q].erase(printing_station_que[q].begin()+index);
            return true;
        }

index++;
    }
    return false;

}
bool searchqueue(int id)
{
    //search a queue to find whether a student has any groupmate in the queue or not
    int mygroup=(int)ceil((id/(student_per_group*1.00)));
    int other_group;
      int l=0;
   
    vector<int> temp=printing_station_que[id%PRINTING_STATION];
    for(int n=0;n<temp.size();n++)
    {
        other_group=(int)ceil((temp[n]/(student_per_group*1.00)));
        if(other_group==mygroup)
        {
            return true;
        }
    }
    return false;


}
void* simulateSubmitReport(void* ptr)
{
    //the first function

student* s=(student*)ptr;

pthread_mutex_lock(&printing);
//printing is a lock just used for safe printing
cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" has arrived at the print station at time "<<getTime()<<endl;
file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" has arrived at the print station at time "+to_string(getTime())+"\n";

pthread_mutex_unlock(&printing);

int desired_ps=(s->getid()%PRINTING_STATION);//o->PS1
//the desired PS for a student 
pthread_mutex_lock(&waitingforprinting[desired_ps]);
//this is a lock used to protect the conditional variable and the waiting room
bool flag2=false;
//ensures that a student is added in the waiting queue only once
while(1)
{
    if(flag2==false){
     printing_station_que[desired_ps].push_back(s->getid());
     flag2=true;
         cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" is waiting and checking for Signals for print station "<<desired_ps<<" at time "<<getTime()<<endl;
        file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" is waiting and checking for Signals for print station "+to_string(desired_ps)+" at time "+to_string(getTime())+"\n";

     //add waiting students to the queue
    }
    //cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" is waiting and checking for Signals for print station "<<desired_ps<<" at time "<<getTime()<<endl;
    //cout<<"printing signal "<<printing_signal[desired_ps]<<endl;

    if(printing_signal[desired_ps]==-1)
    {
        //signal -1 is means either the first student on this ps or a signal for all has been sent
        // //newly added
         if(printing_station_queue[desired_ps]==1)
         {
            //this checks say a process received a signal but before it can go some other process again acquired the printing station
                    pthread_cond_wait(&condition1[desired_ps],&waitingforprinting[desired_ps]);

                    if(printing_signal[desired_ps]==-1)
                    {
                         cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" has allocated(normally)  for print station "<<desired_ps<<" at time "<<getTime()<<endl;
                         file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" is waiting and checking for Signals for print station "+to_string(desired_ps)+" at time "+to_string(getTime())+"\n";

         //cout<<"removing from queue"<<endl;
        removefromqueue(desired_ps,s->getid());
        //cout<<"removed from queue"<<endl;
        break;

                    }

        //  //   cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" is again in wating for print station "<<desired_ps<<" at time "<<getTime()<<endl;
         }
         else{
            //a signal for all has beensent and the priting station is free
        cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" has received MSG(normally) for print station "<<desired_ps<<" at time "<<getTime()<<endl;
         file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" has received MSG(normally) for print station "+to_string(desired_ps)+" at time "+to_string(getTime())+"\n";

         //cout<<"removing from queue"<<endl;
        removefromqueue(desired_ps,s->getid());
        //cout<<"removed from queue"<<endl;
        break;
        //it means a singal has come that told everyone that the Desired PS is now free
         }
    }
    else if(printing_signal[desired_ps]==s->get_group_no())
    {
        //special signal from only groupmate
        if(printing_station_queue[desired_ps]==1)
        {
            //if some other groupmate already acquired the singal
                                pthread_cond_wait(&condition1[desired_ps],&waitingforprinting[desired_ps]);
continue;
        }
else{
        //a member from the group has texted that The desired PS is now free
        cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" has received MSG(on group priority)  for print station "<<desired_ps<<" at time "<<getTime()<<endl;
       file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" has received MSG(on group priority)  for print station "+to_string(desired_ps)+" at time "+to_string(getTime())+"\n";

       // cout<<"removing from queue"<<endl;
        removefromqueue(desired_ps,s->getid());
        //cout<<"removed from queue"<<endl;
        break;
}
    }
    else
    {
        //set the student to waiting 
        s->setsate(WAITING);
       
        pthread_cond_wait(&condition1[desired_ps],&waitingforprinting[desired_ps]);
    }

}

pthread_mutex_unlock(&waitingforprinting[desired_ps]);


if(pthread_mutex_lock(&printing_station[desired_ps])!=0)
{
    cout<<"lock failed for printing station "<<desired_ps<<endl;
}
s->setsate(PRINTING);
//student printing
printing_station_queue[desired_ps]=1;//setting PS BUSY
cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" has started printing at print station "<<desired_ps<<" at time "<<getTime()<<"----------"<<endl;
file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" has started printing at print station "+to_string(desired_ps)+" at time "+to_string(getTime())+"----------\n";


int delay=generateRandomNumber(10,17);
printing_signal[desired_ps]=-2;
sleep(12*time_for_printing);
cout<<"Student "<<s->getid()<<" from group no "<<s->get_group_no()<<" has finished printing at print station "<<desired_ps<<" at time "<<getTime()<<"------------"<<endl;
file_ans+="Student "+to_string(s->getid())+" from group no "+to_string(s->get_group_no())+" has finished printing at print station "+to_string(desired_ps)+" at time "+to_string(getTime())+"------------\n";


printing_station_queue[desired_ps]=-1;
if(pthread_mutex_unlock(&printing_station[desired_ps])!=0)
{
    cout<<"unlock failed for printing station "<<desired_ps<<endl;
}
s->setsate(FINISHED);
bool flag=false;

pthread_mutex_lock(&printing);

if(searchqueue(s->getid()))
{
    //giving signal to groupmates
   int o=s->get_group_no();
printing_signal[desired_ps]=o;
pthread_cond_signal(&condition1[desired_ps]);

//cout<<"primary signal send for desired ps "<<desired_ps<<" by student "<<s->getid() <<endl; 
}
else{
//cout<<"no primary  signal send"<<  desired_ps<<endl;
}


sleep(5);

pthread_mutex_unlock(&printing);

pthread_mutex_lock(&printing);
//giving signal to all students
printing_signal[desired_ps]=-1;
pthread_cond_signal(&condition1[desired_ps]);
//cout<<"all singals send for "<<desired_ps<<" by student  "<<s->getid()<<endl;


pthread_mutex_unlock(&printing);


pthread_mutex_lock(&waitingforbinding);

binder[s->get_group_no()-1]++;
if(binder[s->get_group_no()-1]==student_per_group)
{
    cout<<"all students from group "<<s->get_group_no()<<" have finished printing!!!! at time "<<getTime()<<"*********"<<endl;
    file_ans+="all students from group "+to_string(s->get_group_no())+" have finished printing!!!! at time "+to_string(getTime())+"*********\n";
    pthread_cond_signal(&condition2[s->get_group_no()-1]);
}

pthread_mutex_unlock(&waitingforbinding);
//pthread_exit(NULL);


    return NULL;

}
void* simulatebinding(void *ptr)
{
    //binding
    student* s=(student*)ptr;
    pthread_mutex_lock(&waitingforbinding);
    while(1)
    {
       // cout<<"***Leader******** "<<s->getid()<<" from group "<<s->get_group_no()<<" is waiting for binding at time "<<getTime()<<endl;
            if(binder[s->get_group_no()-1]==student_per_group)
            {
                cout<<"LEADER:::All the students have submitted their report to leader from group"<<s->get_group_no()<<" at time "<<getTime()<<endl;
               file_ans+="LEADER:::All the students have submitted their report to leader from group"+to_string(s->get_group_no())+" at time "+to_string(getTime())+"\n";

                break;
            }
            else
            {
                pthread_cond_wait(&condition2[s->get_group_no()-1],&waitingforbinding);
            }
    }

    pthread_mutex_unlock(&waitingforbinding);
    //now the group leader has all the reports
    //now he will go to the binding station and try to find a free binding station
    int ans=-1;
    pthread_mutex_lock(&preparingforbinding);
    while(1){
    if(binding_station_state[0]==-1)
    {
        //means 1 is free
        cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" Has got access  at binding station 1 at time "<<getTime()<<endl;
        file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" Has got access  at binding station 1 at time "+to_string(getTime())+"\n";

        ans=0;
        break;

    }
    else if(binding_station_state[1]==-1)
    {
        //means 2 is free
        cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" Has got access  at binding station 2 at time "<<getTime()<<endl;
        file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" Has got access  at binding station 2 at time "+to_string(getTime())+"\n";

        ans=1;
        break;
    }
    else
    {
        //means both are busy
        //cout<<"(*******LEADER*****)Leader from group "<<s->get_group_no()<<" is waiting for binding station at time "<<getTime()<<endl;
        pthread_cond_wait(&condition3,&preparingforbinding);
    }
    }
    pthread_mutex_unlock(&preparingforbinding);
    //now binding
    pthread_mutex_lock(&binding_station[ans]);
    binding_station_state[ans]=1;
    cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" is binding at binding station "<<ans+1<<" at time "<<getTime()<<endl;
    file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" is binding at binding station "+to_string(ans+1)+" at time "+to_string(getTime())+"\n";


    int delay=generateRandomNumber(5,10);
    sleep(7*time_for_binding);

    cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" has finished binding at binding station "<<ans+1<<" at time "<<getTime()<<endl;
    file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" has finished binding at binding station "+to_string(ans+1)+" at time "+to_string(getTime())+"\n";

    pthread_mutex_unlock(&binding_station[ans]);

    pthread_mutex_lock(&finishedbinding);//lock to protoect cond variable
    binding_station_state[ans]=-1;
    cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" has finished binding at  "<<getTime()<<endl;
    file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" has finished binding at  "+to_string(getTime())+"\n";


    pthread_cond_signal(&condition3);
    pthread_mutex_unlock(&finishedbinding);
    
    //now all of them are done binding and prepared for the submission
    pthread_mutex_lock(&entrybook);
    cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" has arrived at the library  at time "<<getTime()<<endl;
    file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" has arrived at the library  at time "+to_string(getTime())+"\n";

    while(readers_count>0)
    {
        cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" is waiting for the entry book at time "<<getTime()<<endl;
        file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" is waiting for the entry book at time "+to_string(getTime())+"\n";

        pthread_cond_wait(&condition4,&entrybook);
    }
    cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" has opened the entry book at time "<<getTime()<<endl;
    file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" has opened the entry book at time "+to_string(getTime())+"\n";

    sleep(generateRandomNumber(2,5));
    total_submission++;
    cout<<"LEADER:::Leader from group "<<s->get_group_no()<<" has closed the entry book at time "<<getTime()<<endl;
    file_ans+="LEADER:::Leader from group "+to_string(s->get_group_no())+" has closed the entry book at time "+to_string(getTime())+"\n";

    pthread_mutex_unlock(&entrybook);



    

    return NULL;
}
void* simulatelibrary(void *ptr)
{
    //this is the library thread
    int person=*(int*)ptr;

    //cout<<"---------Library--------!Reader  "<<person<<" has arrived at the library"<<endl;
            while(1)
            {
                int random=generateRandomNumber(10,15);
                sleep(random);
                //generating some random delay before one can read the book
                sem_wait(&readers_sem);
                //
               //cout<<"Library:::Staff  "<<person<<" has arrived at the entry book at time "<<getTime()<<endl;

                pthread_mutex_lock(&entrybook);

                readers_count++;//keeping thee reader count
                cout<<"Library::: Staff  "<<person<<" has opened  the entry book at time "<<getTime()<<"  no of submission = "<<total_submission<<endl;
                file_ans+="Library::: Staff  "+to_string(person)+" has opened  the entry book at time "+to_string(getTime())+"  no of submission = "+to_string(total_submission)+"\n";

                pthread_mutex_unlock(&entrybook);

                sleep(3*time_for_reading);
            

                pthread_mutex_lock(&entrybook);
                readers_count--;

                if(total_submission==no_of_groups)
                {
                    cout<<"Library::: Staff  "<<person<<" has Received  submission from all the group and exited  the library at time "<<getTime()<<endl;
                    file_ans+="Library::: Staff  "+to_string(person)+" has Received  submission from all the group and exited  the library at time "+to_string(getTime())+"\n";

                    pthread_mutex_unlock(&entrybook);
                    //break;
                    return NULL;
                }

                if(readers_count==0)
                {
                    //has to give a available signal
                    //cout<<"###########Library######## Reader  "<<person<<" is the giving the available signal at time "<<getTime()<<endl;
                pthread_cond_signal(&condition4);
                }

                pthread_mutex_unlock(&entrybook);


                cout<<"Library:::Staff  "<<person<<" has closed  the entry book at time "<<getTime()<<endl;
                file_ans+= "Library:::Staff  "+to_string(person)+" has closed  the entry book at time "+to_string(getTime())+"\n";

                //up the readers semaphore
                sem_post(&readers_sem);


            }
            return NULL;

}

int main()
{
    //redirecting input
    freopen("in.txt","r",stdin);
    freopen("out.txt","w",stdout);
    int n,m,w,x,y;
    cin>>n>>m;
    cin>>w>>x>>y;
    total_std=n;
    // cout<<"Number of students = "<<n<<endl;
    // cout<<"Number of Students in each group = "<<m<<endl;
    // cout<<"Relative time unit for printing= "<<w<<endl;
    // cout<<"Relative time unit for binding = "<<x<<endl;
    // cout<<"Relative time unit for reading/writing = "<<y<<endl;
    time_for_printing=w;
    time_for_binding=x;
    time_for_reading=y;
    no_of_groups=n/m;
    binder=new int[n/m];
    for(int i=0;i<n/m;i++)
    binder[i]=0;
    student_per_group=m;
    student_thread=new pthread_t[n];
    groupleader_thread=new pthread_t[no_of_groups];
    for(int i=0;i<PRINTING_STATION;i++)
    printing_station_queue[i]=-1;
    //time count starts
    time(&start_time);
    //initializing all semaphores------------------------------------------------------------
    sem_init(&readers_sem,0,MAX_READER);
    //initializing all mutex----------------------------------------------------------------
       for(int i=0;i<PRINTING_STATION;i++)
    pthread_mutex_init(&printing_station[i],NULL);
    pthread_mutex_init(&printing,NULL);
    pthread_mutex_init(&creating,NULL);
    pthread_mutex_init(&waitingforbinding,NULL);
    for(int i=0;i<PRINTING_STATION;i++)
    pthread_mutex_init(&waitingforprinting[i],NULL);
    for(int i=0;i<BINDING_STATION;i++)
    pthread_mutex_init(&binding_station[i],NULL);
    pthread_mutex_init(&preparingforbinding,NULL);
    pthread_mutex_init(&finishedbinding,NULL);
    pthread_mutex_init(&entrybook,NULL);

//initializing all cond_vars---------------------------------------------------------------
//condition 2 is for a group leader checking whether everyone has done printing or not
condition2=new pthread_cond_t[no_of_groups];
for(int i=0;i<no_of_groups;i++)
pthread_cond_init(&condition2[i],NULL);

    for(int i=0;i<PRINTING_STATION;i++)
    pthread_cond_init(&condition1[i],NULL);
    pthread_cond_init(&condition3,NULL);
    pthread_cond_init(&condition4,NULL);
//creating all the students----------------------------------------------------------------    
   vector<student*>allstudents;
    //creating all the students && the threads------------------------------------------------
int group_no=0;
int a=1,b=2;

pthread_create(&library_thread,NULL,simulatelibrary,(void*)&a);
pthread_create(&library_thread,NULL,simulatelibrary,(void*)&b);
vector<int > arr;
for(int i=0;i<n;i++)
arr.push_back(i);
random_shuffle(arr.begin(),arr.end());
// for(int i=0;i<n;i++)
// {
//     cout<<arr.at(i)<<endl;
// }
int i;
    for(int j=0;j<n;j++)
    {
        i=arr.at(j);

        pthread_mutex_lock(&creating);
student *s=new student(i+1,m);
       ;
        int delay=generateRandomNumber(1,2);
        sleep(delay);
        pthread_create(&student_thread[i],NULL,simulateSubmitReport,(void*)s);//the thread will  call the simulateSubmitReport function;
        //also the s will be passed as an argument to that function
        //the thread wiil be stored in the student_thread array
       // s->print_info();
        //for the group leader a new thread will be created.
        if((i+1)%m==0)
        {
            //means he is the group leader
            pthread_create(&groupleader_thread[group_no++],NULL,simulatebinding,(void*)s);//the thread will  call the simulateSubmitReport function;
        }
        pthread_mutex_unlock(&creating);
        
    }
    //initialize mutex
    //0-->PS1
 
//terminating all the threads----------------------------------------------------------------
    int min_val=1;
    int max_val=3;
    double mean=0.5;
    int delay=1;
    for(int i=0;i<n;i++)
    {
        pthread_join(student_thread[i],NULL);//start
       // int delay=generateRandomNumber(1,2);
        //sleep(delay);
        //generating random delay
      //  cout<<"teminated student "<<i+1<<endl;


    }
    for(int i=0;i<no_of_groups;i++)
    {
        pthread_join(groupleader_thread[i],NULL);//start
        //generating random delay
     //   cout<<"teminated group leader "<<i+1<<endl;
 ;

    }
    for(int i=0;i<MAX_READER;i++)
    {
        pthread_join(library_thread,NULL);//start
        //generating random delay
    //    cout<<"teminated library "<<i+1<<endl;
    }
    ofstream nabid("output.txt");
    nabid<<file_ans;
    nabid.close();


}
