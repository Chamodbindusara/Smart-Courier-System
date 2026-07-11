#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TrackingNode {
    char status[100];
    char location[100];
    char timestamp[50];
    struct TrackingNode* next;
};

struct Parcel {
    int id;
    char sender[50];
    char receiver[50];
    float weight;
    char source[50];
    char destination[50];
    struct TrackingNode* trackingHead;
    struct Parcel* next;
    struct Parcel* prev;
};

struct CenterNode {
    char name[50];
    float distToNext;
    float distToPrev;
    struct CenterNode* next;
    struct CenterNode* prev;
};

struct QueueNode {
    int parcelID;
    struct QueueNode* next;
};

struct StackNode {
    char center[50];
    struct StackNode* next;
};

struct CompletedNode {
    struct Parcel* parcel;
    struct CompletedNode* next;
};

struct ParcelList {
    struct Parcel* head;
};

struct CenterList {
    struct CenterNode* head;
    struct CenterNode* tail;
    int count;
};

struct Queue {
    struct QueueNode* front;
    struct QueueNode* rear;
};

struct Stack {
    struct StackNode* top;
};

struct CompletedStack {
    struct CompletedNode* top;
};

struct ParcelList parcels = {NULL};
struct CenterList centers = {NULL, NULL, 0};
struct Queue pickupQueue = {NULL, NULL};
struct Queue dispatchQueue = {NULL, NULL};
struct Stack routeStack = {NULL};
struct CompletedStack completed = {NULL};
int demoLoaded = 0;

void printLine() {
    printf("--------------------------------------------------\n");
}

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pressEnter() {
    printf("\nPress Enter to continue...");
    fflush(stdout);
    flushInput();
}

void readLine(char* buf, int size) {
    if (fgets(buf, size, stdin) != NULL) {
        int len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[len - 1] = '\0';
    }
}

int readInt(int* out) {
    if (scanf("%d", out) != 1) {
        flushInput();
        return 0;
    }
    flushInput();
    return 1;
}

int readFloat(float* out) {
    if (scanf("%f", out) != 1) {
        flushInput();
        return 0;
    }
    flushInput();
    return 1;
}

void appendTracking(struct Parcel* p, const char* status, const char* location, const char* timestamp) {
    struct TrackingNode* node = malloc(sizeof(struct TrackingNode));
    strcpy(node->status, status);
    strcpy(node->location, location);
    strcpy(node->timestamp, timestamp);
    node->next = NULL;

    if (p->trackingHead == NULL) {
        p->trackingHead = node;
    } else {
        struct TrackingNode* t = p->trackingHead;
        while (t->next != NULL)
            t = t->next;
        t->next = node;
    }
}

struct Parcel* findParcel(int id) {
    struct Parcel* temp = parcels.head;
    while (temp != NULL) {
        if (temp->id == id)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

void freeTracking(struct TrackingNode* head) {
    while (head != NULL) {
        struct TrackingNode* t = head;
        head = head->next;
        free(t);
    }
}

void detachParcelFromList(int id) {
    struct Parcel* p = findParcel(id);
    if (p == NULL)
        return;

    if (p->prev != NULL)
        p->prev->next = p->next;
    else
        parcels.head = p->next;

    if (p->next != NULL)
        p->next->prev = p->prev;

    p->next = NULL;
    p->prev = NULL;
}

void removeFromQueue(struct Queue* q, int id) {
    struct QueueNode* curr = q->front;
    struct QueueNode* prev = NULL;
    while (curr != NULL) {
        if (curr->parcelID == id) {
            if (prev != NULL)
                prev->next = curr->next;
            else
                q->front = curr->next;
            if (curr == q->rear)
                q->rear = prev;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void pushCompleted(struct Parcel* p) {
    struct CompletedNode* node = malloc(sizeof(struct CompletedNode));
    node->parcel = p;
    node->next = completed.top;
    completed.top = node;
}

void addParcel() {
    struct Parcel* p = malloc(sizeof(struct Parcel));

    printLine();
    printf("  Register New Parcel\n");
    printLine();

    printf("  Parcel ID   : ");
    if (!readInt(&p->id)) {
        printf("  Invalid input.\n");
        free(p);
        pressEnter();
        return;
    }

    if (findParcel(p->id) != NULL) {
        printf("  A parcel with ID %d already exists.\n", p->id);
        free(p);
        pressEnter();
        return;
    }

    printf("  Sender      : "); readLine(p->sender, sizeof(p->sender));
    printf("  Receiver    : "); readLine(p->receiver, sizeof(p->receiver));
    printf("  Weight (kg) : "); readFloat(&p->weight);
    printf("  Source      : "); readLine(p->source, sizeof(p->source));
    printf("  Destination : "); readLine(p->destination, sizeof(p->destination));

    char timestamp[50];
    printf("  Timestamp   : "); readLine(timestamp, sizeof(timestamp));

    p->trackingHead = NULL;
    p->next = NULL;
    p->prev = NULL;

    appendTracking(p, "Registered", p->source, timestamp);

    if (parcels.head == NULL) {
        parcels.head = p;
    } else {
        struct Parcel* temp = parcels.head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = p;
        p->prev = temp;
    }

    printf("\n  Parcel #%d registered successfully.\n", p->id);
    pressEnter();
}

void deleteParcel() {
    int id;
    printLine();
    printf("  Delete Parcel\n");
    printLine();
    printf("  Parcel ID: ");
    if (!readInt(&id)) {
        printf("  Invalid input.\n");
        pressEnter();
        return;
    }

    struct Parcel* p = findParcel(id);
    if (p == NULL) {
        printf("  Parcel #%d not found.\n", id);
        pressEnter();
        return;
    }

    removeFromQueue(&pickupQueue, id);
    removeFromQueue(&dispatchQueue, id);
    detachParcelFromList(id);
    freeTracking(p->trackingHead);
    free(p);

    printf("  Parcel #%d deleted and removed from all queues.\n", id);
    pressEnter();
}

void displayParcels() {
    printLine();
    printf("  Parcel Records\n");
    printLine();

    if (parcels.head == NULL) {
        printf("  No parcels registered.\n");
        pressEnter();
        return;
    }

    struct Parcel* p = parcels.head;
    while (p != NULL) {
        printf("  ID: %d  Sender: %s  Receiver: %s\n", p->id, p->sender, p->receiver);
        printf("  Weight: %.2f kg    %s  -->  %s\n", p->weight, p->source, p->destination);
        printLine();
        p = p->next;
    }
    pressEnter();
}

void addTracking() {
    int id;
    printLine();
    printf("  Add Tracking Update\n");
    printLine();
    printf("  Parcel ID  : ");
    if (!readInt(&id)) {
        printf("  Invalid input.\n");
        pressEnter();
        return;
    }

    struct Parcel* p = findParcel(id);
    if (p == NULL) {
        printf("  Parcel #%d not found.\n", id);
        pressEnter();
        return;
    }

    char status[100], location[100], timestamp[50];
    printf("  Status     : "); readLine(status, sizeof(status));
    printf("  Location   : "); readLine(location, sizeof(location));
    printf("  Timestamp  : "); readLine(timestamp, sizeof(timestamp));

    appendTracking(p, status, location, timestamp);

    printf("  Tracking update added for parcel #%d.\n", id);
    pressEnter();
}

void viewTracking() {
    int id;
    printLine();
    printf("  Tracking History\n");
    printLine();
    printf("  Parcel ID: ");
    if (!readInt(&id)) {
        printf("  Invalid input.\n");
        pressEnter();
        return;
    }

    struct Parcel* p = findParcel(id);
    if (p == NULL) {
        printf("  Parcel #%d not found.\n", id);
        pressEnter();
        return;
    }

    if (p->trackingHead == NULL) {
        printf("  No tracking history for parcel #%d.\n", id);
        pressEnter();
        return;
    }

    printf("\n  History for Parcel #%d (%s -> %s):\n\n", id, p->source, p->destination);
    struct TrackingNode* t = p->trackingHead;
    int step = 1;
    while (t != NULL) {
        printf("  [%d] Status: %s  Location: %s  Time: %s\n",
               step++, t->status, t->location, t->timestamp);
        t = t->next;
    }
    pressEnter();
}

struct CenterNode* findCenter(const char* name) {
    if (centers.head == NULL)
        return NULL;
    struct CenterNode* curr = centers.head;
    do {
        if (strcmp(curr->name, name) == 0)
            return curr;
        curr = curr->next;
    } while (curr != centers.head);
    return NULL;
}

void insertCenterAtEnd(const char* name, float distFromPrev, float distToHead) {
    struct CenterNode* node = malloc(sizeof(struct CenterNode));
    strcpy(node->name, name);
    node->distToNext = 0;
    node->distToPrev = 0;
    node->next = NULL;
    node->prev = NULL;

    if (centers.head == NULL) {
        centers.head = node;
        node->next = node;
        node->prev = node;
    } else {
        struct CenterNode* tail = centers.head->prev;
        tail->next = node;
        node->prev = tail;
        node->next = centers.head;
        centers.head->prev = node;

        node->distToPrev = distFromPrev;
        tail->distToNext = distFromPrev;
        node->distToNext = distToHead;
        centers.head->distToPrev = distToHead;
    }
    centers.tail = node;
    centers.count++;
}

void addCenter() {
    printLine();
    printf("  Add Delivery Center\n");
    printLine();

    char name[50];
    printf("  Center Name : "); readLine(name, sizeof(name));

    if (findCenter(name) != NULL) {
        printf("  A center named '%s' already exists.\n", name);
        pressEnter();
        return;
    }

    struct CenterNode* node = malloc(sizeof(struct CenterNode));
    strcpy(node->name, name);
    node->distToNext = 0;
    node->distToPrev = 0;
    node->next = NULL;
    node->prev = NULL;

    if (centers.head == NULL) {
        centers.head = node;
        node->next = node;
        node->prev = node;
        centers.tail = node;
        centers.count++;
        printf("  Center '%s' added.\n", name);
        pressEnter();
        return;
    }

    char afterName[50];
    printf("  Located after which center (default: %s, press Enter to use default): ",
           centers.tail->name);
    readLine(afterName, sizeof(afterName));

    struct CenterNode* afterNode = NULL;
    if (strlen(afterName) == 0) {
        afterNode = centers.tail;
    } else {
        afterNode = findCenter(afterName);
        if (afterNode == NULL) {
            printf("  Center '%s' not found. Aborting.\n", afterName);
            free(node);
            pressEnter();
            return;
        }
    }

    struct CenterNode* beforeNode = afterNode->next;

    float distAfterToNew, distNewToBefore;
    printf("  Distance from %s to %s (km): ", afterNode->name, name);
    readFloat(&distAfterToNew);
    printf("  Distance from %s to %s (km): ", name, beforeNode->name);
    readFloat(&distNewToBefore);

    afterNode->next = node;
    node->prev = afterNode;
    node->next = beforeNode;
    beforeNode->prev = node;

    afterNode->distToNext = distAfterToNew;
    node->distToPrev = distAfterToNew;
    node->distToNext = distNewToBefore;
    beforeNode->distToPrev = distNewToBefore;

    if (afterNode == centers.tail)
        centers.tail = node;

    centers.count++;
    printf("  Center '%s' inserted after '%s'.\n", name, afterNode->name);
    pressEnter();
}

void displayCenters() {
    printLine();
    printf("  Delivery Centers (Circular)\n");
    printLine();

    if (centers.head == NULL) {
        printf("  No centers registered.\n");
        pressEnter();
        return;
    }

    struct CenterNode* curr = centers.head;
    int i = 1;
    do {
        printf("  [%d] %s  -->  %s (%.1f km)\n",
               i++, curr->name, curr->next->name, curr->distToNext);
        curr = curr->next;
    } while (curr != centers.head);

    pressEnter();
}

int calcRoutes(const char* src, const char* dst,
               char* fwdPath, float* fwdDist,
               char* bwdPath, float* bwdDist) {

    struct CenterNode* srcNode = findCenter(src);
    if (srcNode == NULL)
        return 0;
    if (findCenter(dst) == NULL)
        return 0;

    *fwdDist = 0;
    fwdPath[0] = '\0';
    struct CenterNode* curr = srcNode;
    do {
        strcat(fwdPath, curr->name);
        if (strcmp(curr->name, dst) == 0)
            break;
        strcat(fwdPath, " -> ");
        *fwdDist += curr->distToNext;
        curr = curr->next;
    } while (curr != srcNode);

    *bwdDist = 0;
    bwdPath[0] = '\0';
    curr = srcNode;
    do {
        strcat(bwdPath, curr->name);
        if (strcmp(curr->name, dst) == 0)
            break;
        strcat(bwdPath, " -> ");
        *bwdDist += curr->distToPrev;
        curr = curr->prev;
    } while (curr != srcNode);

    return 1;
}

void findBestRoute() {
    char src[50], dst[50];
    printLine();
    printf("  Find Best Route Between Centers\n");
    printLine();

    if (centers.head == NULL || centers.count < 2) {
        printf("  Need at least 2 centers.\n");
        pressEnter();
        return;
    }

    printf("  Source Center      : "); readLine(src, sizeof(src));
    printf("  Destination Center : "); readLine(dst, sizeof(dst));

    char fwdPath[500], bwdPath[500];
    float fwdDist, bwdDist;

    if (calcRoutes(src, dst, fwdPath, &fwdDist, bwdPath, &bwdDist) == 0) {
        printf("  One or both centers not found.\n");
        pressEnter();
        return;
    }

    printf("\n  Forward  : %s (%.2f km)\n", fwdPath, fwdDist);
    printf("  Backward : %s (%.2f km)\n", bwdPath, bwdDist);

    if (fwdDist <= bwdDist)
        printf("\n  Best route: %s (%.2f km)\n", fwdPath, fwdDist);
    else
        printf("\n  Best route: %s (%.2f km)\n", bwdPath, bwdDist);

    pressEnter();
}

void enqueuePickup() {
    int id;
    printLine();
    printf("  Add Pickup Request\n");
    printLine();
    printf("  Parcel ID  : ");
    if (!readInt(&id)) {
        printf("  Invalid input.\n");
        pressEnter();
        return;
    }

    struct Parcel* p = findParcel(id);
    if (p == NULL) {
        printf("  Parcel #%d not found. Register it first.\n", id);
        pressEnter();
        return;
    }

    char timestamp[50];
    printf("  Timestamp  : "); readLine(timestamp, sizeof(timestamp));

    struct QueueNode* node = malloc(sizeof(struct QueueNode));
    node->parcelID = id;
    node->next = NULL;

    if (pickupQueue.rear == NULL) {
        pickupQueue.front = pickupQueue.rear = node;
    } else {
        pickupQueue.rear->next = node;
        pickupQueue.rear = node;
    }

    appendTracking(p, "Pickup Requested", p->source, timestamp);

    printf("  Parcel #%d added to pickup queue.\n", id);
    pressEnter();
}

void displayPickupQueue() {
    printLine();
    printf("  Pickup Queue\n");
    printLine();

    if (pickupQueue.front == NULL) {
        printf("  Pickup queue is empty.\n");
        pressEnter();
        return;
    }

    struct QueueNode* q = pickupQueue.front;
    int pos = 1;
    while (q != NULL) {
        printf("  [%d] Parcel #%d\n", pos++, q->parcelID);
        q = q->next;
    }
    pressEnter();
}

void moveToDispatch() {
    printLine();
    printf("  Move Pickup -> Dispatch\n");
    printLine();

    if (pickupQueue.front == NULL) {
        printf("  Pickup queue is empty.\n");
        pressEnter();
        return;
    }

    char timestamp[50];
    printf("  Timestamp  : "); readLine(timestamp, sizeof(timestamp));

    int id = pickupQueue.front->parcelID;
    struct QueueNode* temp = pickupQueue.front;
    pickupQueue.front = pickupQueue.front->next;
    if (pickupQueue.front == NULL)
        pickupQueue.rear = NULL;
    free(temp);

    struct QueueNode* node = malloc(sizeof(struct QueueNode));
    node->parcelID = id;
    node->next = NULL;

    if (dispatchQueue.rear == NULL) {
        dispatchQueue.front = dispatchQueue.rear = node;
    } else {
        dispatchQueue.rear->next = node;
        dispatchQueue.rear = node;
    }

    struct Parcel* p = findParcel(id);
    if (p != NULL)
        appendTracking(p, "Out for Delivery", p->source, timestamp);

    printf("  Parcel #%d moved to dispatch queue.\n", id);
    pressEnter();
}

void displayDispatchQueue() {
    printLine();
    printf("  Dispatch Queue\n");
    printLine();

    if (dispatchQueue.front == NULL) {
        printf("  Dispatch queue is empty.\n");
        pressEnter();
        return;
    }

    struct QueueNode* q = dispatchQueue.front;
    int pos = 1;
    while (q != NULL) {
        struct Parcel* p = findParcel(q->parcelID);
        if (p != NULL)
            printf("  [%d] Parcel #%d  |  %.2f kg  |  %s -> %s\n",
                   pos, q->parcelID, p->weight, p->source, p->destination);
        else
            printf("  [%d] Parcel #%d\n", pos, q->parcelID);
        pos++;
        q = q->next;
    }
    pressEnter();
}

void sortDispatchByWeight() {
    printLine();
    printf("  Sort Dispatch Queue by Weight (Heaviest First)\n");
    printLine();

    if (dispatchQueue.front == NULL) {
        printf("  Dispatch queue is empty.\n");
        pressEnter();
        return;
    }

    struct QueueNode* sortedTail = dispatchQueue.front;

    while (sortedTail->next != NULL) {
        struct QueueNode* curr = sortedTail->next;
        struct Parcel* currParcel = findParcel(curr->parcelID);
        float currWeight = (currParcel != NULL) ? currParcel->weight : 0;

        struct Parcel* frontParcel = findParcel(dispatchQueue.front->parcelID);
        float frontWeight = (frontParcel != NULL) ? frontParcel->weight : 0;

        if (currWeight >= frontWeight) {
            sortedTail->next = curr->next;
            curr->next = dispatchQueue.front;
            dispatchQueue.front = curr;
        } else {
            struct QueueNode* s = dispatchQueue.front;
            while (s->next != curr) {
                struct Parcel* nextParcel = findParcel(s->next->parcelID);
                float nextWeight = (nextParcel != NULL) ? nextParcel->weight : 0;
                if (currWeight >= nextWeight)
                    break;
                s = s->next;
            }
            if (s->next != curr) {
                sortedTail->next = curr->next;
                curr->next = s->next;
                s->next = curr;
            } else {
                sortedTail = sortedTail->next;
            }
        }
    }

    dispatchQueue.rear = sortedTail;

    printf("  Dispatch queue sorted by weight (heaviest first).\n\n");

    struct QueueNode* q = dispatchQueue.front;
    int pos = 1;
    while (q != NULL) {
        struct Parcel* p = findParcel(q->parcelID);
        if (p != NULL)
            printf("  [%d] Parcel #%d  |  %.2f kg  |  %s -> %s\n",
                   pos, q->parcelID, p->weight, p->source, p->destination);
        pos++;
        q = q->next;
    }
    pressEnter();
}

void pushRoute(const char* center) {
    struct StackNode* node = malloc(sizeof(struct StackNode));
    strcpy(node->center, center);
    node->next = routeStack.top;
    routeStack.top = node;
}

void simulateDelivery() {
    printLine();
    printf("  Simulate Delivery\n");
    printLine();

    if (dispatchQueue.front == NULL) {
        printf("  Dispatch queue is empty. No parcels to deliver.\n");
        pressEnter();
        return;
    }

    int id = dispatchQueue.front->parcelID;
    struct QueueNode* temp = dispatchQueue.front;
    dispatchQueue.front = dispatchQueue.front->next;
    if (dispatchQueue.front == NULL)
        dispatchQueue.rear = NULL;
    free(temp);

    struct Parcel* p = findParcel(id);
    if (p == NULL) {
        printf("  Parcel #%d record missing. Skipping.\n", id);
        pressEnter();
        return;
    }

    printf("\n  Delivering Parcel #%d  (%s -> %s)\n\n", id, p->source, p->destination);

    if (centers.head == NULL) {
        pushRoute(p->destination);
        pushRoute(p->source);
    } else {
        char fwdPath[500], bwdPath[500];
        float fwdDist, bwdDist;
        int found = calcRoutes(p->source, p->destination,
                               fwdPath, &fwdDist, bwdPath, &bwdDist);

        if (found == 0) {
            pushRoute(p->destination);
            pushRoute(p->source);
        } else {
            int useForward = (fwdDist <= bwdDist);
            struct CenterNode* dstNode = findCenter(p->destination);
            struct CenterNode* c = dstNode;
            int steps = 0;

            do {
                pushRoute(c->name);
                if (strcmp(c->name, p->source) == 0)
                    break;
                c = useForward ? c->prev : c->next;
                steps++;
                if (steps > centers.count + 1)
                    break;
            } while (c != dstNode);
        }
    }

    printf("  Route taken:\n\n");
    int step = 1;
    while (routeStack.top != NULL) {
        printf("  Step %d : %s\n", step++, routeStack.top->center);
        struct StackNode* top = routeStack.top;
        routeStack.top = routeStack.top->next;
        free(top);
    }

    char timestamp[50];
    printf("\n  Delivery Timestamp : "); readLine(timestamp, sizeof(timestamp));

    appendTracking(p, "Delivered", p->destination, timestamp);

    detachParcelFromList(id);
    removeFromQueue(&pickupQueue, id);
    pushCompleted(p);

    printf("  Parcel #%d delivered and moved to completed records.\n", id);
    pressEnter();
}

void viewCompleted() {
    printLine();
    printf("  Completed Deliveries\n");
    printLine();

    if (completed.top == NULL) {
        printf("  No completed deliveries.\n");
        pressEnter();
        return;
    }

    struct CompletedNode* cn = completed.top;
    int i = 1;
    while (cn != NULL) {
        struct Parcel* p = cn->parcel;
        printf("  [%d] Parcel #%d  |  %s -> %s  |  %.2f kg  |  %s -> %s\n",
               i++, p->id, p->source, p->destination, p->weight, p->sender, p->receiver);

        struct TrackingNode* t = p->trackingHead;
        int step = 1;
        while (t != NULL) {
            printf("       [%d] %s  |  %s  |  %s\n", step++, t->status, t->location, t->timestamp);
            t = t->next;
        }
        printLine();
        cn = cn->next;
    }
    pressEnter();
}

void clearCompleted() {
    printLine();
    printf("  Clear Completed Deliveries\n");
    printLine();

    if (completed.top == NULL) {
        printf("  Nothing to clear.\n");
        pressEnter();
        return;
    }

    while (completed.top != NULL) {
        struct CompletedNode* cn = completed.top;
        completed.top = completed.top->next;
        freeTracking(cn->parcel->trackingHead);
        free(cn->parcel);
        free(cn);
    }

    printf("  Completed deliveries cleared.\n");
    pressEnter();
}

void loadDemoData() {
    printLine();
    printf("  Load Demo Data\n");
    printLine();

    if (demoLoaded) {
        printf("  Demo data is already loaded.\n");
        pressEnter();
        return;
    }

    char timestamp[50];
    printf("  Timestamp for all demo entries : "); readLine(timestamp, sizeof(timestamp));

    insertCenterAtEnd("Colombo", 0, 120.0);
    insertCenterAtEnd("Gampaha", 35.0, 505.0);
    insertCenterAtEnd("Anuradhapura", 180.0, 290.0);
    insertCenterAtEnd("Galle", 290.0, 120.0);

    printf("  Centers: Colombo, Gampaha, Anuradhapura, Galle\n");

    struct Parcel* p1 = malloc(sizeof(struct Parcel));
    p1->id = 101;
    strcpy(p1->sender, "Chathushi");
    strcpy(p1->receiver, "Sadeesha");
    p1->weight = 2.5;
    strcpy(p1->source, "Colombo");
    strcpy(p1->destination, "Gampaha");
    p1->trackingHead = NULL;
    p1->next = NULL;
    p1->prev = NULL;
    appendTracking(p1, "Registered", "Colombo", timestamp);
    parcels.head = p1;

    struct Parcel* p2 = malloc(sizeof(struct Parcel));
    p2->id = 102;
    strcpy(p2->sender, "Manula");
    strcpy(p2->receiver, "Supun");
    p2->weight = 1.0;
    strcpy(p2->source, "Gampaha");
    strcpy(p2->destination, "Galle");
    p2->trackingHead = NULL;
    p2->next = NULL;
    p2->prev = p1;
    appendTracking(p2, "Registered", "Gampaha", timestamp);
    p1->next = p2;

    struct Parcel* p3 = malloc(sizeof(struct Parcel));
    p3->id = 103;
    strcpy(p3->sender, "Dahamsa");
    strcpy(p3->receiver, "Bindusara");
    p3->weight = 4.0;
    strcpy(p3->source, "Galle");
    strcpy(p3->destination, "Colombo");
    p3->trackingHead = NULL;
    p3->next = NULL;
    p3->prev = p2;
    appendTracking(p3, "Registered", "Galle", timestamp);
    p2->next = p3;

    struct Parcel* p4 = malloc(sizeof(struct Parcel));
    p4->id = 104;
    strcpy(p4->sender, "Thisara");
    strcpy(p4->receiver, "Dahamsa");
    p4->weight = 6.0;
    strcpy(p4->source, "Anuradhapura");
    strcpy(p4->destination, "Galle");
    p4->trackingHead = NULL;
    p4->next = NULL;
    p4->prev = p3;
    appendTracking(p4, "Registered", "Anuradhapura", timestamp);
    p3->next = p4;

    struct Parcel* p5 = malloc(sizeof(struct Parcel));
    p5->id = 105;
    strcpy(p5->sender, "Amal");
    strcpy(p5->receiver, "Kamal");
    p5->weight = 3.2;
    strcpy(p5->source, "Colombo");
    strcpy(p5->destination, "Anuradhapura");
    p5->trackingHead = NULL;
    p5->next = NULL;
    p5->prev = p4;
    appendTracking(p5, "Registered", "Colombo", timestamp);
    p4->next = p5;

    demoLoaded = 1;
    printf("  Parcels: #101 (Colombo->Gampaha, 2.5kg), #102 (Gampaha->Galle, 1.0kg),\n");
    printf("           #103 (Galle->Colombo, 4.0kg), #104 (Anuradhapura->Galle, 6.0kg),\n");
    printf("           #105 (Colombo->Anuradhapura, 3.2kg)\n");
    printf("\n  Demo data loaded.\n");
    pressEnter();
}

void centersMenu() {
    int choice;
    do {
        printf("\n  [ Centers ]\n");
        printLine();
        printf("  1. Add Delivery Center\n");
        printf("  2. Display Centers\n");
        printf("  3. Find Best Route\n");
        printf("  0. Back\n");
        printLine();
        printf("  Choice: "); readInt(&choice);

        switch (choice) {
            case 1: {
                addCenter();
                break;
            }
            case 2: {
                displayCenters();
                break;
            }
            case 3: {
                findBestRoute();
                break;
            }
            case 0: {
                break;
            }
            default: {
                printf("  Invalid choice.\n");
            }
        }
    } while (choice != 0);
}

void parcelsMenu() {
    int choice;
    do {
        printf("\n  [ Parcels ]\n");
        printLine();
        printf("  1. Register Parcel\n");
        printf("  2. Delete Parcel\n");
        printf("  3. Display All Parcels\n");
        printf("  0. Back\n");
        printLine();
        printf("  Choice: "); readInt(&choice);

        switch (choice) {
            case 1: {
                addParcel();
                break;
            }
            case 2: {
                deleteParcel();
                break;
            }
            case 3: {
                displayParcels();
                break;
            }
            case 0: {
                break;
            }
            default: {
                printf("  Invalid choice.\n");
            }
        }
    } while (choice != 0);
}

void trackingMenu() {
    int choice;
    do {
        printf("\n  [ Tracking ]\n");
        printLine();
        printf("  1. Add Tracking Update\n");
        printf("  2. View Tracking History\n");
        printf("  0. Back\n");
        printLine();
        printf("  Choice: "); readInt(&choice);

        switch (choice) {
            case 1: {
                addTracking();
                break;
            }
            case 2: {
                viewTracking();
                break;
            }
            case 0: {
                break;
            }
            default: {
                printf("  Invalid choice.\n");
            }
        }
    } while (choice != 0);
}

void dispatchMenu() {
    int choice;
    do {
        printf("\n  [ Dispatch ]\n");
        printLine();
        printf("  1. Add Pickup Request\n");
        printf("  2. View Pickup Queue\n");
        printf("  3. Move to Dispatch Queue\n");
        printf("  4. View Dispatch Queue\n");
        printf("  5. Sort Dispatch by Weight (Heaviest First)\n");
        printf("  6. Deliver Next Parcel (Simulate)\n");
        printf("  0. Back\n");
        printLine();
        printf("  Choice: "); readInt(&choice);

        switch (choice) {
            case 1: {
                enqueuePickup();
                break;
            }
            case 2: {
                displayPickupQueue();
                break;
            }
            case 3: {
                moveToDispatch();
                break;
            }
            case 4: {
                displayDispatchQueue();
                break;
            }
            case 5: {
                sortDispatchByWeight();
                break;
            }
            case 6: {
                simulateDelivery();
                break;
            }
            case 0: {
                break;
            }
            default: {
                printf("  Invalid choice.\n");
            }
        }
    } while (choice != 0);
}

void completedMenu() {
    int choice;
    do {
        printf("\n  [ Completed Deliveries ]\n");
        printLine();
        printf("  1. View Completed Deliveries\n");
        printf("  2. Clear Completed Deliveries\n");
        printf("  0. Back\n");
        printLine();
        printf("  Choice: "); readInt(&choice);

        switch (choice) {
            case 1: {
                viewCompleted();
                break;
            }
            case 2: {
                clearCompleted();
                break;
            }
            case 0: {
                break;
            }
            default: {
                printf("  Invalid choice.\n");
            }
        }
    } while (choice != 0);
}

int main() {
    int choice;

    printf("\n==================================================\n");
    printf("         COURIER MANAGEMENT SYSTEM\n");
    printf("==================================================\n");

    do {
        printf("\n  MAIN MENU\n");
        printLine();
        printf("  1. Delivery Centers\n");
        printf("  2. Parcels\n");
        printf("  3. Tracking\n");
        printf("  4. Dispatch & Delivery\n");
        printf("  5. Completed Deliveries\n");
        printf("  6. Load Demo Data\n");
        printf("  0. Exit\n");
        printLine();
        printf("  Choice: "); readInt(&choice);

        switch (choice) {
            case 1: {
                centersMenu();
                break;
            }
            case 2: {
                parcelsMenu();
                break;
            }
            case 3: {
                trackingMenu();
                break;
            }
            case 4: {
                dispatchMenu();
                break;
            }
            case 5: {
                completedMenu();
                break;
            }
            case 6: {
                loadDemoData();
                break;
            }
            case 0: {
                printf("\n  Goodbye.\n\n");
                break;
            }
            default: {
                printf("  Invalid choice.\n");
            }
        }
    } while (choice != 0);

    return 0;
}
