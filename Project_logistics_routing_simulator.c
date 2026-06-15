#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 10

// Data structure representing a single shipment
typedef struct {
    char tracking[16];
    char hub[32];
    int priority;
    int storage_days;
} Shipment;

// Queue node for handling shipments within a specific Hub
typedef struct QueueNode {
    Shipment data;
    struct QueueNode* next;
} QueueNode;

// Queue structure (FIFO) with head and tail pointers for O(1) insertion
typedef struct {
    QueueNode* head;
    QueueNode* tail;
} Queue;

// Linked list node representing a logistics Hub, containing its specific queue
typedef struct HubNode {
    char hub_name[32];
    Queue shipment_queue;
    struct HubNode* next;
} HubNode;

// Hash table node for fast tracking lookup (collision handling via chaining)
typedef struct HashNode {
    Shipment data;
    struct HashNode* next;
} HashNode;

typedef HashNode* HashTable[HASH_SIZE];

// Computes a basic hash for string keys
int hash_string(char* str) {
    int h = 0;
    for (int i = 0; i < strlen(str); i++) {
        h += str[i];
    }
    return h % HASH_SIZE;
}

// Initializes the hash table with NULL pointers
void init_hash_table(HashTable ht) {
    for (int i = 0; i < HASH_SIZE; i++) {
        ht[i] = NULL;
    }
}

// Finds an existing Hub in the linked list by name
HubNode *find_hub(HubNode *head, char *hub_name) {
    HubNode *curr_hub = head;
    while (curr_hub != NULL) {
        if (strcmp(curr_hub->hub_name, hub_name) == 0) {
            return curr_hub;
        }
        curr_hub = curr_hub->next;
    }
    return NULL;
}

// Allocates and initializes a new Hub node, inserting it at the head of the list
HubNode *create_hub(HubNode *head, char *hub_name) {
    HubNode *new_node = malloc(sizeof(HubNode));
    if (new_node == NULL) return NULL;

    strcpy(new_node->hub_name, hub_name);
    new_node->shipment_queue.head = new_node->shipment_queue.tail = NULL;
    new_node->next = head;
    return new_node;
}

// Enqueues a shipment into a specific Hub's queue
void enqueue_data(Queue *queue, Shipment s) {
    QueueNode *new_node = malloc(sizeof(QueueNode));
    if (new_node == NULL) return;
    new_node->data = s;
    new_node->next = NULL;

    if (queue->head == NULL) {
        queue->head = new_node;
    } else {
        queue->tail->next = new_node;
    }
    queue->tail = new_node;
}

// Parses logistics data from a text file into a dynamically allocated array
Shipment* Q1a_load_data(const char* filename, int* num_records) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) return NULL;

    *num_records = 0;
    if (fscanf(fp, "%d", num_records) != 1 || *num_records == 0) {
        fclose(fp);
        return NULL;
    }

    Shipment *array = malloc(*num_records * sizeof(Shipment));
    if (array == NULL) {
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < *num_records; i++) {
        fscanf(fp, "%s%s%d%d",
            array[i].tracking,
            array[i].hub,
            &array[i].priority,
            &array[i].storage_days);
    }

    fclose(fp);
    return array;
}

// Simulates time passing by increasing the storage days for all shipments
void Q1b_update_days(Shipment* array, int num_records, int x) {
    if (array == NULL || num_records == 0) return;
    for (int i = 0; i < num_records; i++) {
        array[i].storage_days += x;
    }
}

// Filters shipments, returning a new dynamically allocated array of urgent packages
Shipment* Q1c_filter_urgent(Shipment* array, int num_records, int* num_urgent) {
    if (array == NULL || num_records == 0) return NULL;

    *num_urgent = 0;
    for (int i = 0; i < num_records; i++) {
        if (array[i].priority == 1 || array[i].storage_days > 5) {
            (*num_urgent)++;
        }
    }

    if (*num_urgent == 0) return NULL;

    Shipment *urgent_array = malloc(*num_urgent * sizeof(Shipment));
    if (urgent_array == NULL) return NULL;

    int j = 0;
    for (int i = 0; i < num_records; i++) {
        if (array[i].priority == 1 || array[i].storage_days > 5) {
            urgent_array[j++] = array[i];
        }
    }
    return urgent_array;
}

// Sorts shipments by storage duration in descending order using Insertion Sort
void Q1d_sort(Shipment* array, int num_urgent) {
    if (array == NULL || num_urgent <= 1) return;

    for (int i = 1; i < num_urgent; i++) {
        Shipment key = array[i];
        int j = i - 1;
        while (j >= 0 && array[j].storage_days < key.storage_days) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

// Routes sorted shipments into their respective Hub queues
HubNode* Q2a_route_shipments(Shipment* array, int num_urgent) {
    if (array == NULL || num_urgent == 0) return NULL;

    HubNode *list = NULL;
    for (int i = 0; i < num_urgent; i++) {
        HubNode *curr_hub = find_hub(list, array[i].hub);
        if (curr_hub == NULL) {
            curr_hub = create_hub(list, array[i].hub);
            list = curr_hub;
        }
        enqueue_data(&curr_hub->shipment_queue, array[i]);
    }
    return list;
}

// Inserts a shipment into the hash table handling collisions at the head
void Q2b_hash_insert(HashTable ht, Shipment s) {
    int IDX = hash_string(s.tracking);
    HashNode *new_node = malloc(sizeof(HashNode));
    if (new_node == NULL) return;

    new_node->data = s;
    new_node->next = ht[IDX];
    ht[IDX] = new_node;
}

// Populates the Hash Table by iterating through all Hub queues
void Q2c_hash_populate(HashTable ht, HubNode* hub_list) {
    init_hash_table(ht);
    HubNode *curr_hub = hub_list;

    while (curr_hub != NULL) {
        QueueNode *curr_node = curr_hub->shipment_queue.head;
        while (curr_node != NULL) {
            Q2b_hash_insert(ht, curr_node->data);
            curr_node = curr_node->next;
        }
        curr_hub = curr_hub->next;
    }
}

// Searches for a shipment by tracking ID and prints its details
void Q2d_hash_search(HashTable ht, char* tracking) {
    if (ht == NULL) return;
    int IDX = hash_string(tracking);
    HashNode *curr_node = ht[IDX];

    while (curr_node != NULL) {
        if (strcmp(curr_node->data.tracking, tracking) == 0) {
            Shipment data = curr_node->data;
            printf("\nFound: Tracking: [%s] | Hub: [%s] | Priority: [%d] | Storage Days: [%d]\n",
                data.tracking, data.hub, data.priority, data.storage_days);
            return;
        }
        curr_node = curr_node->next;
    }
    printf("\nNOT FOUND\n");
}

// Removes a shipment from the hash table (simulating delivery)
int Q2e_hash_delete(HashTable ht, char* tracking) {
    if (ht == NULL) return 0;
    int IDX = hash_string(tracking);
    HashNode *curr = ht[IDX];
    HashNode *prev = NULL;

    while (curr != NULL) {
        if (strcmp(curr->data.tracking, tracking) == 0) {
            if (prev == NULL) {
                ht[IDX] = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

// Dequeues the next shipment to be processed from a specific Hub
Shipment Q2f_queue_extract(HubNode* hub_list, char* hub_name, int* success) {
    Shipment empty_shipment = {"", "", 0, 0};
    *success = 0;

    HubNode *curr_hub = find_hub(hub_list, hub_name);
    if (curr_hub == NULL || curr_hub->shipment_queue.head == NULL) return empty_shipment;

    QueueNode *node_to_delete = curr_hub->shipment_queue.head;
    Shipment shipment = node_to_delete->data;

    curr_hub->shipment_queue.head = node_to_delete->next;
    free(node_to_delete);

    if (curr_hub->shipment_queue.head == NULL) {
        curr_hub->shipment_queue.tail = NULL;
    }

    *success = 1;
    return shipment;
}

int main() {
    int num_records = 0;
    int num_urgent = 0;

    printf("--- SYSTEM INIT ---\n");
    Shipment* packages = Q1a_load_data("shipments.txt", &num_records);
    if (packages == NULL) {
        printf("ERROR: Failed to load data.\n");
        return 1;
    }
    printf("Loaded %d records.\n", num_records);

    Q1b_update_days(packages, num_records, 2);
    Shipment* urgent_packages = Q1c_filter_urgent(packages, num_records, &num_urgent);
    Q1d_sort(urgent_packages, num_urgent);

    printf("\n--- LOGISTICS ROUTING ---\n");
    HubNode* hub_list = Q2a_route_shipments(urgent_packages, num_urgent);

    HashTable ht;
    Q2c_hash_populate(ht, hub_list);
    printf("Hash Table populated with urgent shipments.\n");

    printf("\nLookup TRK002B:");
    Q2d_hash_search(ht, "TRK002B");

    printf("\nDelivering TRK002B...");
    if(Q2e_hash_delete(ht, "TRK002B")) {
        printf(" Delivery successful. Package removed from system.\n");
    }

    printf("\nNext dispatch from 'Roma' Hub:\n");
    int success_ext;
    Shipment extracted_shipment = Q2f_queue_extract(hub_list, "Roma", &success_ext);
    if(success_ext) {
        printf("Dispatched: %s\n", extracted_shipment.tracking);
    }

    free(packages);
    free(urgent_packages);
    return 0;
}