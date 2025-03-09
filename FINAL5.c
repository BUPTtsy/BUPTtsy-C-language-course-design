#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_NAME_LEN 51
#define MAX_FOOD_NUMBER 100
#define MAX_PACKAGE_FOOD_NUMBER 5
#define MAX_PACKAGES_NUMBER 100
#define MAX_ORDER_NUMBER 54001
 //全过
typedef struct _Food Food;
 
char* ignoreBlank(char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    return p;
}
 
int getTimeInt(int h, int m, int s) {
    return h * 3600 + m * 60 + s;
}
 
void getTime(int t, int* h, int* m, int* s) {
    *h = t / 3600;
    *m = (t % 3600) / 60;
    *s = t % 60;
}
 
typedef enum {
    unarrive,
    arrive,
    complete,
    fail
}OrderStatus;
 
typedef struct _Order
{
    char name[MAX_NAME_LEN];
    int time;
    int endt;
    OrderStatus status;
 
    Food* food[MAX_PACKAGE_FOOD_NUMBER];
    int foodsNum;
}Order;
 
typedef struct _Element {
    Order* order;
    struct _Element* next;
}Element;
 
typedef struct _Queue {
    Element* front;
    Element* rear;
    int size;
}Queue;
 
void initQueue(Queue* q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}
 
void pushBack(Queue* q, Order* order) {
    Element* ele = ( Element*)malloc(sizeof(Element));
    ele->order = order;
    ele->next = NULL;
    if(q->size == 0) {
        q->front = q->rear = ele;
    }
    else {
        q->rear->next = ele;
        q->rear = q->rear->next;
    }
    q->size++;
}
 
void popFront(Queue* q, Order** order) {
    if(q->size == 0) return;
    Element* p;
    if(q->size == 1) {
        p = q->front;
        *order = p->order;
        q->front = NULL;
        q->rear = NULL;
    }
    else {
        p = q->front;
        *order = p->order;
        q->front = q->front->next;
    }
    free(p);
    q->size--;
}
 
typedef struct _Food
{
    char name[MAX_NAME_LEN];
    int store;
    int capacity;
    int produce;
    int remain;
    Queue queue;
}Food;
 
typedef struct _Package
{
    char name[MAX_NAME_LEN];
    Food* food[10];
    int num;
}Package;
 
Package packages[MAX_PACKAGES_NUMBER];
Food foods[MAX_FOOD_NUMBER];
Order orders[MAX_ORDER_NUMBER];
 
int ordersNum;
int orderIndex = 0;
 
int currentTimeInt;
int endTimeInt;
int stopTimeInt;
 
 
int N,M,W1,W2;
 
int findPackageByName(const char* name) {
    for (int i = 0; i < M; ++i) {
        if (strcmp(packages[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}
 
int pending = 0;
 
int sysrun = 1;
int reopen = 0;
 
void run() {
    //1. check food list, first order first satisfy
    for(int i = 0; i < N; ++i) {
        //(1) first we need produce
        if(foods[i].store < foods[i].capacity) {
            if(foods[i].remain == 1) {
                foods[i].remain = foods[i].produce;
                foods[i].store++;
            }
            else {
                foods[i].remain--;
            }
        }
         
        //(2) then we try to meet the orders' needs
        if(foods[i].store > 0) {
            //we have store
            if(foods[i].queue.size <= 0)//no order needs the food, we ignore it.
                continue;
            foods[i].store--;
            Order* order;
            popFront(&foods[i].queue, &order);
            order->foodsNum--;
            if(order->foodsNum == 0) {
                //the order is finished, we need to set the status of the order
                order->status = complete;
                order->endt = currentTimeInt;
                pending--;
                if (currentTimeInt <= getTimeInt(22, 0, 0) && pending < W2) {
                    reopen = 1;
                }
            }
        }
    }
 
    //2. then we check whether the next orders are arriveved.
    while (orderIndex < ordersNum && currentTimeInt >= orders[orderIndex].time) {
        if (sysrun == 0) {
            //now the order system shutdown, we need reject the order
            orders[orderIndex].status = fail;
        }
        else {
            //system is open for new order
            int imme = 1;   //the order can be meet immediately?
            int foodsnum = 0;
            for (int j = 0; j < orders[orderIndex].foodsNum; ++j) {
                Food* food = orders[orderIndex].food[j];
 
                if (food->store > 0) {
                    //food has store
                    food->store--;
 
                }
                else {
                    //food has no store
                    foodsnum++;
                    imme = 0;
                    pushBack(&food->queue, orders + orderIndex);
                }
            }
            if (imme == 0) {
                //order cannot be meet immediately.
                pending++;
                if (pending > W1) {
                    sysrun = 0;
                }
                orders[orderIndex].foodsNum = foodsnum;
                orders[orderIndex].status = arrive;
            }
            else {
                //order can be meet immediately.
                orders[orderIndex].status = complete;
                orders[orderIndex].endt = currentTimeInt;
            }
        }
        orderIndex++;
    }
}
 
Food* findFoodByName(const char* name) {
    for(int i = 0; i < N; ++i) {
        if(strcmp(foods[i].name, name) == 0) {
            return foods + i;
        }
    }
    return NULL;
}
 
void readMenu()
{
    FILE* file = fopen("dict.dic", "r");
    fscanf(file, "%d %d", &N, &M);
 
    char c;
    for (int i = 0; i < N; i++)
    {
        fscanf(file, "%s", foods[i].name);
        foods[i].store = 0;
    }//2
    for (int i = 0; i < N; i++)
    {
        fscanf(file, "%d", &foods[i].produce);
        foods[i].remain = foods[i].produce;
    }//3
    for (int i = 0; i < N; i++)
    {
        fscanf(file, "%d", &foods[i].capacity);
    }//4
    fscanf(file, "%d %d", &W1, &W2);//5
 
 
    for (int i = 0; i < M; ++i) {
        char line[1024] = { 0 };
        while (1) {
            fgets(line, 1024, file);
             
            char* p1 = ignoreBlank(line);
 
            if (*p1 == '\0') continue;
            else break;
        }
 
        char menu[MAX_NAME_LEN] = { 0 };
 
        char* p = ignoreBlank(line);
 
        while (*p == ' ') ++p;
        sscanf(p, "%s", menu);
        strcpy(packages[i].name, menu);
 
        p += strlen(menu); p = ignoreBlank(p);
        while (1) {
            //find food object by food name
            char foodname[MAX_NAME_LEN] = { 0 };
            if (1 != sscanf(p, "%s", foodname)) {
                break;
            }
            p += strlen(foodname); p = ignoreBlank(p);
            Food* food = findFoodByName(foodname);
            packages[i].food[packages[i].num] = food;
            packages[i].num++;
        }
 
    }
    fclose(file);
}
 
void readinput()
{
    scanf("%d", &ordersNum);
    for (int i = 0; i < ordersNum; ++i) {
        int h, m, s;
        char menu[MAX_NAME_LEN] = { 0 };
        scanf("%d:%d:%d %s", &h, &m, &s, menu);
        orders[i].time = getTimeInt(h, m, s);
        strcpy(orders[i].name, menu);
        orders[i].status = unarrive;
        orders[i].endt = 0;
        Food* food = findFoodByName(menu);
        if (food) {
            //this is a food
            orders[i].foodsNum = 1;
            orders[i].food[0] = food;
        }
        else {
            //this is a combo
            int comboidx = findPackageByName(menu);
            memcpy(orders[i].food, packages[comboidx].food, sizeof(Food*) * packages[comboidx].num);
            orders[i].foodsNum = packages[comboidx].num;
        }
    }
}
 
void printOffTime()
{
    for (int i = 0; i < ordersNum; ++i) {
        if (orders[i].status == complete) {
            int h, m, s;
            getTime(orders[i].endt, &h, &m, &s);
            printf("%02d:%02d:%02d\n", h, m, s);
        }
        else {
            printf("Fail\n");
        }
    }
}
 
int main() {
    currentTimeInt = getTimeInt(7, 0, 0);
 
    endTimeInt = getTimeInt(24, 0, 0);
 
    stopTimeInt = getTimeInt(22, 0, 0);
 
 
    readMenu();
    readinput();
 
 
    while (currentTimeInt < endTimeInt) {
 
        if (currentTimeInt > stopTimeInt) {
            sysrun = 0;
        }
        else if (reopen) {
            reopen = 0;
            sysrun = 1;
        }
 
        currentTimeInt++;
        run();
    }
    printOffTime();
    return 0;
 
}