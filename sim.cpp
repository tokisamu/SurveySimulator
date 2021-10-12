#include<queue>
#include<iostream>
#include<algorithm>
#include<stdio.h>
#include<string.h>
#include <cstdlib>
#include<fstream>
#define net_size 20000000
#define node_number 200
#define payment_number 100000
#define route_number 4
using namespace std;
int head[net_size];
int vis[net_size];
int p[net_size];
int deletedEdge[net_size];
int re[net_size];
double dist[net_size];
vector<int> G[node_number];
int shortest_path[net_size];
int suc,fail;
int sNum[net_size];
int fNum[net_size];
int subCnt[net_size];
int routingMethod = 0,spider = 0, boom = 0;
int n,m,tot,s,t,paymentSize,totalTime,time;
struct edge
{
    int from;
    int to;
    int next;
    int capacity;
    int maxCapacity;
    int flow = 0;
    float w;
}e[net_size];
struct node
{
    int id;
    int flow;
    node(int aa,int bb)
    {
        id = aa;
        flow = bb;
    }
};
struct payment
{
    int from;
    int to;
    int id;
    int amount;
    int timestamp;
    int maxDelay;
    bool operator < (const payment &a)const
    {
        return timestamp<a.timestamp;
    }
}payments[payment_number];
struct task
{
    int timestamp;
    int nodeID;
    int taskID;
    vector<int> path;
    vector<float> fees;
    float amount;
    float realAmount;
    bool operator < (const task &a)const
    {
        return timestamp>a.timestamp;
    }
};
priority_queue<task> channelThread[node_number];
vector<int> busyChannel[20000]; //store channels have tasks to process, it is a cyclic array

void add(int aa,int bb,int cc)
{
    e[tot].from=aa;
    e[tot].to=bb;
    e[tot].w=0.01;
    e[tot].capacity=cc;
    e[tot].maxCapacity=cc;
    e[tot].flow=0;
    e[tot].next=head[aa];
    head[aa]=tot++;
    e[tot].from=bb;
    e[tot].to=aa;
    e[tot].w=100;
    e[tot].capacity=0;
    e[tot].maxCapacity=0;
    e[tot].flow=0;
    e[tot].next=head[bb];
    head[bb]=tot++;
    G[aa].push_back(tot - 2);
    G[bb].push_back(tot - 1);
}

bool pay(int aa,int bb,int amount)
{
    int next_node;
    for(int i=head[aa];i!=-1;i=e[i].next)
    {
        if(e[i].w==100) continue;
        if(e[i].to==bb)
        {
            //cout<<"old: "<<e[i].capacity<<" "<<amount<<endl;
            if(e[i].capacity-amount<0)
            {
                return false;
            }
            e[i].capacity -= amount;
            if(e[i].capacity<e[i].maxCapacity/2)
            {
                //cout<<"w added"<<endl;
                e[i].w = 0.02;
            }
            //cout<<"cap: "<<e[i].capacity<<" "<<amount<<endl;
            next_node = i;
            break;
        }
    }
    for(int i=head[bb];i!=-1;i=e[i].next)
    {
        if(e[i].w==100) continue;
        if(e[i].to==aa)
        {
            //cout<<"old: "<<e[i].capacity<<" "<<amount<<endl;
            if(e[i].capacity-amount<0)
            {
                e[next_node].capacity += amount;
                return false;
            }
            e[i].capacity += amount;
            if(e[i].capacity<e[i].maxCapacity/2)
            {
                //cout<<"w recover"<<endl;
                e[i].w = 0.01;
            }
            //cout<<"cap: "<<e[i].capacity<<endl;
            break;
        }
    }
    return true;
}

bool cancelPay()
{
    return true;
}

void paymentGenerator(int num)
{
    for(int i=0;i<num;i++)
    {
        int temp1 = rand()%node_number;
        int temp2 = temp1;
        while(temp2==temp1)
            temp2 = rand()%node_number;
        payments[i].from = temp1;
        payments[i].to = temp2;
        //random size payment
        payments[i].amount = 200+rand()%200;
        payments[i].timestamp = rand()%(totalTime-100);
        payments[i].maxDelay = 100;
        payments[i].id = i;
    }

}

void netGenerator(int edgeNumber)
{
    for(int i=0;i<node_number;i++)
    {
        int neighbour[edgeNumber];
        for(int j=0;j<edgeNumber;j++)
        {
            int temp;
            int flag = 1;
            while(flag)
            {
                temp = rand()%200;
                if(temp==i)
                    continue;
                int flag2 = 0;
                for(int k=0;k<j;k++)
                {
                    if(neighbour[k]==temp)
                        flag2 = 1;
                }
                if(flag2==0)
                    break;
            }
            neighbour[j] = temp;
            //random funds in channel 
            int randAmount = 500+rand()%1000;
            add(i,temp,randAmount);
            add(temp,i,randAmount);
        }
    }
}

vector<int>* spfa(int s,int target,int minCap)
{
    vector<int> *v = new vector<int>;
    for(int i=0;i<=node_number;i++)
    {
        vis[i]=0;
        dist[i]=-1;
        shortest_path[i] = -1;
    }
    priority_queue<pair<double,int> > q;
    pair<double,int> now=make_pair(0.0,s);
    dist[s]=0;
    q.push(now);
    while(!q.empty())
    {
        now=q.top();
        q.pop();
        int p=now.second;
        //cout<<p<<endl;
        //vis[p]=1;
        //cout<<p<<' '<<now.first<<endl;
        for(int i=head[p];i!=-1;i=e[i].next)
        {
            if(e[i].w==100||deletedEdge[i]==1) continue;
            if(e[i].capacity<minCap+minCap/10)
                continue;
            double dis=now.first;
            dis=dis+e[i].w;
            if(dis<dist[e[i].to]||dist[e[i].to]==-1)
            {
                shortest_path[e[i].to] = p;
                dist[e[i].to]=dis;
                if(!vis[e[i].to])
                {
                    q.push(make_pair(dis,e[i].to));
                    vis[e[i].to] = 1;
                }
            }
        }
    }
    vector<int> ans;
    if(dist[target]==-1)
        return v;
    int temp = target;
    ans.push_back(temp);
    //cout<<" asd "<<temp<<endl;
    while(shortest_path[temp]!=-1)
    {
        ans.push_back(shortest_path[temp]);
        temp = shortest_path[temp];
    }
    for(int i = ans.size()-1;i>=0;i--)
    {
        v->push_back(ans[i]);
        //cout<<ans[i]<<endl;
    }
    //cout<<"fee is "<<dist[target]<<endl;
    return v;
}

int Maxflow(int s, int t) {
    int flow = 0;
    for(int i=0;i<tot;i++)
        e[i].flow = 0;
    for (;;) {
        memset(re, 0, sizeof(re));
        memset(p, 0, sizeof(p));
        queue<int> Q;
        Q.push(s);
        re[s] = 0X3F3F3F3F;
        while (!Q.empty()) {
            int x = Q.front();
            Q.pop();
            for (int i = 0; i < G[x].size(); i++) {
                edge& ee = e[G[x][i]];
                if (!re[ee.to] && ee.capacity > ee.flow) {
                    p[ee.to] = G[x][i];
                    re[ee.to] = min(re[x], ee.capacity - ee.flow);
                    Q.push(ee.to);
                }
            }
            if (re[t]) break;
        }
        if (!re[t]) break;
        for (int u = t; u != s; u = e[p[u]].from) {
            e[p[u]].flow += re[t];
            e[p[u] ^ 1].flow -= re[t];
        }
        flow += re[t];
    }
    return flow;
}

//transfer a payment to events of different nodes
void routePayment(payment p,vector<int>* path)
{
    vector<float>* fees = new vector<float>;
    task t;
    t.nodeID = p.from;
    t.timestamp = p.timestamp;
    t.path = *path;
    t.amount = p.amount;
    t.realAmount = p.amount;
    t.taskID = p.id;
    double tex  = 0.0;
    for(int nn=0;nn<path->size()-1;nn++)
    {
        for(int i=head[t.path[nn]];i!=-1;i=e[i].next)
        {
            if(e[i].w==100) continue;
            if(e[i].to==t.path[nn+1])
            {
                tex += e[i].w;
                fees->push_back(e[i].w);
                break;
            }
        }
    }
    //cout<<"tex is: "<<tex<<endl;
    t.amount += tex*p.amount;
    t.fees = *fees;
    //cout<<"as:"<<t.taskID<<endl;
    channelThread[p.from].push(t);
    busyChannel[t.timestamp%2000].push_back(t.nodeID);
}
//event handler of nodes
void processRouting(int nodeID)
{
    while(!channelThread[nodeID].empty())
    {
        task t = channelThread[nodeID].top();
        if(t.timestamp>time)
            break;
        channelThread[nodeID].pop();
        int nextNode = -1;
        int finish = 0;
        int cnt = 0;
        float tex = 0;
        for(int i=0;i<t.path.size();i++)
        {
            if(t.path[i]==nodeID)
            {
                cnt = i;
                if(i==t.path.size()-1)
                {
                    finish  = 1;
                    //cout<<"asd "<<payments[t.taskID].from<<" "<<payments[t.taskID].amount<<" "<<subCnt[t.taskID]<<" "<<t.realAmount<<endl;
                    payments[t.taskID].amount-=t.realAmount;
                    if(payments[t.taskID].amount<0)
                    {
                        cout<<"Cancel Redundant Packet! node "<<nodeID<<" in "<<time<<" size "<<-payments[t.taskID].amount<<endl;
                        t.amount = -payments[t.taskID].amount;
                        for(;cnt>0;cnt--)
                        {
                            pay(t.path[cnt],t.path[cnt-1],t.amount+t.amount*t.fees[cnt-1]);
                        }
                        payments[t.taskID].amount = 0;
                    }
                    if(payments[t.taskID].amount==0&&subCnt[t.taskID]>-9999)
                    {
                        subCnt[t.taskID] = -9999;
                        suc++;
                        cout<<"Payment Finish ! node "<<nodeID<<" in "<<time<<" size "<<t.amount<<endl;
                        sNum[time] = suc;
                        fNum[time] = fail;
                    }
                }
                nextNode = t.path[i+1];
                tex = t.fees[i];
                break;
            }
        }
        if(nextNode==-1||finish)
            continue;

        //redundant check
        int flag = 1;
        for(int i=head[nodeID];i!=-1;i=e[i].next)
        {
            if(e[i].w==100) continue;
            if(e[i].to==nextNode)
            {
                flag = 0;
                break;
            }
        }
        if(flag)
            continue;

        //simulate the HTLC
        bool success = pay(nodeID,nextNode,t.amount);
        //handle failed payments
        if(!success)
        {
            cout<<"failed: "<<t.taskID<<endl;
            if(subCnt[t.taskID]!=-1)
            {
                fail++;
                sNum[time] = suc;
                fNum[time] = fail;
                cout<<"Payment failed ! node "<<nodeID<<" in "<<time<<endl;
                fNum[t.taskID] = fail;
                subCnt[t.taskID] = -1;
            }
            for(;cnt>0;cnt--)
            {
                pay(t.path[cnt],t.path[cnt-1],t.amount+t.amount*t.fees[cnt-1]);
            }
            continue;
        }
        //pass the payment to next node in the path
        task t2;
        t2.nodeID = nextNode;
        //random delay between channels
        t2.timestamp = time + 2+rand()%10;
        t2.path = t.path;
        t2.taskID = t.taskID;
        t2.realAmount = t.realAmount;
        t2.fees = t.fees;
        t2.amount = t.amount-t.realAmount*tex;
        channelThread[t2.nodeID].push(t2);
        busyChannel[t2.timestamp%2000].push_back(t2.nodeID);
        //cout<<nodeID<<" "<<t2.nodeID<<" "<<t2.amount<<" "<<t.amount<<" "<<time<<endl;
    }
}
void processPayment()
{
    int cnt = 0;
    for(time = 0;time<totalTime;time++)
    {
        //initialize stored payments
        while(time==payments[cnt].timestamp)
        {
            //to be completed, should compatible with multiple routing plan
            cout<<"from "<<payments[cnt].from<<" to "<<payments[cnt].to<<" size "<<payments[cnt].amount<<endl;
            //shortest path
            if(routingMethod==0)
            {
                vector<int> *path = spfa(payments[cnt].from,payments[cnt].to,payments[cnt].amount+ceil(payments[cnt].amount*0.1));
                if(path->size()==0)
                {
                    fail++;
                    sNum[time] = suc;
                    fNum[time] = fail;
                    cout<<"failed"<<endl;
                }
                else
                {
                    subCnt[cnt]++;
                    payments[cnt].id = cnt;
                    routePayment(payments[cnt],path);
                }
            }
            else if(routingMethod==1) //disjoined max flow
            {
                int maxflow = Maxflow(payments[cnt].from,payments[cnt].to);
                cout<<"max flow is "<<maxflow<<endl;
                //print max flow path
                vector<node> flowGraph[node_number];
                for(int nodeID=0;nodeID<node_number;nodeID++)
                {
                    for(int i=head[nodeID];i!=-1;i=e[i].next)
                    {
                        if(e[i].w==100) continue;
                        if(e[i].flow)
                        {
                            flowGraph[nodeID].push_back(node(e[i].to,e[i].flow));
                            //cout<<"from "<<e[i].from<<" to "<<e[i].to<<" : "<<e[i].flow<<endl;
                        }
                    }
                }
                //create disjoint widest paths and routing payments
                int totalFlow = 0;
                for(int subpath=0;subpath<route_number;subpath++)
                {
                    vector<int>* path1 = new vector<int>;
                    int now = payments[cnt].from;
                    int flow1 = 99999999;
                    path1->push_back(payments[cnt].from);
                    while(now!=payments[cnt].to)
                    {
                        int maxEdge = 0;
                        int nextNode = -1;
                        int temp = -1;
                        int haha = 0;
                        for(int i=0;i<flowGraph[now].size();i++)
                        {
                            if(flowGraph[now][i].flow>maxEdge)
                            {
                                maxEdge = flowGraph[now][i].flow;
                                nextNode = flowGraph[now][i].id;
                                temp = i;
                                haha = 1;
                            }
                        }
                        flow1 = min(flow1,maxEdge);
                        if(flow1==99999999||haha==0)
                        {
                            fail++;
                            sNum[time] = suc;
                            fNum[time] = fail;
                            cout<<payments[cnt].id<<" failed"<<endl;
                            flow1 = 99999999;
                            break;
                        }
                        flowGraph[now][temp].flow = 0;
                        now = nextNode;
                        path1->push_back(nextNode);
                    }
                    if(flow1 == 99999999) 
                    {
                        flow1 = 0;
                        break;
                    }
                    //keep money for fees
                    if(totalFlow==payments[cnt].amount)
                        break;
                    flow1 = flow1*0.9;
                    if(totalFlow+flow1>payments[cnt].amount)
                    {
                        flow1 = payments[cnt].amount-totalFlow;
                        totalFlow += flow1;
                    }
                    else totalFlow+=flow1;
                    cout<<"flow"<<subpath<<" is : "<<flow1<<endl;
                    payment subPayment;
                    subPayment.from = payments[cnt].from;
                    subPayment.to = payments[cnt].to;
                    subPayment.amount = flow1;
                    subPayment.id = cnt;
                    subCnt[cnt]++;
                    subPayment.timestamp = payments[cnt].timestamp;
                    subPayment.maxDelay = payments[cnt].maxDelay;

                    routePayment(subPayment,path1);
                    if(totalFlow == payments[cnt].amount)
                        break;
                }
            }
            else if(routingMethod==2) //boom
            {
                int maxflow = Maxflow(payments[cnt].from,payments[cnt].to);
                cout<<"max flow is "<<maxflow<<endl;
                //print max flow path
                vector<node> flowGraph[node_number];
                for(int nodeID=0;nodeID<node_number;nodeID++)
                {
                    for(int i=head[nodeID];i!=-1;i=e[i].next)
                    {
                        if(e[i].w==100) continue;
                        if(e[i].flow)
                        {
                            flowGraph[nodeID].push_back(node(e[i].to,e[i].flow));
                            //cout<<"from "<<e[i].from<<" to "<<e[i].to<<" : "<<e[i].flow<<endl;
                        }
                    }
                }
                //create disjoint widest paths and routing payments
                int totalFlow = 0;
                vector<int>* path0 = new vector<int>;
                vector<int>* path2 = new vector<int>;
                vector<int>* path3 = new vector<int>;
                vector<int>* path4 = new vector<int>;
                int subFlow[5] = {0,0,0,0,0};
                vector<int>* paths[4] = {path0,path2,path3,path4};
                for(int subpath=0;subpath<4;subpath++)
                {
                    int now = payments[cnt].from;
                    int flow1 = 99999999;
                    paths[subpath]->push_back(payments[cnt].from);
                    while(now!=payments[cnt].to)
                    {
                        int maxEdge = 0;
                        int nextNode = -1;
                        int temp = -1;
                        for(int i=0;i<flowGraph[now].size();i++)
                        {
                            if(flowGraph[now][i].flow>maxEdge)
                            {
                                maxEdge = flowGraph[now][i].flow;
                                nextNode = flowGraph[now][i].id;
                                temp = i;
                            }
                        }
                        if(maxEdge==0) break;
                        flow1 = min(flow1,maxEdge);
                        if(flow1==99999999) break;
                        flowGraph[now][temp].flow = 0;
                        now = nextNode;
                        paths[subpath]->push_back(nextNode);
                    }
                    if(flow1 == 99999999) flow1 = 0;
                    subFlow[subpath] = flow1;
                    //keep money for fees

                    /*
                    payment subPayment;
                    subPayment.from = payments[cnt].from;
                    subPayment.to = payments[cnt].to;
                    subPayment.amount = flow1;
                    subPayment.timestamp = payments[cnt].timestamp;
                    subPayment.maxDelay = payments[cnt].maxDelay;

                    routePayment(subPayment,path1);
                    if(totalFlow == payments[cnt].amount)
                        break;
                    */
                }
                int payCnt = 0;
                int validPath[4] = {-1,-1,-1,-1};
                for(int i=0;i<4;i++)
                {
                    if(subFlow[i]>0)
                        validPath[payCnt++] = i;
                    totalFlow+=subFlow[i];
                    cout<<"flow is "<<subFlow[i]<<endl;
                }
                int needed = 0;
                pair<int,int> orderedFlow[5] = {make_pair(subFlow[0],0),make_pair(subFlow[1],1),make_pair(subFlow[2],2),make_pair(subFlow[3],3),make_pair(subFlow[4],4)};
                sort(orderedFlow,orderedFlow+5, greater<pair<int,int>>());
                int waterFlow[5]={0,0,0,0,0};
                int redundantPacket = max(50,int(payments[cnt].amount*0.1)/50);
                payments[cnt].amount+=redundantPacket;
                int flag1 = 0;
                for(int i=0;i<payCnt;i++)
                {
                    int water = 0;
                    int dif = 0;
                    int sum = 0;
                    for(int j=0;j<=i;j++)
                    {
                       water += (orderedFlow[j].first-orderedFlow[i+1].first);
                       if(j>0) dif+= (orderedFlow[j-1].first-orderedFlow[i].first);
                    }
                    if(i==payCnt-1&&water>=payments[cnt].amount&&water<payments[cnt].amount-redundantPacket)
                    {
                        payments[cnt].amount-=redundantPacket;
                        redundantPacket = 0;
                    }
                    if(water>=payments[cnt].amount-redundantPacket)
                    {
                        flag1 = 1;
                        dif = orderedFlow[i].first-(payments[cnt].amount-dif)/(i+1);
                        for(int j=0;j<=i;j++)
                        {
                            if(j==i)
                            {
                                waterFlow[j] = payments[cnt].amount - sum;
                                continue;
                            }
                           waterFlow[j] = orderedFlow[j].first- dif;
                           sum += waterFlow[j];
                        }
                        for(int j=0;j<5;j++)
                        {
                            subFlow[orderedFlow[j].second] = waterFlow[j];
                            cout<<"waterflow "<<waterFlow[j]<<endl;
                        }
                        break;
                    }
                }
                if(flag1==0)
                {
                    fail++;
                    sNum[time] = suc;
                    fNum[time] = fail;
                    cout<<payments[cnt].id<<" failed"<<endl;
                }
                else
                {
                    for(int i=0;i<payCnt;i++)
                    {
                        int sendAmount = 0;
                        sendAmount = subFlow[i];
                        if(sendAmount==0) continue;
                        int res = sendAmount;
                        for(int packet = 0;packet<sendAmount/50+1;packet++)
                        {
                            if(res==0) break;
                            payment subPayment;
                            subPayment.from = payments[cnt].from;
                            subPayment.to = payments[cnt].to;
                            if(res<50)
                                subPayment.amount = res;
                            else subPayment.amount = 50;
                            res -= subPayment.amount;
                            subPayment.id = cnt;
                            subCnt[subPayment.id]++;
                            subPayment.timestamp = payments[cnt].timestamp+packet*2;
                            subPayment.maxDelay = payments[cnt].maxDelay;
                            cout<<"flow"<<i<<" packet "<<packet<<" :"<<subFlow[validPath[i]]<<" routed: "<<subPayment.amount<<endl;
                            routePayment(subPayment,paths[validPath[i]]);
                        }
                    }
                    payments[cnt].amount-=redundantPacket;
                    subCnt[payments[cnt].id]-=redundantPacket/50;
                }
            }
            else if(routingMethod==3) //heuristic shortest path
            {
                //k times heuristic shortest path
                double fees[4] = {0.0,0.0,0.0,0.0};
                vector<int> *cache[20];
                int failFlag = 0;
                for(int k = 0;k<4;k++)
                {
                    memset(deletedEdge,0,sizeof(deletedEdge));
                    int res  = payments[cnt].amount;
                    for(int subShortest = 0;subShortest<=k;subShortest++)
                    {
                        vector<int> *path;
                        if(subShortest<k-1)
                        {
                            path = spfa(payments[cnt].from,payments[cnt].to,payments[cnt].amount/(k+1));
                            res -= payments[cnt].amount/(k+1);
                        }
                        else path = spfa(payments[cnt].from,payments[cnt].to,res);
                        cache[k*4 + subShortest] = path;
                        res -= payments[cnt].amount/(k+1);
                        fees[k]+=dist[payments[cnt].to]/(double)(k+1);
                        if(path->size()==0)
                        {
                            fail++;
                            sNum[time] = suc;
                            fNum[time] = fail;
                            failFlag = 1;
                            cout<<k<<" failed"<<endl;
                            fees[k] = 99999999;
                            break;
                        }
                        //delete visited edge
                        for(int j = 0;j<path->size()-1;j++)
                        {
                            //cout<<path->at(j)<<" ";
                            for(int i=head[path->at(j)];i!=-1;i=e[i].next)
                            {
                                if(e[i].w==100) continue;
                                if(e[i].to==path->at(j+1))
                                {
                                    deletedEdge[i] = 1;
                                    break;
                                }
                            }
                        }
                        //cout<<endl;
                    }
                    if(failFlag) break;
                }
                if(!failFlag)
                {
                    double min = 9999;
                    int mink = -1;
                    for(int k = 0;k<4;k++)
                    {
                        if(fees[k]<min)
                        {
                            min = fees[k];
                            mink = k;
                        }
                    }
                    memset(deletedEdge,0,sizeof(deletedEdge));
                    int res  = payments[cnt].amount;
                    for(int subShortest = 0;subShortest<=mink;subShortest++)
                    {
                        vector<int> *path;
                        int subSize = 0;
                        if(subShortest<mink)
                        {
                            //path = spfa(payments[cnt].from,payments[cnt].to,payments[cnt].amount/(mink+1));
                            path = cache[mink*4+subShortest];
                            subSize = payments[cnt].amount/(mink+1);
                        }
                        else
                        {
                            //path = spfa(payments[cnt].from,payments[cnt].to,res);
                            path = cache[mink*4+subShortest];
                            subSize = res;
                        }
                        res -= subSize;
                        payment subPayment;
                        subPayment.from = payments[cnt].from;
                        subPayment.to = payments[cnt].to;
                        subPayment.id = cnt;
                        subCnt[subPayment.id]++;
                        subPayment.amount = subSize;
                        subPayment.timestamp = payments[cnt].timestamp;
                        subPayment.maxDelay = payments[cnt].maxDelay;
                        cout<<"path"<<subShortest<<" routed: "<<subPayment.amount<<endl;
                        routePayment(subPayment,path);
                    }
                }
            }
            else if(routingMethod==4) //spider water
            {
                int maxflow = Maxflow(payments[cnt].from,payments[cnt].to);
                cout<<"max flow is "<<maxflow<<endl;
                //print max flow path
                vector<node> flowGraph[node_number];
                for(int nodeID=0;nodeID<node_number;nodeID++)
                {
                    for(int i=head[nodeID];i!=-1;i=e[i].next)
                    {
                        if(e[i].w==100) continue;
                        if(e[i].flow)
                        {
                            flowGraph[nodeID].push_back(node(e[i].to,e[i].flow));
                            //cout<<"from "<<e[i].from<<" to "<<e[i].to<<" : "<<e[i].flow<<endl;
                        }
                    }
                }
                //create disjoint widest paths and routing payments
                int totalFlow = 0;
                vector<int>* path0 = new vector<int>;
                vector<int>* path2 = new vector<int>;
                vector<int>* path3 = new vector<int>;
                vector<int>* path4 = new vector<int>;
                int subFlow[5] = {0,0,0,0,0};
                vector<int>* paths[4] = {path0,path2,path3,path4};
                for(int subpath=0;subpath<4;subpath++)
                {
                    int now = payments[cnt].from;
                    int flow1 = 99999999;
                    paths[subpath]->push_back(payments[cnt].from);
                    while(now!=payments[cnt].to)
                    {
                        int maxEdge = 0;
                        int nextNode = -1;
                        int temp = -1;
                        for(int i=0;i<flowGraph[now].size();i++)
                        {
                            if(flowGraph[now][i].flow>maxEdge)
                            {
                                maxEdge = flowGraph[now][i].flow;
                                nextNode = flowGraph[now][i].id;
                                temp = i;
                            }
                        }
                        if(maxEdge==0) break;
                        flow1 = min(flow1,maxEdge);
                        if(flow1==99999999) break;
                        flowGraph[now][temp].flow = 0;
                        now = nextNode;
                        paths[subpath]->push_back(nextNode);
                    }
                    if(flow1 == 99999999) flow1 = 0;
                    subFlow[subpath] = flow1;
                    //keep money for fees

                    /*
                    payment subPayment;
                    subPayment.from = payments[cnt].from;
                    subPayment.to = payments[cnt].to;
                    subPayment.amount = flow1;
                    subPayment.timestamp = payments[cnt].timestamp;
                    subPayment.maxDelay = payments[cnt].maxDelay;

                    routePayment(subPayment,path1);
                    if(totalFlow == payments[cnt].amount)
                        break;
                    */
                }
                int payCnt = 0;
                int validPath[4] = {-1,-1,-1,-1};
                for(int i=0;i<4;i++)
                {
                    if(subFlow[i]>0)
                        validPath[payCnt++] = i;
                    totalFlow+=subFlow[i];
                    cout<<"flow is "<<subFlow[i]<<endl;
                }
                int needed = 0;
                pair<int,int> orderedFlow[5] = {make_pair(subFlow[0],0),make_pair(subFlow[1],1),make_pair(subFlow[2],2),make_pair(subFlow[3],3),make_pair(subFlow[4],4)};
                sort(orderedFlow,orderedFlow+5, greater<pair<int,int>>());
                int waterFlow[5]={0,0,0,0,0};
                int flag1 = 0;
                for(int i=0;i<payCnt;i++)
                {
                    int water = 0;
                    int dif = 0;
                    int sum = 0;
                    for(int j=0;j<=i;j++)
                    {
                       water += (orderedFlow[j].first-orderedFlow[i+1].first);
                       if(j>0) dif+= (orderedFlow[j-1].first-orderedFlow[i].first);
                    }
                    if(water>=payments[cnt].amount)
                    {
                        flag1 = 1;
                        dif = orderedFlow[i].first-(payments[cnt].amount-dif)/(i+1);
                        for(int j=0;j<=i;j++)
                        {
                            if(j==i)
                            {
                                waterFlow[j] = payments[cnt].amount - sum;
                                continue;
                            }
                           waterFlow[j] = orderedFlow[j].first- dif;
                           sum += waterFlow[j];
                        }
                        for(int j=0;j<5;j++)
                        {
                            subFlow[orderedFlow[j].second] = waterFlow[j];
                            cout<<"waterflow "<<waterFlow[j]<<endl;
                        }
                        break;
                    }
                }
                if(flag1==0)
                {
                    fail++;
                    sNum[time] = suc;
                    fNum[time] = fail;
                    cout<<payments[cnt].id<<" failed"<<endl;
                }
                else
                {
                    for(int i=0;i<payCnt;i++)
                    {
                        int sendAmount = 0;
                        sendAmount = subFlow[i];
                        if(sendAmount==0) continue;
                        int res = sendAmount;
                        for(int packet = 0;packet<sendAmount/50+1;packet++)
                        {
                            if(res==0) break;
                            payment subPayment;
                            subPayment.from = payments[cnt].from;
                            subPayment.to = payments[cnt].to;
                            if(res<50)
                                subPayment.amount = res;
                            else subPayment.amount = 50;
                            res -= subPayment.amount;
                            subPayment.id = cnt;
                            subCnt[subPayment.id]++;
                            subPayment.timestamp = payments[cnt].timestamp+packet*2;
                            subPayment.maxDelay = payments[cnt].maxDelay;
                            cout<<"flow"<<i<<" packet "<<packet<<" :"<<subFlow[validPath[i]]<<" routed: "<<subPayment.amount<<endl;
                            routePayment(subPayment,paths[validPath[i]]);
                        }
                    }
                }
            }
            else if(routingMethod==5) //spider
            {
                int maxflow = Maxflow(payments[cnt].from,payments[cnt].to);
                cout<<"max flow is "<<maxflow<<endl;
                //print max flow path
                vector<node> flowGraph[node_number];
                for(int nodeID=0;nodeID<node_number;nodeID++)
                {
                    for(int i=head[nodeID];i!=-1;i=e[i].next)
                    {
                        if(e[i].w==100) continue;
                        if(e[i].flow)
                        {
                            flowGraph[nodeID].push_back(node(e[i].to,e[i].flow));
                            //cout<<"from "<<e[i].from<<" to "<<e[i].to<<" : "<<e[i].flow<<endl;
                        }
                    }
                }
                //create disjoint widest paths and routing payments
                int totalFlow = 0;
                vector<int>* path0 = new vector<int>;
                vector<int>* path2 = new vector<int>;
                vector<int>* path3 = new vector<int>;
                vector<int>* path4 = new vector<int>;
                int subFlow[4] = {0,0,0,0};
                vector<int>* paths[4] = {path0,path2,path3,path4};
                for(int subpath=0;subpath<4;subpath++)
                {
                    int now = payments[cnt].from;
                    int flow1 = 99999999;
                    paths[subpath]->push_back(payments[cnt].from);
                    while(now!=payments[cnt].to)
                    {
                        int maxEdge = 0;
                        int nextNode = -1;
                        int temp = -1;
                        for(int i=0;i<flowGraph[now].size();i++)
                        {
                            if(flowGraph[now][i].flow>maxEdge)
                            {
                                maxEdge = flowGraph[now][i].flow;
                                nextNode = flowGraph[now][i].id;
                                temp = i;
                            }
                        }
                        if(maxEdge==0) break;
                        flow1 = min(flow1,maxEdge);
                        flowGraph[now][temp].flow = 0;
                        now = nextNode;
                        paths[subpath]->push_back(nextNode);
                    }
                    if(flow1 == 99999999) flow1 = 0;
                    subFlow[subpath] = flow1;
                    //keep money for fees

                    /*
                    payment subPayment;
                    subPayment.from = payments[cnt].from;
                    subPayment.to = payments[cnt].to;
                    subPayment.amount = flow1;
                    subPayment.timestamp = payments[cnt].timestamp;
                    subPayment.maxDelay = payments[cnt].maxDelay;

                    routePayment(subPayment,path1);
                    if(totalFlow == payments[cnt].amount)
                        break;
                    */
                }
                int payCnt = 0;
                int validPath[4] = {-1,-1,-1,-1};
                for(int i=0;i<4;i++)
                {
                    if(subFlow[i]>0)
                        validPath[payCnt++] = i;
                    totalFlow+=subFlow[i];
                }
                int flowAssign[4] = {0,0,0,0};
                int res = payments[cnt].amount;
                if(res>totalFlow)
                {
                    fail++;
                    sNum[time] = suc;
                    fNum[time] = fail;
                    cout<<payments[cnt].id<<" failed"<<endl;
                }
                else
                {
                    for(int i=0;i<payCnt;i++)
                    {
                        int sendAmount = 0;
                        if(i==payCnt-1)
                        {
                            sendAmount = res;
                        }
                        else sendAmount =payments[cnt].amount*((double)subFlow[validPath[i]]/(double)totalFlow);
                        for(int packet = 0;packet<sendAmount/50+1;packet++)
                        {
                            if(res==0) break;
                            if(sendAmount==0) continue;
                            payment subPayment;
                            subPayment.from = payments[cnt].from;
                            subPayment.to = payments[cnt].to;
                            subPayment.id = cnt;
                            subCnt[subPayment.id]++;
                            if(res<50)
                                subPayment.amount = res;
                            else subPayment.amount = 50;
                            res -= subPayment.amount;
                            subPayment.timestamp = payments[cnt].timestamp+packet*2;
                            subPayment.maxDelay = payments[cnt].maxDelay;
                            cout<<"flow"<<i<<" packet "<<packet<<" :"<<subFlow[validPath[i]]<<" routed: "<<subPayment.amount<<endl;
                            routePayment(subPayment,paths[validPath[i]]);
                        }
                    }
                }

            }
            //cout<<payments[cnt].timestamp<<endl;
            cnt++;
        }
        //check if some channels have payments to handle now
        if(busyChannel[time%2000].size()>0)
        {
            for(int i=0;i<busyChannel[time%2000].size();i++)
            {
                processRouting(busyChannel[time%2000][i]);
            }
            //cout<<(busyChannel[time%2000].size())<<endl;
            //clear task queue
            vector<int>().swap(busyChannel[time%2000]);
            //cout<<(busyChannel[time%2000].size())<<endl;
        }
        sNum[time] = suc;
        fNum[time] = fail;

    }
}
int main()
{
    //3 1 4 2
    int t = 10;
    int testSize []= {200,400,800,1600,3200,6400};
    int testMethod []= {3,1,4,2};
    for(int t=0;t<6;t++)
    {
        for(int tt=0;tt<4;tt++)
        {
            time = 0;
            routingMethod = testMethod[tt]; //0 spfa, 1 max flow, 2 boom, 3  heuristic, 4 spider water, 5 package
            boom = 0;
            spider = 0;
            memset(head,-1,sizeof(head));
            memset(subCnt,0,sizeof(subCnt));
            memset(sNum,0,sizeof(sNum));
            memset(fNum,0,sizeof(fNum));
            memset(p,0,sizeof(p));
            memset(deletedEdge,0,sizeof(deletedEdge));
            memset(re,0,sizeof(re));
            memset(dist,0,sizeof(dist));
            memset(shortest_path,0,sizeof(shortest_path));
            for(int i=0;i<node_number;i++)
            {
                vector<int>().swap(G[i]);
                if(i<20000)
                    vector<int>().swap(busyChannel[i]);
            }
            suc=0;
            fail=0;
            tot=0;
            netGenerator(4);
            //200 400 800 1600
            paymentSize = testSize[t];
            totalTime = 1100;
            paymentGenerator(paymentSize);
            sort(payments,payments+paymentSize);
            processPayment();
            //spfa(2,100);
            ofstream outfile("Method"+to_string(routingMethod)+"Size"+to_string(paymentSize)+"cnt.txt");
            ofstream outfile2("Method"+to_string(routingMethod)+"Size"+to_string(paymentSize)+"rate.txt");
            for(int i=1;i<totalTime;i++)
            {
                if(sNum[i]==0)
                    sNum[i] = sNum[i-1];
                if(fNum[i]==0)
                    fNum[i] = fNum[i-1];
                if(sNum[i]==0)
                {
                    outfile<<0<<endl;
                    outfile2<<0<<endl;
                }
                else
                {
                    outfile<<sNum[i]<<endl;
                    outfile2<<double(sNum[i])/double(sNum[i]+fNum[i])<<endl;
                }
            }
            cout<<suc<<" / "<<suc+fail<<endl;
            outfile.close();
        }
    }
    return 0;
}
